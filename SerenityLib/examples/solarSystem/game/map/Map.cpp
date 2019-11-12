#include "Map.h"
#include "Anchor.h"
#include "../Core.h"
#include "../Menu.h"
#include "../networking/Client.h"
#include "../ResourceManifest.h"
#include "../Planet.h"
#include "../GameCamera.h"
#include "../Ship.h"
#include "../GameSkybox.h"
#include "../Helper.h"
#include "../hud/HUD.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/lights/Lights.h>
#include <core/engine/model/ModelInstance.h>

#include <algorithm>
#include <sstream>
#include <btBulletCollisionCommon.h>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <ecs/Entity.h>
#include <ecs/Components.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>

#include "core/Terrain.h"
#include "../ships/Ships.h"
#include <SFML/Graphics.hpp>

#include "../modes/GameplayMode.h"
#include "../teams/Team.h"

#include "../hud/SensorStatusDisplay.h"
#include "../hud/ShipStatusDisplay.h"

using namespace Engine;
using namespace std;
namespace boost_io = boost::iostreams;

Map::Map(GameplayMode& mode, Client& client, const string& n, const string& file):Scene(n), m_Client(client), m_GameplayMode(mode){
    m_Player      = nullptr;

    m_ActiveCannonProjectiles.initialize(2500);
    m_ActiveTorpedoProjectiles.initialize(2500);

    GameCamera* playerCamera = new GameCamera(0.35f,7000000000.0f,this);

    setActiveCamera(*playerCamera);
    m_Objects.push_back(playerCamera);
    m_Filename = file;
    m_SkyboxFile = "";
    m_RootAnchor = std::tuple<std::string,Anchor*>("",nullptr);
    if(file != "NULL")
        Map::loadFromFile(file);
    Font& font = client.m_Core.m_Menu->getFont();
    m_HUD = new HUD(*this, font);
}

void Map::cleanup() {
    m_ActiveCannonProjectiles.clear();
    m_ActiveTorpedoProjectiles.clear();
    m_Planets.clear();
    m_ShipsNPCControlled.clear();
    m_ShipsPlayerControlled.clear();
    m_Ships.clear();
    SAFE_DELETE_VECTOR(m_Objects);
}

Map::~Map(){
    cleanup();
    SAFE_DELETE(m_HUD);
}
PrimaryWeaponCannonProjectile* Map::getCannonProjectile(const int index) {
    return m_ActiveCannonProjectiles[index];
}
SecondaryWeaponTorpedoProjectile* Map::getTorpedoProjectile(const int index) {
    return m_ActiveTorpedoProjectiles[index];
}
void Map::removeCannonProjectile(const int index) {
    const auto res = m_ActiveCannonProjectiles.delete_data_index(index);
}
void Map::removeTorpedoProjectile(const int index) {
    const auto res = m_ActiveTorpedoProjectiles.delete_data_index(index);
}
const int Map::addCannonProjectile(PrimaryWeaponCannonProjectile* projectile, const int index) {
    if(index == -1)
        return m_ActiveCannonProjectiles.push_back(projectile);
    return m_ActiveCannonProjectiles.insert(projectile, index);
}
const int Map::addTorpedoProjectile(SecondaryWeaponTorpedoProjectile* projectile, const int index) {
    if (index == -1)
        return m_ActiveTorpedoProjectiles.push_back(projectile);
    return m_ActiveTorpedoProjectiles.insert(projectile, index);
}
const int Map::try_addCannonProjectile() {
    return m_ActiveCannonProjectiles.use_next_available_index();
}
const int Map::try_addTorpedoProjectile() {
    return m_ActiveTorpedoProjectiles.use_next_available_index();
}
const bool Map::try_addCannonProjectile(const int requestedIndex) {
    return m_ActiveCannonProjectiles.can_push_at_index(requestedIndex);
}
const bool Map::try_addTorpedoProjectile(const int requestedIndex) {
    return m_ActiveTorpedoProjectiles.can_push_at_index(requestedIndex);
}
EntityWrapper* Map::getEntityFromName(const string& name) {
    for (auto& wrapper : m_Objects) {
        auto* Name = wrapper->getComponent<ComponentName>();
        if (Name  && Name->name() == name)
            return wrapper;
    }
    return nullptr;
}

string Map::allowedShipsSingleString() {
    uint                                             count = 0;
    boost_io::stream<boost_io::mapped_file_source>   str(m_Filename);
    for (string line; getline(str, line, '\n');) {
        line.erase(remove(line.begin(), line.end(), '\r'), line.end()); //remove \r from the line
        if (line[0] != '#') {//ignore commented lines
            if (count == 5) {//this line has the allowed ships
                return line;
            }
        }
        ++count;
    }
    return "";
}
unordered_map<string, Planet*>& Map::getPlanets() { 
    return m_Planets; 
}
unordered_map<string, Ship*>& Map::getShipsPlayerControlled() { 
    return m_ShipsPlayerControlled; 
}
unordered_map<string, Ship*>& Map::getShipsNPCControlled() {
    return m_ShipsNPCControlled;
}
unordered_map<string, Ship*>& Map::getShips() {
    return m_Ships;
}
Ship* Map::getPlayer() { 
    return m_Player; 
}
void Map::setPlayer(Ship* p) { 
    m_Player = p; 
    m_HUD->getSensorDisplay().setShip(p);
    m_HUD->getShipStatusDisplay().setTarget(p);
}

const string& Map::skyboxFile() const {
    return m_SkyboxFile;
}
vector<string> Map::allowedShips() {
    uint                                             count = 0;
    boost_io::stream<boost_io::mapped_file_source>   str(m_Filename);
    for (string line; getline(str, line, '\n');) {
        line.erase(remove(line.begin(), line.end(), '\r'), line.end()); //remove \r from the line
        if (line[0] != '#') {//ignore commented lines
            if (count == 5) {//this line has the allowed ships
                return Helper::SeparateStringByCharacter(line, ',');
            }
        }
        ++count;
    }
    return vector<string>();
}
Anchor* Map::internalCreateDeepspaceAnchor(const decimal& x, const decimal& y, const decimal& z, const string& name) {
    Anchor* anchor = new Anchor(*this, x, y, z);
    Anchor* root = std::get<1>(m_RootAnchor);
    m_Objects.push_back(anchor);

    string key;
    if (name.empty()) {
        key = "Deepspace Anchor 0";
    }else{
        key = name;
    }
    unsigned int count = 0;
    while (true) {
        if (!root->m_Children.count(key)) {
            root->m_Children.emplace(key, anchor);
            break;
        }else{
            ++count;
            key = "Deepspace Anchor " + std::to_string(count);
        }
    }
    return anchor;
}
Anchor* Map::internalCreateAnchor(const string& parentAnchor, const string& thisName, unordered_map<string, Anchor*>& loadedAnchors, const decimal& x, const decimal& y, const decimal& z) {
    Anchor* anchor = new Anchor(*this, x, y, z);
    m_Objects.push_back(anchor);
    const string key = thisName + " Anchor";
    if (parentAnchor.empty()) {
        if (std::get<0>(m_RootAnchor).empty()) {
            m_RootAnchor = std::tuple<std::string, Anchor*>(key, anchor);
        }
    }else{
        Anchor* parent = loadedAnchors.at(parentAnchor + " Anchor");
        if (!parent->m_Children.count(key)) {
            parent->m_Children.emplace(key, anchor);
        }
    }
    if (!loadedAnchors.count(key)) {
        loadedAnchors.emplace(key, anchor);
    }
    return anchor;
}
Client& Map::getClient() {
    return m_Client;
}
Anchor* Map::internalCreateAnchor(const string& parentAnchor, const string& thisName, unordered_map<string, Anchor*>& loadedAnchors, const glm_vec3& position) {
    return internalCreateAnchor(parentAnchor, thisName, loadedAnchors, position.x, position.y, position.z);
}
void Map::loadFromFile(const string& filename) {
    uint                                             count = 0;
    boost_io::stream<boost_io::mapped_file_source>   str(filename);
    unordered_map<string, vector<RingInfo>>          planetRings;
    unordered_map<string, Handle>                    loadedMaterials;
    unordered_map<string, Handle>                    loadedMeshes;
    unordered_map<string, Anchor*>                   loadedAnchors;

    float gi_diffuse  = 1.0f;
    float gi_specular = 1.0f;
    float gi_global   = 1.0f;

    internalCreateAnchor("", "Root", loadedAnchors, static_cast<decimal>(0.0), static_cast<decimal>(0.0), static_cast<decimal>(0.0));

    for (string line; getline(str, line, '\n');) {
        line.erase(remove(line.begin(), line.end(), '\r'), line.end()); //remove \r from the line
        if (line[0] != '#') {//ignore commented lines
            if (count == 1) {//this line has the system's name
                setName(line);
            }else if (count == 2) {//this line has the system's skybox
                m_SkyboxFile = ResourceManifest::BasePath + line;
            }else if (count == 3) {//this line has the system's skybox's number of flares
                GameSkybox* box = new GameSkybox(m_SkyboxFile, boost::lexical_cast<uint>(line));
                setSkybox(box);
            }else if (count == 4) {//this line has the system's GI contribution
                string token;
                istringstream stream(line);
                while (getline(stream, token, ' ')) {
                    size_t pos = token.find("=");
                    string key = token.substr(0, pos);
                    string value = token.substr(pos + 1, string::npos);
                    if      (key == "giDiffuse")   gi_diffuse = stof(value);
                    else if (key == "giSpecular")  gi_specular = stof(value);
                    else if (key == "giGlobal")    gi_global = stof(value);
                }
            }else if (count == 5) {//this line has the allowed ships
                stringstream ss(line);
                vector<string> result;
                while (ss.good()){
                    string substr;
                    getline(ss, substr, ',');
                    result.push_back(substr);
                }

            }
            if (!line.empty() && line[0] != ' ' && line[1] == ' ') {//we got something to work with
                Planet* planetoid = nullptr;

                string token;
                istringstream stream(line);

                string NAME;
                string LAGRANGE_TYPE;
                string LAGRANGE_PLANET_1, LAGRANGE_PLANET_2;
                string PARENT = "";
                float R, G, B, R1, G1, B1, R2, G2, B2, qx, qy, qz = 0.0f;
                float ATMOSPHERE_HEIGHT = 0.0f;
                PlanetType::Type TYPE;
                string MESH = ResourceManifest::BasePath + "data/Planets/Models/";
                string TEXTURE = ResourceManifest::BasePath + "data/Planets/Textures/";
                string MATERIAL_NAME = "";
                string MESH_NAME = "";

                float ORBIT_PERIOD = -1;
                unsigned long long ORBIT_MAJOR_AXIS = -1;
                float ORBIT_ECCENTRICITY = -1;
                float INCLINATION = 0.0f;

                float ROTATIONAL_TILT = -1;
                float ROTATIONAL_PERIOD = -1;

                unsigned long long RADIUS = 0;
                unsigned long long POSITION = 0;

                long long X = 0;
                long long Y = 0;
                long long Z = 0;
                float     Angle = 0;

                uint BREAK = 0;

                while (getline(stream, token, ' ')) {
                    size_t pos = token.find("=");

                    string key = token.substr(0, pos);
                    string value = token.substr(pos + 1, string::npos);

                    if (key == "name") {
                        NAME = value;
                        replace(NAME.begin(), NAME.end(), '_', ' ');
                    }
                    else if (key == "lp1")              LAGRANGE_PLANET_1 = value;
                    else if (key == "lp2")              LAGRANGE_PLANET_2 = value;
                    else if (key == "lType")            LAGRANGE_TYPE = value;
                    else if (key == "radius")           RADIUS = stoull(value) * 10;
                    else if (key == "r")                R = stof(value);
                    else if (key == "g")                G = stof(value);
                    else if (key == "b")                B = stof(value);
                    else if (key == "r1")               R1 = stof(value);
                    else if (key == "g1")               G1 = stof(value);
                    else if (key == "b1")               B1 = stof(value);
                    else if (key == "r2")               R2 = stof(value);
                    else if (key == "g2")               G2 = stof(value);
                    else if (key == "b2")               B2 = stof(value);
                    else if (key == "position")         POSITION = stoull(value) * 10;

                    else if (key == "x")                X = stoull(value);
                    else if (key == "y")                Y = stoull(value);
                    else if (key == "z")                Z = stoull(value);
                    else if (key == "qx")               qx = stof(value);
                    else if (key == "qy")               qy = stof(value);
                    else if (key == "qz")               qz = stof(value);
                    else if (key == "a")                Angle = stof(value);

                    else if (key == "parent")           PARENT = value;
                    else if (key == "type")             TYPE = static_cast<PlanetType::Type>(static_cast<unsigned int>(stoi(value)));
                    else if (key == "atmosphereHeight") ATMOSPHERE_HEIGHT = stof(value);
                    else if (key == "break")            BREAK = stoi(value);
                    else if (key == "eccentricity")     ORBIT_ECCENTRICITY = stof(value);
                    else if (key == "period")           ORBIT_PERIOD = stof(value);
                    else if (key == "majorAxis")        ORBIT_MAJOR_AXIS = stoll(value) * 10;
                    else if (key == "days")             ROTATIONAL_PERIOD = stof(value);
                    else if (key == "tilt")             ROTATIONAL_TILT = stof(value);
                    else if (key == "inclination")      INCLINATION = stof(value);
                    //else if (key == "material") { //todo: implement this somehow
                    //    MATERIAL_NAME = value;
                    //    TEXTURE = ""; 
                    //}
                    else if (key == "texture") {
                        TEXTURE += value;
                        auto ext = boost::filesystem::extension(value);
                        MATERIAL_NAME = value.substr(0, value.size() - ext.size());
                    }
                    else if (key == "mesh") {
                        MESH += value;
                        auto ext = boost::filesystem::extension(value);
                        MESH_NAME = value.substr(0, value.size() - ext.size());
                    }
                }
                const float x = static_cast<float>(X);
                const float y = static_cast<float>(Y);
                const float z = static_cast<float>(Z);
                if (!MATERIAL_NAME.empty()) {
                    string extention  = boost::filesystem::extension(TEXTURE);
                    string normalFile = (TEXTURE.substr(0, TEXTURE.size() - extention.size())) + "_Normal" + extention;
                    string glowFile   = (TEXTURE.substr(0, TEXTURE.size() - extention.size())) + "_Glow" + extention;
                    if (!boost::filesystem::exists(normalFile)) {
                        normalFile = "";
                    }
                    if (!boost::filesystem::exists(glowFile)) {
                        glowFile = "";
                    }
                    if (!loadedMaterials.count(MATERIAL_NAME)) {
                        Handle material_handle = Resources::loadMaterial(MATERIAL_NAME, TEXTURE, normalFile, glowFile, "", "", "", "");
                        loadedMaterials.emplace(MATERIAL_NAME, material_handle);
                    }
                }
                if (!MESH_NAME.empty()) {
                    if (!loadedMeshes.count(MESH_NAME)) {
                        Handle mesh_handle = Resources::loadMesh(MESH)[0];
                        loadedMeshes.emplace(MESH_NAME, mesh_handle);
                    }
                }


                if (line[0] == 'S') {//Star
                    Star* star = new Star(loadedMeshes.at(MESH_NAME), loadedMaterials.at(MATERIAL_NAME), glm::vec3(R, G, B), glm::vec3(R1, G1, B1), glm::vec3(R2, G2, B2), glm::vec3(x, y, z), static_cast<float>(RADIUS), NAME, this);
                    if (!PARENT.empty()) {
                        star->setPosition(m_Planets.at(PARENT)->getPosition() + star->getPosition());
                    }
                    m_Planets.emplace(NAME, star);
                    internalCreateAnchor("Root", NAME, loadedAnchors, star->getPosition());
                }else if (line[0] == 'P') {//Planet
                    planetoid = new Planet(loadedMeshes.at(MESH_NAME), loadedMaterials.at(MATERIAL_NAME), TYPE, glm::vec3(x, y, z), static_cast<float>(RADIUS), NAME, ATMOSPHERE_HEIGHT, this);
                    if (!PARENT.empty()) {
                        Planet* parent = m_Planets.at(PARENT);
                        planetoid->setPosition(planetoid->getPosition() + parent->getPosition());
                        if (ORBIT_PERIOD != -1.0f) {
                            planetoid->setOrbit(new OrbitInfo(ORBIT_ECCENTRICITY, ORBIT_PERIOD, static_cast<float>(ORBIT_MAJOR_AXIS), Angle, *parent, INCLINATION));
                        }
                        if (ROTATIONAL_TILT != -1.0f) {
                            planetoid->setRotation(new RotationInfo(ROTATIONAL_TILT, ROTATIONAL_PERIOD));
                        }
                    }
                    m_Planets.emplace(NAME, planetoid);
                    internalCreateAnchor(PARENT, NAME, loadedAnchors, planetoid->getPosition());
                }else if (line[0] == 'M') {//Moon
                    planetoid = new Planet(loadedMeshes.at(MESH_NAME), loadedMaterials.at(MATERIAL_NAME), TYPE, glm::vec3(x, y, z), static_cast<float>(RADIUS), NAME, ATMOSPHERE_HEIGHT, this);
                    if (!PARENT.empty()) {
                        Planet* parent = m_Planets.at(PARENT);
                        planetoid->setPosition(planetoid->getPosition() + parent->getPosition());
                        if (ORBIT_PERIOD != -1.0f) {
                            planetoid->setOrbit(new OrbitInfo(ORBIT_ECCENTRICITY, ORBIT_PERIOD, static_cast<float>(ORBIT_MAJOR_AXIS), Angle, *parent, INCLINATION));
                        }
                        if (ROTATIONAL_TILT != -1.0f) {
                            planetoid->setRotation(new RotationInfo(ROTATIONAL_TILT, ROTATIONAL_PERIOD));
                        }
                    }
                    m_Planets.emplace(NAME, planetoid);
                    internalCreateAnchor(PARENT, NAME, loadedAnchors, planetoid->getPosition());
                }else if (line[0] == '?') {//Anchor (Primary Spawn) point
                    const auto& parentPos = m_Planets.at(PARENT)->getPosition();
                    auto spawnAnchor = internalCreateAnchor(PARENT, "Spawn", loadedAnchors, parentPos.x + x, parentPos.y + y, parentPos.z + z);
                    m_SpawnAnchors.push_back( std::make_tuple("Spawn Anchor", spawnAnchor) );
                }else if (line[0] == '>') {//Anchor (Secondary Spawn) point
                    const auto& parentPos = m_Planets.at(PARENT)->getPosition();
                    auto spawnAnchor = internalCreateAnchor(PARENT, PARENT + " Spawn", loadedAnchors, parentPos.x + x, parentPos.y + y, parentPos.z + z);
                    m_SpawnAnchors.push_back(std::make_tuple(PARENT + " Spawn Anchor", spawnAnchor));

                }else if (line[0] == 'R') {//Rings
                    if (!PARENT.empty()) {
                        if (!planetRings.count(PARENT)) {
                            vector<RingInfo> rings;
                            planetRings.emplace(PARENT, rings);
                        }
                        planetRings.at(PARENT).push_back(RingInfo(static_cast<uint>(POSITION) / 10, static_cast<uint>(RADIUS) / 10, glm::ivec3(R, G, B), BREAK));
                    }
                }else if (line[0] == 'L') {//Lagrange Point               
                }
            }
        }
        ++count;
    }
    //add planetary rings
    for (auto& rings : planetRings) {
        new Ring(rings.second, m_Planets.at(rings.first));
    }

    centerSceneToObject(loadedAnchors.at("Spawn Anchor")->entity());

    setGlobalIllumination(gi_global, gi_diffuse, gi_specular);
}
Ship* Map::createShip(const AIType::Type ai_type, Team& team, Client& client, const string& shipClass, const string& shipName, const glm::vec3& position) {
    if ((ai_type == AIType::Player_You || ai_type == AIType::Player_Other) && m_ShipsPlayerControlled.count(shipName))
        return nullptr;
    Ship* ship = nullptr;

    if     (shipClass == "Defiant")
        ship = new Defiant(ai_type, team, client, *this, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull);
    else if(shipClass == "Nova")
        ship = new Nova(ai_type, team, client, *this, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull);
    else if(shipClass == "Shrike")
        ship = new Shrike(ai_type, team, client, *this, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull);
    else if(shipClass == "B'rel")
        ship = new Brel(ai_type, team, client, *this, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull);
    else if(shipClass == "Constitution")
        ship = new Constitution(ai_type, team, client, *this, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull);
    else if(shipClass == "Miranda")
        ship = new Miranda(ai_type, team, client, *this, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull);
    else if(shipClass == "Excelsior")
        ship = new Excelsior(ai_type, team, client, *this, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull);
    else if (shipClass == "Akira")
        ship = new Akira(ai_type, team, client, *this, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull);
    else if (shipClass == "Norway")
        ship = new Norway(ai_type, team, client, *this, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull);
    else if (shipClass == "Intrepid")
        ship = new Intrepid(ai_type, team, client, *this, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull);
    else if (shipClass == "Vor'cha")
        ship = new Vorcha(ai_type, team, client, *this, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull);
    return ship;
}
Anchor* Map::getRootAnchor() {
    return std::get<1>(m_RootAnchor);
}
Anchor* Map::getSpawnAnchor() {
    return std::get<1>(m_SpawnAnchors[0]);
}
const bool Map::hasShipPlayer(const string& shipName) const {
    return (m_ShipsPlayerControlled.size() > 0 && m_ShipsPlayerControlled.count(shipName)) ? true : false;
}
const bool Map::hasShipNPC(const string& shipName) const {
    return (m_ShipsNPCControlled.size() > 0 && m_ShipsNPCControlled.count(shipName)) ? true : false;
}
const bool Map::hasShip(const string& shipName) const {
    return (m_Ships.size() > 0 && m_Ships.count(shipName)) ? true : false;
}

HUD& Map::getHUD() {
    return *m_HUD;
}
Anchor* Map::getSpawnAnchor(const string& spawn_anchor_name) {
    for (auto& tuple : m_SpawnAnchors) {
        auto& name = std::get<0>(tuple);
        if (name == spawn_anchor_name)
            return std::get<1>(tuple);
    }
    return nullptr;
}
const string Map::getClosestSpawnAnchor() {
    string ret = "";
    decimal minDist = static_cast<decimal>(-1.0);
    auto playerPos = m_Player->getComponent<ComponentBody>()->position();
    for (auto& spawn_anchor_tuple : m_SpawnAnchors) {
        auto& spawn_anchor = *std::get<1>(spawn_anchor_tuple);
        const auto dist = glm::distance(playerPos, spawn_anchor.getPosition());
        if (minDist < static_cast<decimal>(0.0) || dist < minDist) {
            minDist = dist;
            ret = std::get<0>(spawn_anchor_tuple);
        }
    }
    return ret;
}
const vector<string> Map::getClosestAnchor(Anchor* currentAnchor) {
    vector<string> res;
    if (!currentAnchor) {
        currentAnchor = std::get<1>(m_RootAnchor);
    }
    decimal minDist = static_cast<decimal>(-1.0);
    auto playerPos = m_Player->getComponent<ComponentBody>()->position();
    while (currentAnchor->m_Children.size() > 0) {
        string least = currentAnchor->m_Children.begin()._Ptr->_Myval.first;
        bool hasChanged = false;
        for (auto& child : currentAnchor->m_Children) {
            auto childPos = child.second->getPosition();
            const auto dist = glm::distance(playerPos, childPos);
            if (minDist < static_cast<decimal>(0.0) || dist < minDist) {
                minDist = dist;
                least = child.first;
                currentAnchor = child.second;
                hasChanged = true;
            }
        }
        if (!hasChanged)
            break;
        res.push_back(least);
    }
    return res;
}
void Map::onResize(const unsigned int& width, const unsigned int& height) {
    m_HUD->onResize(width, height);
    Scene::onResize(width, height);
}
void Map::update(const double& dt){
    //torpedos
    auto& torps = m_ActiveTorpedoProjectiles.data();
    for (uint i = 0; i < torps.size(); ++i) {
        auto* t = torps[i];
        if (t && t->active) {
            t->update(dt);
            if (!t->active) {
                m_ActiveTorpedoProjectiles.delete_data_index(i);
            }
        }
    }
    //cannons
    auto& cannons = m_ActiveCannonProjectiles.data();
    for (uint i = 0; i < cannons.size(); ++i) {
        auto* c = cannons[i];
        if (c && c->active) {
            c->update(dt);
            if (!c->active) {
                m_ActiveCannonProjectiles.delete_data_index(i);
            }
        }
    }
    for (auto& ship : m_ShipsPlayerControlled)
        ship.second->update(dt);
    for (auto& ship : m_ShipsNPCControlled)
        ship.second->update(dt);

    m_HUD->update(dt);
    Scene::update(dt);
}
void Map::render() {
    m_HUD->render();
    Scene::render();
}
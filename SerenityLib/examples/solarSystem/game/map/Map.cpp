#include "Map.h"
#include "Anchor.h"
#include "../ResourceManifest.h"
#include "../Planet.h"
#include "../GameCamera.h"
#include "../Ship.h"
#include "../GameSkybox.h"
#include "../Helper.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/lights/Lights.h>
#include <core/ModelInstance.h>

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
#include <core/ModelInstance.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>

#include "core/Terrain.h"
#include "../ships/Ships.h"
#include <SFML/Graphics.hpp>

using namespace Engine;
using namespace std;
namespace boost_io = boost::iostreams;

Map::Map(const string& n, const string& file):Scene(n){
    m_Player      = nullptr; 
    GameCamera* playerCamera = new GameCamera(0.35f,7000000000.0f,this);
    setActiveCamera(*playerCamera);
    m_Objects.push_back(playerCamera);
    m_Filename = file;
    m_SkyboxFile = "";
    m_RootAnchor = std::tuple<std::string,Anchor*>("",nullptr);
    //m_oldClientPos = m_oldAnchorPos = glm::vec3(0.0f);
    if(file != "NULL")
        Map::loadFromFile(file);
}
Map::~Map(){
    SAFE_DELETE_VECTOR(m_Objects);
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
Anchor* Map::internalCreateDeepspaceAnchor(const float& x, const float& y, const float& z, const string& name) {
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
Anchor* Map::internalCreateAnchor(const string& parentAnchor, const string& thisName, unordered_map<string, Anchor*>& loadedAnchors, const float& x, const float& y, const float& z) {
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
Anchor* Map::internalCreateAnchor(const string& parentAnchor, const string& thisName, unordered_map<string, Anchor*>& loadedAnchors, const glm::vec3& position) {
    return internalCreateAnchor(parentAnchor, thisName, loadedAnchors, position.x, position.y, position.z);
}
void Map::loadFromFile(const string& filename) {
    uint                                             count = 0;
    boost_io::stream<boost_io::mapped_file_source>   str(filename);
    unordered_map<string, vector<RingInfo>>          planetRings;
    unordered_map<string, Handle>                    loadedMaterials;
    unordered_map<string, Anchor*>                   loadedAnchors;

    float gi_diffuse  = 1.0f;
    float gi_specular = 1.0f;
    float gi_global   = 1.0f;

    internalCreateAnchor("", "Root", loadedAnchors, 0.0f, 0.0f, 0.0f);

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
                float R, G, B, R1, G1, B1, R2, G2, B2, qx, qy, qz;
                qx = qy = qz = 0.0f;
                float ATMOSPHERE_HEIGHT = 0.0f;
                string TYPE;
                string TEXTURE = ResourceManifest::BasePath + "data/Textures/Planets/";
                string MATERIAL_NAME = "";

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
                float     A = 0;

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
                    else if (key == "a")                A = stof(value);

                    else if (key == "parent")           PARENT = value;
                    else if (key == "type")             TYPE = value;
                    else if (key == "atmosphereHeight") ATMOSPHERE_HEIGHT = stof(value);
                    else if (key == "break")            BREAK = stoi(value);
                    else if (key == "eccentricity")     ORBIT_ECCENTRICITY = stof(value);
                    else if (key == "period")           ORBIT_PERIOD = stof(value);
                    else if (key == "majorAxis")        ORBIT_MAJOR_AXIS = stoll(value) * 10;
                    else if (key == "days")             ROTATIONAL_PERIOD = stof(value);
                    else if (key == "tilt")             ROTATIONAL_TILT = stof(value);
                    else if (key == "inclination")      INCLINATION = stof(value);
                    else if (key == "material") { MATERIAL_NAME = value; TEXTURE = ""; } //todo: implement this somehow
                    else if (key == "texture") {
                        TEXTURE += value;
                        auto ext = boost::filesystem::extension(value);
                        MATERIAL_NAME = value.substr(0, value.size() - ext.size());
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
                        Handle handle = Resources::addMaterial(MATERIAL_NAME, TEXTURE, normalFile, glowFile);
                        loadedMaterials.emplace(MATERIAL_NAME, handle);
                    }
                }
                if (line[0] == 'S') {//Star
                    Star* star = new Star(glm::vec3(R, G, B), glm::vec3(R1, G1, B1), glm::vec3(R2, G2, B2), glm::vec3(x, y, z), static_cast<float>(RADIUS), NAME, this);
                    if (!PARENT.empty()) {
                        star->setPosition(m_Planets.at(PARENT)->getPosition() + star->getPosition());
                    }
                    m_Planets.emplace(NAME, star);
                    internalCreateAnchor("Root", NAME, loadedAnchors, star->getPosition());
                }else if (line[0] == 'P') {//Planet
                    PlanetType::Type PLANET_TYPE;
                    if      (TYPE == "Rock")     PLANET_TYPE = PlanetType::Rocky;
                    else if (TYPE == "Ice")      PLANET_TYPE = PlanetType::Ice;
                    else if (TYPE == "GasGiant") PLANET_TYPE = PlanetType::GasGiant;
                    else if (TYPE == "IceGiant") PLANET_TYPE = PlanetType::IceGiant;
                    else if (TYPE == "Asteroid") PLANET_TYPE = PlanetType::Asteroid;
                    planetoid = new Planet(loadedMaterials.at(MATERIAL_NAME), PLANET_TYPE, glm::vec3(x, y, z), static_cast<float>(RADIUS), NAME, ATMOSPHERE_HEIGHT, this);
                    if (!PARENT.empty()) {
                        Planet* parent = m_Planets.at(PARENT);
                        planetoid->setPosition(planetoid->getPosition() + parent->getPosition());
                        if (ORBIT_PERIOD != -1.0f) {
                            planetoid->setOrbit(new OrbitInfo(ORBIT_ECCENTRICITY, ORBIT_PERIOD, static_cast<float>(ORBIT_MAJOR_AXIS), A, *parent, INCLINATION));
                        }
                        if (ROTATIONAL_TILT != -1.0f) {
                            planetoid->setRotation(new RotationInfo(ROTATIONAL_TILT, ROTATIONAL_PERIOD));
                        }
                    }
                    m_Planets.emplace(NAME, planetoid);
                    internalCreateAnchor(PARENT, NAME, loadedAnchors, planetoid->getPosition());
                }else if (line[0] == 'M') {//Moon
                    PlanetType::Type PLANET_TYPE;
                    if      (TYPE == "Rock")     PLANET_TYPE = PlanetType::Rocky;
                    else if (TYPE == "Ice")      PLANET_TYPE = PlanetType::Ice;
                    else if (TYPE == "GasGiant") PLANET_TYPE = PlanetType::GasGiant;
                    else if (TYPE == "IceGiant") PLANET_TYPE = PlanetType::IceGiant;
                    else if (TYPE == "Asteroid") PLANET_TYPE = PlanetType::Asteroid;
                    planetoid = new Planet(loadedMaterials.at(MATERIAL_NAME), PLANET_TYPE, glm::vec3(x, y, z), static_cast<float>(RADIUS), NAME, ATMOSPHERE_HEIGHT, this);
                    if (!PARENT.empty()) {
                        Planet* parent = m_Planets.at(PARENT);
                        planetoid->setPosition(planetoid->getPosition() + parent->getPosition());
                        if (ORBIT_PERIOD != -1.0f) {
                            planetoid->setOrbit(new OrbitInfo(ORBIT_ECCENTRICITY, ORBIT_PERIOD, static_cast<float>(ORBIT_MAJOR_AXIS), A, *parent, INCLINATION));
                        }
                        if (ROTATIONAL_TILT != -1.0f) {
                            planetoid->setRotation(new RotationInfo(ROTATIONAL_TILT, ROTATIONAL_PERIOD));
                        }
                    }
                    m_Planets.emplace(NAME, planetoid);
                    internalCreateAnchor(PARENT, NAME, loadedAnchors, planetoid->getPosition());
                }else if (line[0] == '?') {//Anchor (Spawn) point
                    const auto& parentPos = m_Planets.at(PARENT)->getPosition();
                    auto spawnAnchor = internalCreateAnchor(PARENT, "Spawn", loadedAnchors, parentPos.x + x, parentPos.y + y, parentPos.z + z);
                    m_SpawnAnchor = std::tuple<std::string, Anchor*>("Spawn Anchor", spawnAnchor);
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

    std::get<1>(m_RootAnchor)->finalize_all();

    setGlobalIllumination(gi_global, gi_diffuse, gi_specular);
}
Ship* Map::createShip(Client& client, const string& shipClass, const string& shipName, const bool& playerShip, const glm::vec3& position) {
    if (m_Ships.size() > 0 && m_Ships.count(shipName))
        return nullptr;
    auto& handles = ResourceManifest::Ships[shipClass];
    Ship* ship = nullptr;

    if     (shipClass == "Defiant")
        ship = new Defiant(client, playerShip, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull, this);
    else if(shipClass == "Nova")
        ship = new Nova(client, playerShip, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull, this);
    else if(shipClass == "Shrike")
        ship = new Shrike(client, playerShip, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull, this);
    else if(shipClass == "Brel")
        ship = new Brel(client, playerShip, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull, this);
    else if(shipClass == "Constitution")
        ship = new Constitution(client, playerShip, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull, this);
    else if(shipClass == "Miranda")
        ship = new Miranda(client, playerShip, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull, this);
    else if(shipClass == "Excelsior")
        ship = new Excelsior(client, playerShip, shipName, position, glm::vec3(1.0f), CollisionType::ConvexHull, this);

    return ship;
}
Anchor* Map::getRootAnchor() {
    return std::get<1>(m_RootAnchor);
}
Anchor* Map::getSpawnAnchor() {
    return std::get<1>(m_SpawnAnchor);
}
const bool Map::hasShip(const string& shipName) const {
    return (m_Ships.size() > 0 && m_Ships.count(shipName)) ? true : false;
}
const vector<string> Map::getClosestAnchor(Anchor* currentAnchor) {
    vector<string> res;
    if (!currentAnchor) {
        currentAnchor = std::get<1>(m_RootAnchor);
    }
    float minDist = -1.0f;
    while (currentAnchor->m_Children.size() > 0) {
        string least = currentAnchor->m_Children.begin()._Ptr->_Myval.first;
        bool hasChanged = false;
        for (auto& child : currentAnchor->m_Children) {
            auto childPos = child.second->getPosition();
            auto playerPos = m_Player->getComponent<ComponentBody>()->position();
            const float dist = glm::distance(playerPos, childPos);
            if (minDist < 0 || dist < minDist) {
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
void Map::update(const double& dt){
    Scene::update(dt);
}

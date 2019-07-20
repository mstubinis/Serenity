#include "SolarSystem.h"
#include "ResourceManifest.h"
#include "Planet.h"
#include "GameCamera.h"
#include "Ship.h"
#include "GameSkybox.h"

#include <core/engine/events/Engine_Events.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/lights/Lights.h>
#include <core/MeshInstance.h>

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
#include <core/MeshInstance.h>
#include <core/engine/mesh/Mesh.h>
#include <core/Material.h>

#include "core/Terrain.h"
#include <SFML/Graphics.hpp>

using namespace Engine;
using namespace std;
namespace boost_io = boost::iostreams;

SolarSystem::SolarSystem(const string& n, const string& file):Scene(n){
    m_Player      = nullptr;
    m_AnchorPoint = nullptr;
    GameCamera* playerCamera = new GameCamera(0.35f,7000000000.0f,this);
    setActiveCamera(*playerCamera);
    m_Objects.push_back(playerCamera);
    m_Filename = file;
    if(file != "NULL")
        SolarSystem::loadFromFile(file);

    m_AnchorPoint = new EntityWrapper(*this);
    auto& anchor_body = *m_AnchorPoint->entity().addComponent<ComponentBody>();
    anchor_body.setScale(0.005f);
}
SolarSystem::~SolarSystem(){
    SAFE_DELETE_VECTOR(m_Objects);
}
vector<string> SolarSystem::allowedShips() {
    vector<string> result;
    uint                                             count = 0;
    boost_io::stream<boost_io::mapped_file_source>   str(m_Filename);
    for (string line; getline(str, line, '\n');) {
        line.erase(remove(line.begin(), line.end(), '\r'), line.end()); //remove \r from the line
        if (line[0] != '#') {//ignore commented lines
            if (count == 5) {//this line has the allowed ships
                stringstream ss(line);
                while (ss.good()) {
                    string substr;
                    getline(ss, substr, ',');
                    result.push_back(substr);
                }
                return result;
            }
        }
        ++count;
    }
    return result;
}
void SolarSystem::loadFromFile(const string& filename) {
    uint                                             count = 0;
    boost_io::stream<boost_io::mapped_file_source>   str(filename);
    unordered_map<string, vector<RingInfo>>          planetRings;
    unordered_map<string, Handle>                    loadedMaterials;

    float gi_diffuse  = 1.0f;
    float gi_specular = 1.0f;
    float gi_global   = 1.0f;

    string skyboxDirectory;
    for (string line; getline(str, line, '\n');) {
        line.erase(remove(line.begin(), line.end(), '\r'), line.end()); //remove \r from the line
        if (line[0] != '#') {//ignore commented lines
            if (count == 1) {//this line has the system's name
                setName(line);
            }else if (count == 2) {//this line has the system's skybox
                skyboxDirectory = ResourceManifest::BasePath + line;
            }else if (count == 3) {//this line has the system's skybox's number of flares
                GameSkybox* box = new GameSkybox(skyboxDirectory, boost::lexical_cast<uint>(line), this);
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
            if ((line[0] == 'S' || line[0] == 'M' || line[0] == 'P' || line[0] == '*' || line[0] == 'R' || line[0] == '$' || line[0] == 'L' || line[0] == 's') && line[1] == ' ') {//we got something to work with
                Planet* planetoid = nullptr;

                string token;
                istringstream stream(line);

                string NAME;
                string LAGRANGE_TYPE;
                string LAGRANGE_PLANET_1, LAGRANGE_PLANET_2;
                string PARENT = "";
                float R, G, B, R1, G1, B1, R2, G2, B2;
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
                    else if (key == "a")                A = stoull(value);

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
                float x = static_cast<float>(X);
                float y = static_cast<float>(Y);
                float z = static_cast<float>(Z);
                if (MATERIAL_NAME != "") {
                    string extention = boost::filesystem::extension(TEXTURE);
                    string normalFile = (TEXTURE.substr(0, TEXTURE.size() - extention.size())) + "_Normal" + extention;
                    string glowFile = (TEXTURE.substr(0, TEXTURE.size() - extention.size())) + "_Glow" + extention;
                    if (!boost::filesystem::exists(normalFile)) {
                        normalFile = "";
                    }
                    if (!boost::filesystem::exists(glowFile)) {
                        glowFile = "";
                    }
                    if (!loadedMaterials.count(MATERIAL_NAME)) {
                        Handle handle = Resources::addMaterial(MATERIAL_NAME, TEXTURE, normalFile, glowFile, "");
                        loadedMaterials.emplace(MATERIAL_NAME, handle);
                    }
                }
                if (line[0] == 'S') {//Star
                    Star* star = new Star(glm::vec3(R, G, B), glm::vec3(R1, G1, B1), glm::vec3(R2, G2, B2), glm::vec3(0), static_cast<float>(RADIUS), NAME, this);
                    if (PARENT != "") {
                        star->setPosition(m_Planets.at(PARENT)->getPosition() + glm::vec3(x, y, z));
                    }
                    m_Planets.emplace(NAME, star);
                }else if (line[0] == 'P') {//Planet
                    PlanetType::Type PLANET_TYPE;
                    if      (TYPE == "Rock")     PLANET_TYPE = PlanetType::Rocky;
                    else if (TYPE == "Ice")      PLANET_TYPE = PlanetType::Ice;
                    else if (TYPE == "GasGiant") PLANET_TYPE = PlanetType::GasGiant;
                    else if (TYPE == "IceGiant") PLANET_TYPE = PlanetType::IceGiant;
                    else if (TYPE == "Asteroid") PLANET_TYPE = PlanetType::Asteroid;
                    planetoid = new Planet(loadedMaterials.at(MATERIAL_NAME), PLANET_TYPE, glm::vec3(x, y, z), static_cast<float>(RADIUS), NAME, ATMOSPHERE_HEIGHT, this);
                    if (PARENT != "") {
                        Planet* parent = m_Planets.at(PARENT);
                        planetoid->setPosition(planetoid->getPosition() + parent->getPosition());
                        if (ORBIT_PERIOD != -1.0f) {
                            planetoid->setOrbit(new OrbitInfo(ORBIT_ECCENTRICITY, ORBIT_PERIOD, (float)ORBIT_MAJOR_AXIS, A, *parent, INCLINATION));
                        }
                        if (ROTATIONAL_TILT != -1.0f) {
                            planetoid->setRotation(new RotationInfo(ROTATIONAL_TILT, ROTATIONAL_PERIOD));
                        }
                    }
                    m_Planets.emplace(NAME, planetoid);
                }else if (line[0] == 'M') {//Moon
                    PlanetType::Type PLANET_TYPE;
                    if      (TYPE == "Rock")     PLANET_TYPE = PlanetType::Rocky;
                    else if (TYPE == "Ice")      PLANET_TYPE = PlanetType::Ice;
                    else if (TYPE == "GasGiant") PLANET_TYPE = PlanetType::GasGiant;
                    else if (TYPE == "IceGiant") PLANET_TYPE = PlanetType::IceGiant;
                    else if (TYPE == "Asteroid") PLANET_TYPE = PlanetType::Asteroid;
                    planetoid = new Planet(loadedMaterials.at(MATERIAL_NAME), PLANET_TYPE, glm::vec3(x, y, z), static_cast<float>(RADIUS), NAME, ATMOSPHERE_HEIGHT, this);
                    if (PARENT != "") {
                        Planet* parent = m_Planets.at(PARENT);
                        planetoid->setPosition(planetoid->getPosition() + parent->getPosition());
                        if (ORBIT_PERIOD != -1.0f) {
                            planetoid->setOrbit(new OrbitInfo(ORBIT_ECCENTRICITY, ORBIT_PERIOD, (float)ORBIT_MAJOR_AXIS, A, *parent, INCLINATION));
                        }
                        if (ROTATIONAL_TILT != -1.0f) {
                            planetoid->setRotation(new RotationInfo(ROTATIONAL_TILT, ROTATIONAL_PERIOD));
                        }
                    }
                    m_Planets.emplace(NAME, planetoid);
                }else if (line[0] == '*') {//Player ship
                    m_Player = new Ship(ResourceManifest::DefiantMesh, ResourceManifest::DefiantMaterial, true, NAME, glm::vec3(x, y, z), glm::vec3(1.0f), CollisionType::ConvexHull, this);
                    if (PARENT != "") {
                        Planet* parent = m_Planets.at(PARENT);
                        auto& cbody = *m_Player->entity().getComponent<ComponentBody>();
                        cbody.setPosition(cbody.position() + parent->getPosition());
                    }
                    GameCamera* playerCamera = (GameCamera*)getActiveCamera();
                    playerCamera->follow(m_Player->entity());
                }else if (line[0] == 'R') {//Rings
                    if (PARENT != "") {
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
    centerSceneToObject(m_Player->entity());

    setGlobalIllumination(gi_global, gi_diffuse, gi_specular);

    //new Ship(ResourceManifest::DefiantMesh, ResourceManifest::DefiantMaterial, false, "test", glm::vec3(-4, 0, 4), glm::vec3(1.0f), CollisionType::ConvexHull, this);
}



void SolarSystem::update(const double& dt){
    Scene::update(dt);
}

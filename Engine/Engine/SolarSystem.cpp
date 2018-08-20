#include "Engine_Events.h"
#include "Engine_Renderer.h"
#include "Engine_Resources.h"
#include "Light.h"

#include "SolarSystem.h"
#include "ResourceManifest.h"
#include "Planet.h"
#include "GameCamera.h"
#include "Ship.h"
#include "GameSkybox.h"

#include <algorithm>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace Engine;
using namespace std;


SolarSystem::SolarSystem(string n, string file):Scene(n){
    GameCamera* playerCamera = new GameCamera(60,Resources::getWindowSize().x/(float)Resources::getWindowSize().y,0.1f,7000000000.0f,this);
    setActiveCamera(playerCamera);
	giGlobal = giSpecular = giDiffuse = 1.0f;
    if(file != "NULL")
        SolarSystem::_loadFromFile(file);	
}
SolarSystem::~SolarSystem(){

}
void SolarSystem::_loadFromFile(string filename){
    uint count = 0;
    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
    unordered_map<string,vector<RingInfo>> planetRings;

    unordered_map<string,Handle> loadedMaterials;

    string skyboxDirectory;
    for(string line; getline(str, line, '\n');){
        line.erase( remove(line.begin(), line.end(), '\r'), line.end() ); //remove \r from the line
        if(line[0] != '#'){//ignore commented lines
            if(count == 1){//this line has the system's name
                setName(line);
            }
            else if(count == 2){//this line has the system's skybox
                skyboxDirectory = line;
            }
            else if(count == 3){//this line has the system's skybox's number of flares
                GameSkybox* box = new GameSkybox(skyboxDirectory,boost::lexical_cast<uint>(line),this);
            }
            else if(count == 4){//this line has the system's GI contribution
                string token;
                istringstream stream(line);
                while(getline(stream, token, ' ')) {
                    size_t pos = token.find("=");

                    string key = token.substr(0, pos);
                    string value = token.substr(pos + 1,string::npos);

                         if(key == "giDiffuse")   giDiffuse = stof(value);
                    else if(key == "giSpecular")  giSpecular = stof(value);
                    else if(key == "giGlobal")    giGlobal = stof(value);

                }
            }
            if((line[0] == 'S' || line[0] == 'M' || line[0] == 'P' || line[0] == '*' || line[0] == 'R' || line[0] == '$' || line[0] == 'L' || line[0] == 's') && line[1] == ' '){//we got something to work with
                Planet* planetoid = nullptr;

                string token;
                istringstream stream(line);

                string NAME;
                string LAGRANGE_TYPE;
                string LAGRANGE_PLANET_1, LAGRANGE_PLANET_2;
                string PARENT = "";
                float R,G,B,   R1,G1,B1;
                float ATMOSPHERE_HEIGHT;
                string LIGHTCOLOR;
                string TYPE;
                string TEXTURE = "data/Textures/Planets/";
                string MATERIAL_NAME = "";

                float ORBIT_PERIOD = -1;
                unsigned long long ORBIT_MAJOR_AXIS = -1;
                float ORBIT_ECCENTRICITY = -1;
                float INCLINATION = 0.0f;

                float ROTATIONAL_TILT = -1;
                float ROTATIONAL_PERIOD = -1;

                unsigned long long RADIUS = 0;
                unsigned long long POSITION = 0;
                uint BREAK = 0;

                while(getline(stream, token, ' ')) {
                    size_t pos = token.find("=");

                    string key = token.substr(0, pos);
                    string value = token.substr(pos + 1,string::npos);

                    if(key == "name"){                  
                        NAME = value;
                        replace(NAME.begin(),NAME.end(),'_',' ');
                    }
                    else if(key == "lp1")              LAGRANGE_PLANET_1 = value;
                    else if(key == "lp2")              LAGRANGE_PLANET_2 = value;
                    else if(key == "lType")            LAGRANGE_TYPE = value;
                    else if(key == "radius")           RADIUS = stoull(value)*10;
                    else if(key == "r")                R = stof(value);
                    else if(key == "g")                G = stof(value);
                    else if(key == "b")                B = stof(value);
                    else if(key == "r1")               R1 = stof(value);
                    else if(key == "g1")               G1 = stof(value);
                    else if(key == "b1")               B1 = stof(value);
                    else if(key == "position")         POSITION = stoull(value)*10; 
                    else if(key == "parent")           PARENT = value;
                    else if(key == "type")             TYPE = value;
                    else if(key == "atmosphereHeight") ATMOSPHERE_HEIGHT = stof(value);
                    else if(key == "break")            BREAK = stoi(value);
                    else if(key == "eccentricity")     ORBIT_ECCENTRICITY = stof(value);
                    else if(key == "period")           ORBIT_PERIOD = stof(value);
                    else if(key == "majorAxis")        ORBIT_MAJOR_AXIS = stoll(value)*10;
                    else if(key == "days")             ROTATIONAL_PERIOD = stof(value);
                    else if(key == "tilt")             ROTATIONAL_TILT = stof(value);
                    else if(key == "inclination")      INCLINATION = stof(value);
                    else if(key == "material"){        MATERIAL_NAME = value; TEXTURE = ""; } //todo: implement this somehow
                    else if(key == "texture"){    
                        TEXTURE += value;
                        MATERIAL_NAME = value.substr(0,value.size()-4);
                    }

                }
                float randAngle = float(rand() % 3600);
                randAngle /= 10.0f;
                randAngle *= 3.14159f / 180.0f;
                float xPos = glm::cos(randAngle) * float(POSITION);
                float zPos = glm::sin(randAngle) * float(POSITION);

                if(MATERIAL_NAME != ""){
                    string extention = boost::filesystem::extension(TEXTURE);
                    string normalFile = (TEXTURE.substr(0,TEXTURE.size()-extention.size())) + "_Normal" + extention;
                    string glowFile = (TEXTURE.substr(0,TEXTURE.size()-extention.size())) + "_Glow" + extention;
                    if(!boost::filesystem::exists(normalFile)){
                        normalFile = "";
                    }
                    if(!boost::filesystem::exists(glowFile)){
                        glowFile = "";
                    }
                    if(!loadedMaterials.count(MATERIAL_NAME)){
                        Handle handle;
                        handle = Resources::addMaterial(MATERIAL_NAME,TEXTURE,normalFile,glowFile,"",ResourceManifest::groundFromSpace);
                        loadedMaterials.emplace(MATERIAL_NAME,handle);
                    }
                }
                if(line[0] == 'S'){//Sun
                    Star* star = new Star(glm::vec3(R,G,B),glm::vec3(R1,G1,B1),glm::vec3(0),(float)RADIUS,NAME,this);
                    if(PARENT != ""){
                        star->setPosition(m_Planets.at(PARENT)->getPosition()+glm::vec3(xPos,0,zPos));
                    }
                    m_Planets.emplace(NAME,star);
                }
                else if(line[0] == 'P'){//Planet
                    PlanetType PLANET_TYPE;
                    if(TYPE == "Rock") PLANET_TYPE = PLANET_TYPE_ROCKY;
                    else if(TYPE == "Ice") PLANET_TYPE = PLANET_TYPE_ICE;
                    else if(TYPE == "GasGiant") PLANET_TYPE = PLANET_TYPE_GAS_GIANT;
                    else if(TYPE == "IceGiant") PLANET_TYPE = PLANET_TYPE_ICE_GIANT;
                    else if(TYPE == "Asteroid") PLANET_TYPE = PLANET_TYPE_ASTEROID;
                    planetoid = new Planet(loadedMaterials.at(MATERIAL_NAME),PLANET_TYPE,glm::vec3(xPos,0,zPos),(float)RADIUS,NAME,ATMOSPHERE_HEIGHT,this);
                    if(PARENT != ""){
                        Planet* parent = m_Planets.at(PARENT);
                        planetoid->setPosition(planetoid->getPosition() + parent->getPosition());

                        if(ORBIT_PERIOD != -1.0f){
                            planetoid->setOrbit(new OrbitInfo(ORBIT_ECCENTRICITY,ORBIT_PERIOD,(float)ORBIT_MAJOR_AXIS,randAngle,parent->id(),INCLINATION));
                        }
                        if(ROTATIONAL_TILT != -1.0f){
                            planetoid->setRotation(new RotationInfo(ROTATIONAL_TILT,ROTATIONAL_PERIOD));
                        }
                    }
                    m_Planets.emplace(NAME,planetoid);
                }
                else if(line[0] == 'M'){//Moon
                    PlanetType PLANET_TYPE;
                    if(TYPE == "Rock") PLANET_TYPE = PLANET_TYPE_ROCKY;
                    else if(TYPE == "Ice") PLANET_TYPE = PLANET_TYPE_ICE;
                    else if(TYPE == "GasGiant") PLANET_TYPE = PLANET_TYPE_GAS_GIANT;
                    else if(TYPE == "IceGiant") PLANET_TYPE = PLANET_TYPE_ICE_GIANT;
                    else if(TYPE == "Asteroid") PLANET_TYPE = PLANET_TYPE_ASTEROID;
                    planetoid = new Planet(loadedMaterials.at(MATERIAL_NAME),PLANET_TYPE,glm::vec3(xPos,0,zPos),(float)RADIUS,NAME,ATMOSPHERE_HEIGHT,this);
                    if(PARENT != ""){
                        Planet* parent = m_Planets.at(PARENT);
                        planetoid->setPosition(planetoid->getPosition() + parent->getPosition());

                        if(ORBIT_PERIOD != -1.0f){
                            planetoid->setOrbit(new OrbitInfo(ORBIT_ECCENTRICITY,ORBIT_PERIOD,(float)ORBIT_MAJOR_AXIS,randAngle,parent->id(),INCLINATION));
                        }
                        if(ROTATIONAL_TILT != -1.0f){
                            planetoid->setRotation(new RotationInfo(ROTATIONAL_TILT,ROTATIONAL_PERIOD));
                        }
                    }
                    m_Planets.emplace(NAME,planetoid);
                    
                }
                else if(line[0] == '*'){//Player ship
                    if(PARENT != ""){
                        float parentX = m_Planets.at(PARENT)->getPosition().x;
                        float parentZ = m_Planets.at(PARENT)->getPosition().z;
                        xPos += parentX;
                        zPos += parentZ;
                    }
                    setPlayer(new Ship(ResourceManifest::DefiantMesh,ResourceManifest::DefiantMaterial,true,NAME,glm::vec3(xPos,0,zPos),glm::vec3(1),nullptr,this));
                    GameCamera* playerCamera = (GameCamera*)getActiveCamera();
                    playerCamera->follow(getPlayer());
                }
                else if(line[0] == '$'){//Other ship
                    if(PARENT != ""){
                        //float parentX = objects().at(PARENT)->getPosition().x;
                        //float parentZ = objects().at(PARENT)->getPosition().z;
                        //xPos += parentX;
                        //zPos += parentZ;
                    }
                    //new Ship(ResourceManifest::AkiraMesh,ResourceManifest::AkiraMaterial,false,NAME,glm::vec3(xPos,0,zPos),glm::vec3(1),nullptr,this);
                }
                else if(line[0] == 'R'){//Rings
                    if(PARENT != ""){
                        if(!planetRings.count(PARENT)){
                            vector<RingInfo> rings;
                            planetRings.emplace(PARENT,rings);
                        }
                        planetRings.at(PARENT).push_back(RingInfo((uint)POSITION/10,(uint)RADIUS/10,glm::uvec3(R,G,B),BREAK));
                    }
                }
                else if(line[0] == 'L'){//Lagrange Point
                    
                }
            }
        }
        ++count;
    }

    //add planetary rings
    for(auto rings:planetRings){
        new Ring(rings.second,m_Planets.at(rings.first));
    }

    centerSceneToObject(player);
    ComponentBody& body = *player->getComponent<ComponentBody>();
    float xPos = body.position().x;
    float zPos = body.position().z;

    /*
    ObjectDisplay* _s = new ObjectDisplay(ResourceManifest::TestMesh,ResourceManifest::MirandaMaterial,glm::vec3(xPos+4,0,zPos-2),glm::vec3(1.0f),"TestObject1",nullptr);
    _s->playAnimation("Skeleton|fire",0.0f,-1.0f,0);
    _s->playAnimation("Skeleton|fire_top",0.0f,-1.0f,0);
    _s->playAnimation("Skeleton|fire_hammer",0.0f,-1.0f,0);
    */

    new Ship(ResourceManifest::StarbaseMesh,ResourceManifest::StarbaseMaterial,false,"Starfleet Command",glm::vec3(xPos+50,0,zPos+50),glm::vec3(1),nullptr,this);

    body.translate(0,0,2);

    //LightProbe* lightP = new LightProbe("MainLightProbe",512,glm::vec3(0),false,this,1);
    //player->addChild(lightP);
}
void SolarSystem::update(const float& dt){
    Scene::update(dt);
}
void SolarSystem::onEvent(const Event& e){
	if(e.type == EventType::SceneChanged && e.eventSceneChanged.newScene == this){
		std::cout << giDiffuse << std::endl;
		std::cout << giSpecular << std::endl;
		std::cout << giGlobal << std::endl;
		Renderer::Settings::Lighting::setGIContributionDiffuse(giDiffuse);
		Renderer::Settings::Lighting::setGIContributionSpecular(giSpecular);
		Renderer::Settings::Lighting::setGIContributionGlobal(giGlobal);
	}
}

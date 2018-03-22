#include "Engine_Events.h"
#include "SolarSystem.h"
#include "Engine_Resources.h"
#include "ResourceManifest.h"
#include "Planet.h"
#include "Lagrange.h"
#include "GameCamera.h"
#include "Ship.h"
#include "GameSkybox.h"
#include "Light.h"

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
    GameCamera* playerCamera = new GameCamera(60,Resources::getWindowSize().x/(float)Resources::getWindowSize().y,0.01f,9000000000.0f,this);
    this->setActiveCamera(playerCamera);

    if(file != "NULL"){
        if(file == ""){
            SolarSystem::_loadRandomly();
        }
        else{
            SolarSystem::_loadFromFile(file);
        }
    }
}
SolarSystem::~SolarSystem(){

}
void SolarSystem::_loadFromFile(string filename){
    uint count = 0;
    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
    unordered_map<string,std::vector<RingInfo>> planetRings;

	unordered_map<string,Handle> loadedMaterials;

    string skybox;
    for(string line; getline(str, line, '\n');){
        line.erase( remove(line.begin(), line.end(), '\r'), line.end() ); //remove \r from the line
        if(line[0] != '#'){//ignore commented lines
            if(count == 1){//this line has the system's name
                setName(line);
            }
            else if(count == 2){//this line has the system's skybox
                skybox = line;
            }
            else if(count == 3){//this line has the system's skybox's number of flares
                GameSkybox* box = new GameSkybox(skybox,boost::lexical_cast<uint>(line),this);
            }
            if((line[0] == 'S' || line[0] == 'M' || line[0] == 'P' || line[0] == '*' || line[0] == 'R' || line[0] == '$' || line[0] == 'L' || line[0] == 's') && line[1] == ' '){//we got something to work with
                Planet* planetoid = nullptr;

                string token;
                istringstream stream(line);

                string NAME;
                string LAGRANGE__TYPE;
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
                    else if(key == "lType")            LAGRANGE__TYPE = value;
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
                    string normalFile = "";
                    string glowFile = "";
                    string extention;

                    //get file extension
                    for(uint i = TEXTURE.length() - 4; i < TEXTURE.length(); ++i) extention += tolower(TEXTURE.at(i));

                    string normFile = TEXTURE.substr(0,TEXTURE.size()-4);
                    normFile += "_Normal" + extention;
                    if(boost::filesystem::exists(normFile)){
                        normalFile = normFile;
                    }
                    string gloFile = TEXTURE.substr(0,TEXTURE.size()-4);
                    gloFile += "_Glow" + extention;
                    if(boost::filesystem::exists(gloFile)){
                        glowFile = gloFile;
                    }
					if(!loadedMaterials.count(MATERIAL_NAME)){
						Handle handle = Resources::addMaterial(MATERIAL_NAME,TEXTURE,normalFile,glowFile,"",ResourceManifest::groundFromSpace);
						loadedMaterials.emplace(MATERIAL_NAME,handle);
					}
                }
                if(line[0] == 'S'){//Sun
                    Star* star = new Star(glm::vec3(R,G,B),glm::vec3(R1,G1,B1),glm::vec3(0),(float)RADIUS,NAME,this);
                    if(PARENT != ""){
                        star->setPosition(objects().at(PARENT)->getPosition()+glm::vec3(xPos,0,zPos));
                    }
                    m_Stars.emplace(NAME,star);
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
                        Object* parent = objects().at(PARENT);
                        planetoid->setPosition(planetoid->getPosition() + parent->getPosition());

                        if(ORBIT_PERIOD != -1.0f){
                            planetoid->setOrbit(new OrbitInfo(ORBIT_ECCENTRICITY,ORBIT_PERIOD,(float)ORBIT_MAJOR_AXIS,randAngle,PARENT));
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
                        Object* parent = objects().at(PARENT);
                        planetoid->setPosition(planetoid->getPosition() + parent->getPosition());

                        if(ORBIT_PERIOD != -1.0f){
                            planetoid->setOrbit(new OrbitInfo(ORBIT_ECCENTRICITY,ORBIT_PERIOD,(float)ORBIT_MAJOR_AXIS,randAngle,PARENT));
                        }
                        if(ROTATIONAL_TILT != -1.0f){
                            planetoid->setRotation(new RotationInfo(ROTATIONAL_TILT,ROTATIONAL_PERIOD));
                        }
                    }
                    m_Moons.emplace(NAME,planetoid);
					
                }
                else if(line[0] == '*'){//Player ship
                    if(PARENT != ""){
                        float parentX = objects().at(PARENT)->getPosition().x;
                        float parentZ = objects().at(PARENT)->getPosition().z;
                        xPos += parentX;
                        zPos += parentZ;
                    }
					setPlayer(new Ship(ResourceManifest::DefiantMesh,ResourceManifest::DefiantMaterial,true,NAME,glm::vec3(xPos,0,zPos),glm::vec3(1),nullptr,this));
					GameCamera* playerCamera = (GameCamera*)getActiveCamera();
					playerCamera->follow(getPlayer());
                }
                else if(line[0] == '$'){//Other ship
                    if(PARENT != ""){
                        float parentX = objects().at(PARENT)->getPosition().x;
                        float parentZ = objects().at(PARENT)->getPosition().z;
                        xPos += parentX;
                        zPos += parentZ;
                    }
                    new Ship(ResourceManifest::AkiraMesh,ResourceManifest::AkiraMaterial,false,NAME,glm::vec3(xPos,0,zPos),glm::vec3(1),nullptr,this);
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
                    m_LagrangePoints.emplace(NAME,new Lagrange(LAGRANGE_PLANET_1,LAGRANGE_PLANET_2,LAGRANGE__TYPE,NAME,this));
                }
            }
        }
        ++count;
    }

    //add planetary rings
    for(auto rings:planetRings){
        new Ring(rings.second,(Planet*)(m_Objects.at(rings.first)));
    }

    centerSceneToObject(player);

    float xPos = Resources::getObject("Valiant")->getPosition().x;
    float zPos = Resources::getObject("Valiant")->getPosition().z;

    ObjectDisplay* _s = new ObjectDisplay(ResourceManifest::TestMesh,ResourceManifest::MirandaMaterial,glm::vec3(xPos+4,0,zPos-2),glm::vec3(1.0f),"TestObject1",nullptr);
    _s->playAnimation("Skeleton|fire",0.0f,-1.0f,0);
    _s->playAnimation("Skeleton|fire_top",0.0f,-1.0f,0);
    _s->playAnimation("Skeleton|fire_hammer",0.0f,-1.0f,0);
    new Ship(ResourceManifest::StarbaseMesh,ResourceManifest::StarbaseMaterial,false,"Starfleet Command",glm::vec3(xPos+50,0,zPos+50),glm::vec3(1),nullptr,this);
	new ObjectDisplay(ResourceManifest::PlanetMesh,ResourceManifest::GoldMaterial,glm::vec3(0,5,2),glm::vec3(2));
    player->translate(0,0,2);

	new Ship(ResourceManifest::StarbaseMesh,ResourceManifest::StarbaseMaterial,false,"Luna Starbase",m_Moons.at("Moon")->getPosition() + glm::vec3(16000,0,16000),glm::vec3(1),nullptr,this);

	//entity testing
	Entity* test = new Entity();
	addEntity(test);
	ComponentBasicBody* basicBody = new ComponentBasicBody();
	basicBody->setPosition(xPos+2,0,zPos-3);
	ComponentModel* model = new ComponentModel(ResourceManifest::DefiantMesh,ResourceManifest::DefiantMaterial,test);
	test->addComponent(basicBody);
	test->addComponent(model);	
	////////////////////////////////

	//LightProbe* lightP = new LightProbe("MainLightProbe",512,glm::vec3(0),false,this,1);
	//player->addChild(lightP);
}
void SolarSystem::_loadRandomly(){
    #pragma region Skybox
    //get random skybox folder from the skybox directory
    vector<std::string> folders;
	unordered_map<string,Handle> loadedMaterials;
    string path = "data/Textures/Skyboxes/";
    if ( boost::filesystem::exists( path ) ) {
        boost::filesystem::directory_iterator end_itr;
        for ( boost::filesystem::directory_iterator itr( path );itr != end_itr;++itr ){
            if ( boost::filesystem::is_directory(itr->status()) ){
                string path_name = to_string(itr->path());
                replace(path_name.begin(),path_name.end(),'\\','/');
                boost::erase_all(path_name,"\"");
                path_name = path_name.substr(path.size(),path_name.size());
                folders.push_back(path_name);
            }
        }
    }
    uint random_skybox_index = uint(rand() % folders.size());
    string skybox = folders.at(random_skybox_index);
    uint numFlares = rand() % 200;
    new GameSkybox(path + skybox,numFlares,this);
    #pragma endregion

    #pragma region ConstructStars
    uint percent = uint(rand() % 1000);

    uint numberOfStars = 1;
    if(percent < 50)                         numberOfStars = 7;
    else if(percent >= 50 && percent < 135)  numberOfStars = 6;
    else if(percent >= 135 && percent < 250) numberOfStars = 5;
    else if(percent >= 250 && percent < 450) numberOfStars = 4;
    else if(percent >= 450 && percent < 750) numberOfStars = 3;
    else if(percent >= 750 && percent < 850) numberOfStars = 2;


    for(uint i = 0; i < numberOfStars; ++i){
        Star* star = nullptr;
        //star sizes: most big: 1,800 * the sun's size, smallest: 14% the size of the sun
        float radius = float(97412.0f + (rand() % 1252440000))*10.0f;
        float position = float(radius * 2.0f + (rand() % 841252440000));

        float R = float((rand()%100)/100.0f);
        float G = float((rand()%100)/100.0f);
        float B = float((rand()%100)/100.0f);

        glm::vec3 starColor = glm::vec3(R,G,B);
        glm::vec3 lightColor = glm::vec3(glm::min(1.0f,R+0.1f),glm::min(1.0f,G+0.1f),glm::min(1.0f,B+0.1f));
        float posX, posZ;

        float randomDegree = (rand() % 36000)/100.0f;
        posX = glm::sin(randomDegree) * position;
        posZ = glm::cos(randomDegree) * position;

        star = new Star(starColor,lightColor,glm::vec3(posX,0,posZ),radius,"Star " + to_string(1 + i),this);
        m_Stars["Star " + to_string(1 + i)] = star;
    }
    #pragma endregion

    #pragma region CheckStarPositions
    //check if any stars are too close to each other, and move them accordingly
    bool allStarsGood = true;
    glm::vec3 centerOfMassPosition;

    vector<float> starMasses;
    vector<glm::vec3> starPositions;
    float totalMasses = 0.0;
    float biggestRadius = 0.0;
    for(auto star:m_Stars){
        for(auto otherStar:m_Stars){
            if(star != otherStar){
                float biggerRadius = glm::max(star.second->getRadius(),otherStar.second->getRadius());
                unsigned long long dist = star.second->getDistanceLL(otherStar.second);
                if(dist < biggerRadius * 10.0f){
                    allStarsGood = false;
                }
            }
        }
        biggestRadius = glm::max(float(star.second->getRadius()),biggestRadius);
        starPositions.push_back(star.second->getPosition());
        starMasses.push_back(star.second->getRadius());
        totalMasses += star.second->getRadius();
    }

    float numerator1 = 0, numerator2 = 0;
    for(uint i = 0; i < starMasses.size(); ++i){
        numerator1 += (starMasses.at(i) * starPositions.at(i).x);
        numerator2 += (starMasses.at(i) * starPositions.at(i).z);
    }
    centerOfMassPosition.x = (numerator1) / (totalMasses);
    centerOfMassPosition.z = (numerator2) / (totalMasses);

    while(!allStarsGood){
        for(auto star:m_Stars){
            glm::vec3 starPos = star.second->getPosition();
            glm::vec3 offset = starPos - centerOfMassPosition;
            glm::vec3 normOffset = glm::normalize(offset);
            star.second->setPosition(star.second->getPosition() + (normOffset*biggestRadius*float(2.0)));
        }
        allStarsGood = true;
        for(auto star:m_Stars){
            for(auto otherStar:m_Stars){
                if(star != otherStar){
                    float biggerRadius = glm::max(star.second->getRadius(),otherStar.second->getRadius());
                    unsigned long long dist = star.second->getDistanceLL(otherStar.second);
                    if(dist < biggerRadius * 10.0f){
                        allStarsGood = false;
                    }
                }
            }
        }
    }
    #pragma endregion

    //Then load planets. Generally the more stars, the more planets

    //First get the database of random textures to choose from
    unordered_map<string,string> planets_folders;
    unordered_map<string,vector<string>> planet_textures;
    string planets_path = "data/Textures/Planets/Random/Planet";
    if ( boost::filesystem::exists( planets_path ) ) {
        boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
        for ( boost::filesystem::directory_iterator itr( planets_path );itr != end_itr;++itr ){
            if ( boost::filesystem::is_directory(itr->status()) ){
                string path_name = to_string(itr->path());
                string folder_name = "";
                replace(path_name.begin(),path_name.end(),'\\','/');
                boost::erase_all(path_name,"\"");
                string short_name = path_name.substr(planets_path.size()+1,path_name.size()-1);
                planets_folders[short_name] = path_name;

            }
        }
    }
    
    for(auto folder: planets_folders){
        if ( boost::filesystem::exists( folder.second ) ) {
            string key = folder.second.substr(planets_path.size() + 1,folder.second.size()-1);
            boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
            vector<string> value;
            for ( boost::filesystem::directory_iterator itr( folder.second );itr != end_itr;++itr ){
                if ( boost::filesystem::is_directory(itr->status()) ){
                    string path_name = to_string(itr->path());
                    string folder_name = "";
                    replace(path_name.begin(),path_name.end(),'\\','/');
                    boost::erase_all(path_name,"\"");
                    value.push_back(path_name);
                }
            }
            planet_textures[key] = value;
        }
    }

    for(auto star:m_Stars){
        uint numberOfPlanets = uint(1 + rand() % 100);
        for(uint i = 0; i < numberOfPlanets; ++i){
            Planet* planet = nullptr;

            float maxPositionAwayFromSun = glm::abs(glm::length(star.second->getPosition() - centerOfMassPosition));
            maxPositionAwayFromSun -= (maxPositionAwayFromSun / 4.0f);
            if(m_Stars.size() == 1)
                maxPositionAwayFromSun = glm::abs(glm::length(star.second->getRadius()*2000.0f));
            float minPositionAwayFromSun = star.second->getRadius() * 4.0f;
            float positionAwayFromSun = glm::max(minPositionAwayFromSun,rand() *maxPositionAwayFromSun);

            float posX,posZ;
            float randomDegree = (rand() % 36000) / 100.0f;
            posX = sin(randomDegree) * positionAwayFromSun;
            posZ = cos(randomDegree) * positionAwayFromSun;

            float RADIUS = (500.0f + (rand() % 85000))*10.0f;
            PlanetType PLANET_TYPE = PLANET_TYPE_ROCKY;
            if(RADIUS <= 15000.0f){
                float chance = rand() % 1000 / 1000.0f;
                if(chance > 0.85f)
                    PLANET_TYPE = PLANET_TYPE_ICE;
            }
            else{
                float chance = (rand() % 1000) / 1000.0f;
                if(chance > 0.6f)
                    PLANET_TYPE = PLANET_TYPE_ICE_GIANT;
                else
                    PLANET_TYPE = PLANET_TYPE_GAS_GIANT;
            }

            float ATMOSPHERE_HEIGHT = 0.0f;

            //dist of earth from sun 149,600,000. radius of sun 695,800
            //planets with atmosphere must be roughly 215 sun radius's away (Earth like) to min 155 away (Venus) and max 327 (Mars)
            if(positionAwayFromSun > 155.0 * star.second->getRadius() && positionAwayFromSun < 327.0 * star.second->getRadius()){
                float chance = (rand() % 1000) / 1000.0f;
                if(chance > 0.5f)
                    ATMOSPHERE_HEIGHT = 0.025f;
            }

            //now to get a random planet material based on a random texture based on planet type
            string MATERIAL_NAME,FOLDER,normalFile,glowFile = "";
            string base_name;
            string base_path = planets_path;
            string key = "";
            if(PLANET_TYPE == PLANET_TYPE_ROCKY){          key = "Rocky"; }
            else if(PLANET_TYPE == PLANET_TYPE_ICE){       key = "Ice"; }
            else if(PLANET_TYPE == PLANET_TYPE_GAS_GIANT){ key = "GasGiant"; }
            else if(PLANET_TYPE == PLANET_TYPE_ICE_GIANT){ key = "IceGiant"; }
            FOLDER = planets_folders[key];
            base_path += "/" + key + "/";
            vector<string> textures = planet_textures[key];
            string texture = textures.at(rand() % textures.size());
            base_name = texture.substr(base_path.size(),texture.size()-1);
            FOLDER += "/" + base_name;
            MATERIAL_NAME = FOLDER;
            FOLDER += "/";

            string diffuseFile = FOLDER + base_name + ".jpg";
            string normFile = FOLDER + base_name + "_Normal.jpg";
            string gloFile = FOLDER + base_name + "_Glow.jpg";
            if(boost::filesystem::exists(normFile)){ normalFile = normFile; }
            if(boost::filesystem::exists(gloFile)){ glowFile = gloFile; }

            if(boost::filesystem::exists(diffuseFile)){
                Handle h = Resources::addMaterial(MATERIAL_NAME,diffuseFile,normalFile,glowFile);
				loadedMaterials.emplace(MATERIAL_NAME,h);
                string pName = "Planet " + to_string(i + 1);
                planet = new Planet(loadedMaterials.at(MATERIAL_NAME),PLANET_TYPE,glm::vec3(posX,0,posZ),RADIUS,pName,ATMOSPHERE_HEIGHT,this);

                float R = (rand() % 1000)/1000.0f;
                float G = (rand() % 1000)/1000.0f;
                float B = (rand() % 1000)/1000.0f;
                //planet->setColor(R,G,B,1);
                m_Planets[pName] = planet;

                //rings
                vector<RingInfo> rings;
                uint numRings = rand() % 20;
                uint chance = rand() % 100;
                if(chance <= 20){
                    for(uint i = 0; i < numRings; ++i){
                        uint randSize = (rand() % 200 + 3);
                        uint randPos = (randSize + 1 ) + (rand() % (1024 - ((randSize + 1)*2)));

                        uint RR = rand() % 255;
                        uint RG = rand() % 255;
                        uint RB = rand() % 255;

                        uint randBreak = (rand() % randSize);
                        uint chance1 = rand() % 100;
                        uint chance2 = rand() % 100;

                        if(chance1 > 25){
                            rings.push_back(RingInfo(randPos,randSize,glm::uvec3(RR,RG,RB),randBreak));
                        }
                        else if(randSize < 50){
                            if(chance2 < 20)
                                rings.push_back(RingInfo(randPos,randSize,glm::uvec3(-1),randBreak));
                        }
                    }
                    if(rings.size() > 0)
                        new Ring(rings,planet);
                }
            }
        }
    }
    //Then load moons. Generally the number of moons depends on the type of planet. Gas Giants have more moons than normal planets, etc..

	player = new Ship(ResourceManifest::DefiantMesh,ResourceManifest::DefiantMaterial,true,"USS Defiant",glm::vec3(0),glm::vec3(1),nullptr,this);
	GameCamera* playerCamera = (GameCamera*)getActiveCamera();
	playerCamera->follow(getPlayer());
    centerSceneToObject(player);
}
void SolarSystem::update(float dt){
    Scene::update(dt);
}

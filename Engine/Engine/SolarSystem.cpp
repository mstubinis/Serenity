#include "Engine_Events.h"
#include "SolarSystem.h"
#include "Engine_Resources.h"
#include "Planet.h"
#include "Lagrange.h"
#include "GameCamera.h"
#include "Ship.h"
#include "GameSkybox.h"
#include "Light.h"
#include "Particles.h"
#include "Station.h"
#include "Terrain.h"
#include "Atmosphere.h"

#include <algorithm>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace Engine;
using namespace Engine::Events;

SolarSystem::SolarSystem(std::string name, std::string file,bool test):Scene(name){
    playerCamera = new GameCamera("PlayerCamera_" + m_Name,45,Resources::getWindowSize().x/(float)Resources::getWindowSize().y,0.1f,9000000000.0f,this);
    Resources::setActiveCamera(playerCamera);

    if(!test){
        if(file != "NULL"){
            if(file == ""){
                SolarSystem::_loadRandomly();
            }
            else{
                SolarSystem::_loadFromFile(file);
            }
        }
    }
    else{
        SolarSystem::_loadTest();
    }
}
SolarSystem::~SolarSystem(){

}
void SolarSystem::_loadTest(){
    new GameSkybox("data/Textures/Skyboxes/SolarSystem",0,this);

    Star* star = new Star(glm::vec3(1,1,0),glm::vec3(1,1,1),glm::v3(0),static_cast<float>(1000000),"Sun",this);
    star->setPosition(glm::v3(0,3000000,-1000000));
    m_Stars["Sun"] = star;

    player = new Ship("Dreadnaught","Dreadnaught",true,"Dreadnaught",glm::v3(0,20,0),glm::vec3(1),nullptr,this);
    playerCamera = static_cast<GameCamera*>(Resources::getActiveCamera());
    playerCamera->follow(player);


    sf::Image heightmap;
    heightmap.loadFromFile("data/Textures/test.png");
    Terrain* t = new Terrain("Heightfield",heightmap,"Defiant",this);
    t->setScale(3,16,3);
}
void SolarSystem::_loadFromFile(std::string filename){
    unsigned int count = 0;
    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
    std::unordered_map<std::string,std::vector<RingInfo>> planetRings;

    std::string skybox;
    for(std::string line; std::getline(str, line, '\n');){
        line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() ); //remove \r from the line
        if(line[0] != '#'){//ignore commented lines
            if(count == 1){//this line has the system's name
                this->setName(line);
            }
            else if(count == 2){//this line has the system's skybox
                skybox = line;
            }
            else if(count == 3){//this line has the system's skybox's number of flares
                new GameSkybox(skybox,boost::lexical_cast<unsigned int>(line),this);
				//new Atmosphere();
            }
            if((line[0] == 'S' || line[0] == 'M' || line[0] == 'P' || line[0] == '*' || line[0] == 'R' || line[0] == '$' || line[0] == 'L' || line[0] == 's') && line[1] == ' '){//we got something to work with
                Planet* planetoid = nullptr;

                std::string token;
                std::istringstream stream(line);

                std::string NAME;
                std::string LAGRANGE__TYPE;
                std::string LAGRANGE_PLANET_1, LAGRANGE_PLANET_2;
                std::string PARENT = "";
                float R,G,B,   R1,G1,B1;
                float ATMOSPHERE_HEIGHT;
                std::string LIGHTCOLOR;
                std::string TYPE;
                std::string TEXTURE = "data/Textures/Planets/";
                std::string MATERIAL_NAME = "";

                float ORBIT_PERIOD = -1;
                unsigned long long ORBIT_MAJOR_AXIS = -1;
                float ORBIT_ECCENTRICITY = -1;

                float ROTATIONAL_TILT = -1;
                float ROTATIONAL_PERIOD = -1;

                unsigned long long RADIUS = 0;
                unsigned long long POSITION = 0;
                unsigned int BREAK = 0;

                while(std::getline(stream, token, ' ')) {
                    size_t pos = token.find("=");

                    std::string key = token.substr(0, pos);
                    std::string value = token.substr(pos + 1, std::string::npos);

                    if(key == "name"){                  
                        NAME = value;
                        std::replace(NAME.begin(),NAME.end(),'_',' ');
                    }
                    else if(key == "lp1")              LAGRANGE_PLANET_1 = value;
                    else if(key == "lp2")              LAGRANGE_PLANET_2 = value;
                    else if(key == "lType")            LAGRANGE__TYPE = value;
                    else if(key == "radius")           RADIUS = stoull(value)*10;
                    else if(key == "r")			       R = stof(value);
                    else if(key == "g")			       G = stof(value);
                    else if(key == "b")			       B = stof(value);
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
                glm::num randAngle = rand() % 3600;
                randAngle /= 10;
                randAngle *= 3.14159 / 180.0;
                glm::num xPos = glm::cos(randAngle) * static_cast<glm::num>(POSITION);
                glm::num zPos = glm::sin(randAngle) * static_cast<glm::num>(POSITION);

                if(MATERIAL_NAME != ""){
                    std::string normalFile = "";
                    std::string glowFile = "";
                    std::string extention;

                    //get file extension
                    for(unsigned int i = TEXTURE.length() - 4; i < TEXTURE.length(); i++) extention += tolower(TEXTURE.at(i));

                    std::string normFile = TEXTURE.substr(0,TEXTURE.size()-4);
                    normFile += "_Normal" + extention;
                    if(boost::filesystem::exists(normFile)){
                        normalFile = normFile;
                    }
                    std::string gloFile = TEXTURE.substr(0,TEXTURE.size()-4);
                    gloFile += "_Glow" + extention;
                    if(boost::filesystem::exists(gloFile)){
                        glowFile = gloFile;
                    }
                    Resources::addMaterial(MATERIAL_NAME,TEXTURE,normalFile,glowFile);
                }

                if(line[0] == 'S'){//Sun
                    Star* star = new Star(glm::vec3(R,G,B),glm::vec3(R1,G1,B1),glm::v3(0),static_cast<float>(RADIUS),NAME,this);
                    if(PARENT != ""){
                        star->setPosition(getObjects()[PARENT]->getPosition()+glm::v3(xPos,0,zPos));
                    }
                    m_Stars[NAME] = star;
                }
                else if(line[0] == 'P'){//Planet
                    PlanetType PLANET_TYPE;
                    if(TYPE == "Rock") PLANET_TYPE = PLANET_TYPE_ROCKY;
                    else if(TYPE == "Ice") PLANET_TYPE = PLANET_TYPE_ICE;
                    else if(TYPE == "GasGiant") PLANET_TYPE = PLANET_TYPE_GAS_GIANT;
                    else if(TYPE == "IceGiant") PLANET_TYPE = PLANET_TYPE_ICE_GIANT;
                    else if(TYPE == "Asteroid") PLANET_TYPE = PLANET_TYPE_ASTEROID;
                    planetoid = new Planet(MATERIAL_NAME,PLANET_TYPE,glm::v3(xPos,0,zPos),static_cast<float>(RADIUS),NAME,ATMOSPHERE_HEIGHT,this);
                    if(PARENT != ""){
                        Object* parent = getObjects()[PARENT];
                        planetoid->setPosition(planetoid->getPosition() + parent->getPosition());

                        if(ORBIT_PERIOD != -1.0f){
                            planetoid->setOrbit(new OrbitInfo(ORBIT_ECCENTRICITY,ORBIT_PERIOD,static_cast<float>(ORBIT_MAJOR_AXIS),randAngle,PARENT));
                        }
                        if(ROTATIONAL_TILT != -1.0f){
                            planetoid->setRotation(new RotationInfo(ROTATIONAL_TILT,ROTATIONAL_PERIOD));
                        }
                    }
                    m_Planets[NAME] = planetoid;
                }
                else if(line[0] == 'M'){//Moon
                    PlanetType PLANET_TYPE;
                    if(TYPE == "Rock") PLANET_TYPE = PLANET_TYPE_ROCKY;
                    else if(TYPE == "Ice") PLANET_TYPE = PLANET_TYPE_ICE;
                    else if(TYPE == "GasGiant") PLANET_TYPE = PLANET_TYPE_GAS_GIANT;
                    else if(TYPE == "IceGiant") PLANET_TYPE = PLANET_TYPE_ICE_GIANT;
                    else if(TYPE == "Asteroid") PLANET_TYPE = PLANET_TYPE_ASTEROID;
                    planetoid = new Planet(MATERIAL_NAME,PLANET_TYPE,glm::v3(xPos,0,zPos),static_cast<float>(RADIUS),NAME,ATMOSPHERE_HEIGHT,this);
                    if(PARENT != ""){
                        Object* parent = getObjects()[PARENT];
                        planetoid->setPosition(planetoid->getPosition() + parent->getPosition());

                        if(ORBIT_PERIOD != -1.0f){
                            planetoid->setOrbit(new OrbitInfo(ORBIT_ECCENTRICITY,ORBIT_PERIOD,static_cast<float>(ORBIT_MAJOR_AXIS),randAngle,PARENT));
                        }
                        if(ROTATIONAL_TILT != -1.0f){
                            planetoid->setRotation(new RotationInfo(ROTATIONAL_TILT,ROTATIONAL_PERIOD));
                        }
                    }
                    m_Moons[NAME] = planetoid;
                }
                else if(line[0] == '*'){//Player ship
                    if(PARENT != ""){
                        glm::num parentX = getObjects()[PARENT]->getPosition().x;
                        glm::num parentZ = getObjects()[PARENT]->getPosition().z;
                        xPos += parentX;
                        zPos += parentZ;
                    }
                    setPlayer(new Ship("Defiant","Defiant",true,NAME,glm::v3(xPos,0,zPos),glm::vec3(1),nullptr,this));
                    setPlayerCamera(static_cast<GameCamera*>(Resources::getActiveCamera()));
                    getPlayerCamera()->follow(getPlayer());

                }
                else if(line[0] == '$'){//Other ship
                    if(PARENT != ""){
                        glm::num parentX = getObjects()[PARENT]->getPosition().x;
                        glm::num parentZ = getObjects()[PARENT]->getPosition().z;
                        xPos += parentX;
                        zPos += parentZ;
                    }
                    new Ship("Akira","Akira",false,NAME,glm::v3(xPos,0,zPos),glm::vec3(1),nullptr,this);
                }
                else if(line[0] == 'R'){//Rings
                    if(PARENT != ""){
                        if(!planetRings.count(PARENT)){
                            std::vector<RingInfo> rings;
                            planetRings[PARENT] = rings;
                        }
                        planetRings[PARENT].push_back(RingInfo(static_cast<unsigned int>(POSITION/10),static_cast<unsigned int>(RADIUS/10),glm::uvec3(R,G,B),BREAK));
                    }
                }
                else if(line[0] == 'L'){//Lagrange Point
                    m_LagrangePoints[NAME] = new Lagrange(LAGRANGE_PLANET_1,LAGRANGE_PLANET_2,LAGRANGE__TYPE,NAME,this);
                }
                else if(line[0] == 's'){//Station
                    m_Stations[NAME] = new Station("","",glm::v3(xPos,0,zPos),glm::vec3(1),NAME,nullptr,this);
                }
            }
        }
        count++;
    }

    //add planetary rings
    for(auto rings:planetRings){
        new Ring(rings.second,static_cast<Planet*>(m_Objects[rings.first]));
    }
    centerSceneToObject(player);

	glm::num xPos = Resources::getObject("Valiant")->getPosition().x;
	glm::num zPos = Resources::getObject("Valiant")->getPosition().z;

	new Ship("Defiant","Defiant",false,"Defiant 1",glm::v3(xPos+3,0,zPos-3),glm::vec3(1),nullptr,this);
	new Ship("Intrepid","Intrepid",false,"Intrepid 2",glm::v3(xPos-3,0,zPos+3),glm::vec3(1),nullptr,this);
	new Ship("Defiant","Defiant",false,"Defiant 3",glm::v3(xPos+2,0+2,zPos+2),glm::vec3(1),nullptr,this);
	new Ship("Intrepid","Intrepid",false,"Intrepid 4",glm::v3(xPos-2,0-2,zPos-2),glm::vec3(1),nullptr,this);
	new Ship("Norway","Norway",false,"Norway 5",glm::v3(xPos+4,0+4,zPos+4),glm::vec3(1),nullptr,this);
	new Ship("Norway","Norway",false,"Norway 6",glm::v3(xPos+4,0-4,zPos+4),glm::vec3(1),nullptr,this);

	new Ship("Starbase","Starbase",false,"Starfleet Command",glm::v3(xPos+50,0,zPos+50),glm::vec3(1),nullptr,this);

	player->translate(0,0,11);
}
void SolarSystem::_loadRandomly(){
    #pragma region Skybox
    //get random skybox folder from the skybox directory
    std::vector<std::string> folders;
    std::string path = "data/Textures/Skyboxes/";
    if ( boost::filesystem::exists( path ) ) {
        boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
        for ( boost::filesystem::directory_iterator itr( path );itr != end_itr;++itr ){
            if ( boost::filesystem::is_directory(itr->status()) ){
                std::string path_name = boost::lexical_cast<std::string>(itr->path());
                std::replace(path_name.begin(),path_name.end(),'\\','/');
                boost::erase_all(path_name,"\"");
                path_name = path_name.substr(path.size(),path_name.size());
                folders.push_back(path_name);
            }
        }
    }
    unsigned int random_skybox_index = static_cast<unsigned int>((rand() % folders.size()));
    std::string skybox = folders.at(random_skybox_index);
    unsigned int numFlares = rand() % 200;
    new GameSkybox(path + skybox,numFlares,this);
    #pragma endregion

    #pragma region ConstructStars
    unsigned int percent = static_cast<unsigned int>(rand() % 1000);

    unsigned int numberOfStars = 1;
    if(percent < 50) 
        numberOfStars = 7;
    else if(percent >= 50 && percent < 135)
        numberOfStars = 6;
    else if(percent >= 135 && percent < 250)
        numberOfStars = 5;
    else if(percent >= 250 && percent < 450)
        numberOfStars = 4;
    else if(percent >= 450 && percent < 750)
        numberOfStars = 3;
    else if(percent >= 750 && percent < 850)
        numberOfStars = 2;


    for(unsigned int i = 0; i < numberOfStars; i++){
        Star* star = nullptr;
        //star sizes: most big: 1,800 * the sun's size, smallest: 14% the size of the sun
        glm::num radius = static_cast<glm::num>(97412.0 + (rand() % 1252440000))*10.0;
        glm::num position = static_cast<glm::num>(radius * 2.0 + (rand() % 841252440000));

        float R = static_cast<float>((rand()%100)/100.0f);
        float G = static_cast<float>((rand()%100)/100.0f);
        float B = static_cast<float>((rand()%100)/100.0f);

        glm::vec3 starColor = glm::vec3(R,G,B);
        glm::vec3 lightColor = glm::vec3(glm::min(1.0f,R+0.1f),glm::min(1.0f,G+0.1f),glm::min(1.0f,B+0.1f));
        glm::num posX,posZ;

        float randomDegree = (rand() % 36000)/100.0f;
        posX = glm::sin(randomDegree) * position;
        posZ = glm::cos(randomDegree) * position;

        star = new Star(starColor,lightColor,glm::v3(posX,0,posZ),radius,"Star " + boost::lexical_cast<std::string>(1 + i),this);
        m_Stars["Star " + boost::lexical_cast<std::string>(1 + i)] = star;
    }
    #pragma endregion

    #pragma region CheckStarPositions
    //check if any stars are too close to each other, and move them accordingly
    bool allStarsGood = true;
    glm::v3 centerOfMassPosition;

    std::vector<glm::num> starMasses;
    std::vector<glm::v3> starPositions;
    glm::num totalMasses = 0.0;
    glm::num biggestRadius = 0.0;
    for(auto star:m_Stars){
        for(auto otherStar:m_Stars){
            if(star != otherStar){
                glm::num biggerRadius = glm::max(star.second->getRadius(),otherStar.second->getRadius());
                unsigned long long dist = star.second->getDistanceLL(otherStar.second);
                if(dist < biggerRadius * 10.0f){
                    allStarsGood = false;
                }
            }
        }
        biggestRadius = glm::max(static_cast<glm::num>(star.second->getRadius()),biggestRadius);
        starPositions.push_back(star.second->getPosition());
        starMasses.push_back(star.second->getRadius());
        totalMasses += star.second->getRadius();
    }

    glm::num numerator1 = 0,numerator2 = 0;
    for(unsigned int i = 0; i < starMasses.size(); i++){
        numerator1 += (starMasses.at(i) * starPositions.at(i).x);
        numerator2 += (starMasses.at(i) * starPositions.at(i).z);
    }
    centerOfMassPosition.x = (numerator1) / (totalMasses);
    centerOfMassPosition.z = (numerator2) / (totalMasses);

    while(!allStarsGood){
        for(auto star:m_Stars){
            glm::v3 starPos = star.second->getPosition();
            glm::v3 offset = starPos - centerOfMassPosition;
            glm::v3 normOffset = glm::normalize(offset);
            star.second->setPosition(star.second->getPosition() + (normOffset*biggestRadius*static_cast<glm::num>(2.0)));
        }
        allStarsGood = true;
        for(auto star:m_Stars){
            for(auto otherStar:m_Stars){
                if(star != otherStar){
                    glm::num biggerRadius = glm::max(star.second->getRadius(),otherStar.second->getRadius());
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
    std::unordered_map<std::string,std::string> planets_folders;
    std::unordered_map<std::string,std::vector<std::string>> planet_textures;
    std::string planets_path = "data/Textures/Planets/Random/Planet";
    if ( boost::filesystem::exists( planets_path ) ) {
        boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
        for ( boost::filesystem::directory_iterator itr( planets_path );itr != end_itr;++itr ){
            if ( boost::filesystem::is_directory(itr->status()) ){
                std::string path_name = boost::lexical_cast<std::string>(itr->path());
                std::string folder_name = "";
                std::replace(path_name.begin(),path_name.end(),'\\','/');
                boost::erase_all(path_name,"\"");
                std::string short_name = path_name.substr(planets_path.size()+1,path_name.size()-1);
                planets_folders[short_name] = path_name;

            }
        }
    }
    
    for(auto folder: planets_folders){
        if ( boost::filesystem::exists( folder.second ) ) {
            std::string key = folder.second.substr(planets_path.size() + 1,folder.second.size()-1);
            boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
            std::vector<std::string> value;
            for ( boost::filesystem::directory_iterator itr( folder.second );itr != end_itr;++itr ){
                if ( boost::filesystem::is_directory(itr->status()) ){
                    std::string path_name = boost::lexical_cast<std::string>(itr->path());
                    std::string folder_name = "";
                    std::replace(path_name.begin(),path_name.end(),'\\','/');
                    boost::erase_all(path_name,"\"");
                    value.push_back(path_name);
                }
            }
            planet_textures[key] = value;
        }
    }

    for(auto star:m_Stars){
        unsigned int numberOfPlanets = static_cast<unsigned int>(1 + (rand() % 100));
        for(unsigned int i = 0; i < numberOfPlanets; i++){
            Planet* planet = nullptr;

            glm::num maxPositionAwayFromSun = glm::abs(glm::length(star.second->getPosition() - centerOfMassPosition));
            maxPositionAwayFromSun -= (maxPositionAwayFromSun / 4.0f);
            if(m_Stars.size() == 1)
                maxPositionAwayFromSun = glm::abs(glm::length(star.second->getRadius()*2000.0f));
            glm::num minPositionAwayFromSun = star.second->getRadius() * 4.0f;
            glm::num positionAwayFromSun = glm::max(minPositionAwayFromSun,rand() *maxPositionAwayFromSun);

            glm::num posX,posZ;
            glm::num randomDegree = rand() % 36000 / 100.0f;
            posX = sin(randomDegree) * positionAwayFromSun;
            posZ = cos(randomDegree) * positionAwayFromSun;

            glm::num RADIUS = (500.0 + rand() % 85000)*10.0;
            PlanetType PLANET_TYPE = PLANET_TYPE_ROCKY;
            if(RADIUS <= 15000){
                float chance = rand() % 1000 / 1000.0f;
                if(chance > 0.85f)
                    PLANET_TYPE = PLANET_TYPE_ICE;
            }
            else{
                float chance = rand() % 1000 / 1000.0f;
                if(chance > 0.6f)
                    PLANET_TYPE = PLANET_TYPE_ICE_GIANT;
                else
                    PLANET_TYPE = PLANET_TYPE_GAS_GIANT;
            }

            float ATMOSPHERE_HEIGHT = 0.0f;

            //dist of earth from sun 149,600,000. radius of sun 695,800
            //planets with atmosphere must be roughly 215 sun radius's away (Earth like) to min 155 away (Venus) and max 327 (Mars)
            if(positionAwayFromSun > 155.0 * star.second->getRadius() && positionAwayFromSun < 327.0 * star.second->getRadius()){
                float chance = rand() % 1000 / 1000.0f;
                if(chance > 0.5f)
                    ATMOSPHERE_HEIGHT = 0.025f;
            }

            //now to get a random planet material based on a random texture based on planet type
            std::string MATERIAL_NAME,FOLDER,normalFile,glowFile = "";
            std::string base_name;
            std::string base_path = planets_path;
            std::string key = "";
            if(PLANET_TYPE == PLANET_TYPE_ROCKY){
                key = "Rocky";
            }
            else if(PLANET_TYPE == PLANET_TYPE_ICE){
                key = "Ice";
            }
            else if(PLANET_TYPE == PLANET_TYPE_GAS_GIANT){
                key = "GasGiant";
            }
            else if(PLANET_TYPE == PLANET_TYPE_ICE_GIANT){
                key = "IceGiant";
            }
            FOLDER = planets_folders[key];
            base_path += "/" + key + "/";
            std::vector<std::string> textures = planet_textures[key];
            std::string texture = textures.at(rand() % textures.size());
            base_name = texture.substr(base_path.size(),texture.size()-1);
            FOLDER += "/" + base_name;
            MATERIAL_NAME = FOLDER;
            FOLDER += "/";

            std::string diffuseFile = FOLDER + base_name + ".jpg";
            std::string normFile = FOLDER + base_name + "_Normal.jpg";
            std::string gloFile = FOLDER + base_name + "_Glow.jpg";
            if(boost::filesystem::exists(normFile)){ normalFile = normFile; }
            if(boost::filesystem::exists(gloFile)){ glowFile = gloFile; }

            if(boost::filesystem::exists(diffuseFile)){
                Resources::addMaterial(MATERIAL_NAME,diffuseFile,normalFile,glowFile);
                std::string pName = "Planet " + boost::lexical_cast<std::string>(i + 1);
                planet = new Planet(MATERIAL_NAME,PLANET_TYPE,glm::v3(posX,0,posZ),RADIUS,pName,ATMOSPHERE_HEIGHT,this);

                float R = (rand() % 1000)/1000.0f;
                float G = (rand() % 1000)/1000.0f;
                float B = (rand() % 1000)/1000.0f;
                planet->setColor(R,G,B,1);
                m_Planets[pName] = planet;

                //rings
                std::vector<RingInfo> rings;
                unsigned int numRings = rand() % 20;
                unsigned int chance = rand() % 100;
                if(chance <= 20){
                    for(unsigned int i = 0; i < numRings; i++){
                        
                        unsigned int randSize = ((rand() % 200 + 3));

                        unsigned int randPos = (randSize + 1 ) + (rand() % (1024 - ((randSize + 1)*2)));

                        unsigned int RR = rand() % 255;
                        unsigned int RG = rand() % 255;
                        unsigned int RB = rand() % 255;

                        unsigned int randBreak = (rand() % randSize);
                        unsigned int chance1 = rand() % 100;
                        unsigned int chance2 = rand() % 100;

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
    //Then load moons. Generally the number of moons depends on the type of planet. Giants have more moons than normal planets, etc..

    player = new Ship("Akira","Akira",true,"USS Thunderchild",glm::v3(0),glm::vec3(1),nullptr,this);
    playerCamera = static_cast<GameCamera*>(Resources::getActiveCamera());
    playerCamera->follow(player);
    centerSceneToObject(player);
}
void SolarSystem::update(float dt){
    Scene::update(dt);
}

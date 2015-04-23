#include "Engine_Events.h"
#include "SolarSystem.h"
#include "Engine_Resources.h"
#include "Planet.h"
#include "GameCamera.h"
#include "Ship.h"
#include "Skybox.h"
#include "Light.h"

#include <algorithm>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace Engine;
using namespace Engine::Events;

SolarSystem::SolarSystem(std::string name, std::string file):Scene(name){
	playerCamera = new GameCamera(45,Resources::getWindow()->getSize().x/(float)Resources::getWindow()->getSize().y,0.1f,9000000000.0f,this);
	Resources::setActiveCamera(playerCamera);
	Engine::Resources::Detail::ResourceManagement::m_Cameras["Debug"] = new Camera(45,Resources::getWindow()->getSize().x/(float)Resources::getWindow()->getSize().y,0.1f,9000000000.0f,this);
	Engine::Resources::Detail::ResourceManagement::m_Cameras["HUD"] = new Camera(0,(float)Resources::getWindow()->getSize().x,0,(float)Resources::getWindow()->getSize().y,0.05f,10.0f,this);

	if(file == ""){
		SolarSystem::_loadRandomly();
		//SolarSystem::_loadTestSystem();
	}
	else{
		SolarSystem::_loadFromFile(file);
	}
}
SolarSystem::~SolarSystem(){
	for(auto star:m_Stars)    delete star;
	for(auto planet:m_Planets)delete planet;
	for(auto moon:m_Moons)    delete moon;
}
void SolarSystem::_loadTestSystem(){
	new Skybox("Basic",this);

	Star* sun = new Star(glm::vec3(1,0.6f,0),glm::vec3(228.0f/255.0f,228.0f/255.0f,1),glm::vec3(0,0,0),6958000,"Sun",this);
	m_Stars.push_back(sun);
	Planet* p = new Planet("Earth",PLANET_TYPE_ROCKY,glm::vec3(-80000,0,1499000000),63710,"Earth",0.025f,this);
	m_Planets.push_back(p);

	player = new PlayerShip("Defiant","Defiant","USS Defiant",glm::vec3(0,0,1499000000),glm::vec3(1,1,1),nullptr,this);
	Ship* other = new Ship("Defiant","Defiant","USS Valiant",glm::vec3(3,0,1499000000),glm::vec3(1,1,1),nullptr,this);

	glm::vec3 offset = -player->getPosition();
	Scene* s =  Resources::getCurrentScene();
	for(auto obj:s->getObjects()){
		if(obj.second != player && obj.second->getParent() == nullptr){
			obj.second->translate(offset,false);
		}
	}
	for(auto obj:s->getLights()){
		if(obj.second->getParent() == nullptr){
			obj.second->translate(offset,false);
		}
	}
	player->setPosition(0,0,0);
}
void SolarSystem::_loadFromFile(std::string filename){
	unsigned int count = 0;
	boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
	for(std::string line; std::getline(str, line, '\n');){

		//remove \r from the line
		line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() );

		if(line[0] != '#'){//ignore commented lines
			if(count == 1){//this line has the system's name
				this->setName(line);
			}
			else if(count == 2){//this line has the system's skybox
				new Skybox(line,this);
			}
			if((line[0] == 'S' || line[0] == 'M' || line[0] == 'P' || line[0] == '*') && line[1] == ' '){//we got something to work with
				Planet* planetoid = nullptr;

				std::string token;
				std::istringstream stream(line);

				std::string NAME;
				std::string PARENT = "";
				float R,G,B,   R1,G1,B1;
				float ATMOSPHERE_HEIGHT;
				std::string LIGHTCOLOR;
				std::string TYPE;
				std::string TEXTURE = "Textures/Planets/";
				std::string MATERIAL_NAME = "";
				unsigned long long RADIUS;
				unsigned long long POSITION;

				while(std::getline(stream, token, ' ')) {
					size_t pos = token.find("=");

					std::string key = token.substr(0, pos);
					std::string value = token.substr(pos + 1, std::string::npos);

					if(key == "name")                  NAME = value;
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
					else if(key == "texture"){    
						TEXTURE += value;
						MATERIAL_NAME = value.substr(0,value.size()-4);
					}

				}
				float randDegree = rand() * 360.0f;
				float xPos = sin(randDegree) * static_cast<float>(POSITION);
				float zPos = cos(randDegree) * static_cast<float>(POSITION);

				if(MATERIAL_NAME != ""){
					std::string normalFile = "";
					std::string glowFile = "";
					std::string extention;

					//get file extension
					for(unsigned int i = TEXTURE.length() - 4; i < TEXTURE.length(); i++) extention += tolower(TEXTURE.at(i));

					std::string normFile = TEXTURE.substr(0,TEXTURE.size()-4);
					normFile += "Norm" + extention;
					if(boost::filesystem::exists(normFile)){
						normalFile = normFile;
					}
					std::string gloFile = TEXTURE.substr(0,TEXTURE.size()-4);
					gloFile += "Glow" + extention;
					if(boost::filesystem::exists(gloFile)){
						glowFile = gloFile;
					}


					Resources::addMaterial(MATERIAL_NAME,TEXTURE,normalFile,glowFile);
				}

				if(line[0] == 'S'){//Sun
					Star* star = new Star(glm::vec3(R,G,B),glm::vec3(R1,G1,B1),glm::vec3(xPos,0,zPos),static_cast<float>(RADIUS),NAME,this);
					m_Stars.push_back(star);
				}
				else if(line[0] == 'P'){//Planet
					PlanetType PLANET_TYPE;
					if(TYPE == "Rock") PLANET_TYPE = PLANET_TYPE_ROCKY;
					else if(TYPE == "Ice") PLANET_TYPE = PLANET_TYPE_ICE;
					else if(TYPE == "GasGiant") PLANET_TYPE = PLANET_TYPE_GAS_GIANT;
					else if(TYPE == "IceGiant") PLANET_TYPE = PLANET_TYPE_ICE_GIANT;
					else if(TYPE == "Asteroid") PLANET_TYPE = PLANET_TYPE_ASTEROID;
					planetoid = new Planet(MATERIAL_NAME,PLANET_TYPE,glm::vec3(xPos,0,zPos),static_cast<float>(RADIUS),NAME,ATMOSPHERE_HEIGHT,this);
					if(PARENT != ""){
						planetoid->translate(getObjects()[PARENT]->getPosition(),false);
					}
					m_Planets.push_back(planetoid);
				}
				else if(line[0] == 'M'){//Moon
					PlanetType PLANET_TYPE;
					if(TYPE == "Rock") PLANET_TYPE = PLANET_TYPE_ROCKY;
					else if(TYPE == "Ice") PLANET_TYPE = PLANET_TYPE_ICE;
					else if(TYPE == "GasGiant") PLANET_TYPE = PLANET_TYPE_GAS_GIANT;
					else if(TYPE == "IceGiant") PLANET_TYPE = PLANET_TYPE_ICE_GIANT;
					else if(TYPE == "Asteroid") PLANET_TYPE = PLANET_TYPE_ASTEROID;
					planetoid = new Planet(MATERIAL_NAME,PLANET_TYPE,glm::vec3(xPos,0,zPos),static_cast<float>(RADIUS),NAME,ATMOSPHERE_HEIGHT,this);
					if(PARENT != ""){
						planetoid->translate(getObjects()[PARENT]->getPosition(),false);
					}
					m_Moons.push_back(planetoid);
				}
				else if(line[0] == '*'){//Player ship
					if(PARENT != ""){
						xPos += getObjects()[PARENT]->getPosition().x;
						zPos += getObjects()[PARENT]->getPosition().z;
					}
					player = new PlayerShip("Defiant","Defiant",NAME,glm::vec3(xPos,0,zPos),glm::vec3(1,1,1),nullptr,this);

				}
			}
		}
		count++;
	}
	glm::vec3 offset = -player->getPosition();
	Scene* s =  Resources::getCurrentScene();
	for(auto obj:Resources::getCurrentScene()->getObjects()){
		if(obj.second != player && obj.second->getParent() == nullptr){
			obj.second->translate(offset,false);
		}
	}
	for(auto obj:Resources::getCurrentScene()->getLights()){
		if(obj.second->getParent() == nullptr){
			obj.second->translate(offset,false);
		}
	}
	player->setPosition(0,0,0);
}
void SolarSystem::_loadRandomly(){
	#pragma region Skybox
	//get random skybox folder from the skybox directory
	std::vector<std::string> folders;
	std::string path = "Textures/Skyboxes/";
	if ( boost::filesystem::exists( path ) ) {
		boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
		for ( boost::filesystem::directory_iterator itr( path );itr != end_itr;++itr ){
			if ( boost::filesystem::is_directory(itr->status()) ){
				std::string path_name = boost::lexical_cast<std::string>(itr->path());
				std::replace(path_name.begin(),path_name.end(),'\\','/');
				boost::erase_all(path_name,"\"");
				path_name = path_name.substr(18,path_name.size());
				folders.push_back(path_name);

			}
		}
	}
	unsigned int random_skybox_index = static_cast<unsigned int>((rand() % folders.size()));
	std::string skybox = folders.at(random_skybox_index);
	new Skybox(skybox);
	#pragma endregion

	//solar systems are normally mono - seven. Load 1 - 7 stars (2 being most common, 7 most rare)
	#pragma region ConstructStars
	unsigned int percent = static_cast<unsigned int>(rand() % 1000);
	unsigned int numberOfStars = 7;



	for(unsigned int i = 0; i < numberOfStars; i++){
		Star* star = nullptr;
		//star sizes: most big: 1,800 * the sun's size, smallest: 14% the size of the sun
		float radius = static_cast<float>(200000.0f + (rand() % 1252440000));
		float position = static_cast<float>(radius * 2.0f + (rand() % 41252440000));

		float R = static_cast<float>((rand()%100)/100.0f);
		float G = static_cast<float>((rand()%100)/100.0f);
		float B = static_cast<float>((rand()%100)/100.0f);

		glm::vec3 starColor = glm::vec3(R,G,B);
		glm::vec3 lightColor = glm::vec3(glm::min(1.0f,R+0.1f),glm::min(1.0f,G+0.1f),glm::min(1.0f,B+0.1f));
		float posX,posZ;

		float randomDegree = (rand() % 36000)/100.0f;
		posX = glm::sin(randomDegree) * position;
		posZ = glm::cos(randomDegree) * position;

		star = new Star(starColor,lightColor,glm::vec3(posX,0,posZ),radius,"Star " + boost::lexical_cast<std::string>(1 + i),this);
		m_Stars.push_back(star);
	}
	#pragma endregion

	#pragma region CheckStarPositions
	//check if any stars are too close to each other, and move them accordingly
	bool allStarsGood = true;
	glm::vec3 centerOfMassPosition;

	std::vector<float> starMasses;
	std::vector<glm::vec3> starPositions;
	float totalMasses = 0.0f;
	float biggestRadius = 0.0f;
	for(auto star:m_Stars){
		for(auto otherStar:m_Stars){
			if(star != otherStar){
				float biggerRadius = glm::max(star->getRadius(),otherStar->getRadius());
				unsigned long long dist = star->getDistanceLL(otherStar);
				if(dist < biggerRadius * 10.0f){
					allStarsGood = false;
				}
			}
		}
		biggestRadius = glm::max(star->getRadius(),biggestRadius);
		starPositions.push_back(star->getPosition());
		starMasses.push_back(star->getRadius());
		totalMasses += star->getRadius();
	}

	float numerator1 = 0,numerator2 = 0;
	for(unsigned int i = 0; i < starMasses.size(); i++){
		numerator1 += (starMasses.at(i) * starPositions.at(i).x);
		numerator2 += (starMasses.at(i) * starPositions.at(i).z);
	}
	centerOfMassPosition.x = (numerator1) / (totalMasses);
	centerOfMassPosition.z = (numerator2) / (totalMasses);

	while(!allStarsGood){
		for(auto star:m_Stars){
			glm::vec3 starPos = star->getPosition();
			glm::vec3 offset = starPos - centerOfMassPosition;
			glm::vec3 normOffset = glm::normalize(offset);
			star->translate(normOffset*biggestRadius*2.0f);
			star->update(1);
		}
		allStarsGood = true;
		for(auto star:m_Stars){
			for(auto otherStar:m_Stars){
				if(star != otherStar){
					float biggerRadius = glm::max(star->getRadius(),otherStar->getRadius());
					unsigned long long dist = star->getDistanceLL(otherStar);
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
	std::string planets_path = "Textures/Planets/Random/Planet";
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

			float maxPositionAwayFromSun = glm::abs(glm::length(star->getPosition() - centerOfMassPosition));
			maxPositionAwayFromSun -= (maxPositionAwayFromSun / 4.0f);
			if(m_Stars.size() == 1)
				maxPositionAwayFromSun = glm::abs(glm::length(star->getRadius()*2000.0f));
			float minPositionAwayFromSun = star->getRadius() * 4.0f;
			float positionAwayFromSun = glm::max(minPositionAwayFromSun,rand() *maxPositionAwayFromSun);

			float posX,posZ;
			float randomDegree = rand() % 36000 / 100.0f;
			posX = sin(randomDegree) * positionAwayFromSun;
			posZ = cos(randomDegree) * positionAwayFromSun;

			float RADIUS = static_cast<float>(500 + rand() % 85000);
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
			if(positionAwayFromSun > 155 * star->getRadius() && positionAwayFromSun < 327 * star->getRadius()){
				float chance = rand() % 1000 / 1000.0f;
				if(chance > 0.5f)
					ATMOSPHERE_HEIGHT = 0.025f;
			}

			//now to get a random planet material based on a random texture based on planet type
			std::string MATERIAL_NAME,FOLDER,normalFile,glowFile = "";
			std::string base_name;
			std::string base_path = planets_path;
			if(PLANET_TYPE == PLANET_TYPE_ROCKY){
				FOLDER = planets_folders["Rocky"];
				base_path += "/Rocky/";
				std::vector<std::string> textures = planet_textures["Rocky"];
				std::string texture = textures.at(rand() % textures.size());
				base_name = texture.substr(base_path.size(),texture.size()-1);
				FOLDER += "/" + base_name;
				MATERIAL_NAME = FOLDER;
				FOLDER += "/";
			}
			else if(PLANET_TYPE == PLANET_TYPE_ICE){
				FOLDER = planets_folders["Ice"];
				base_path += "/Ice/";
				std::vector<std::string> textures = planet_textures["Ice"];
				std::string texture = textures.at(rand() % textures.size());
				base_name = texture.substr(base_path.size(),texture.size()-1);
				FOLDER += "/" + base_name;
				MATERIAL_NAME = FOLDER;
				FOLDER += "/";
			}
			else if(PLANET_TYPE == PLANET_TYPE_GAS_GIANT){
			}
			else if(PLANET_TYPE == PLANET_TYPE_ICE_GIANT){
			}


			std::string diffuseFile = FOLDER + base_name + ".png";
			std::string normFile = FOLDER + base_name + "Norm.png";
			std::string gloFile = FOLDER + base_name + "Glow.png";
			if(boost::filesystem::exists(normFile)){ normalFile = normFile; }
			if(boost::filesystem::exists(gloFile)){ glowFile = gloFile; }

			if(boost::filesystem::exists(diffuseFile)){
				Resources::addMaterial(MATERIAL_NAME,diffuseFile,normalFile,glowFile);

				planet = new Planet(MATERIAL_NAME,PLANET_TYPE,glm::vec3(posX,0,posZ),RADIUS,"Planet " + boost::lexical_cast<std::string>(i + 1),ATMOSPHERE_HEIGHT,this);

				float R = 0.3f,G = 0.3f,B = 0.3f;
				R += (rand() % 1000 / 1000) * 0.5f;
				G += (rand() % 1000 / 1000) * 0.2f;
				B += (rand() % 1000 / 1000) * 0.3f;
				planet->setColor(R,G,B,1);
				m_Planets.push_back(planet);
			}
		}
	}
	//Then load moons. Generally the number of moons depends on the type of planet. Gas giants have more moons than rocky planets, etc..

	player = new PlayerShip("Defiant","Defiant","USS Defiant",glm::vec3(0,0,0),glm::vec3(1,1,1),nullptr,this);

	#pragma region ChangeCoordSpace
	glm::vec3 offset = -player->getPosition();
	Scene* s =  Resources::getCurrentScene();
	for(auto obj:s->getObjects()){
		if(obj.second != player && obj.second->getParent() == nullptr){ obj.second->translate(offset,false); }
	}
	for(auto obj:s->getLights()){
		if(obj.second->getParent() == nullptr){ obj.second->translate(offset,false); }
	}
	player->setPosition(0,0,0);
	#pragma endregion
}
void SolarSystem::update(float dt){
	if(Keyboard::isKeyDown("esc"))
		exit(EXIT_SUCCESS);
}
void SolarSystem::render()
{
}
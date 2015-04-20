#include "Engine_Events.h"
#include "SolarSystem.h"
#include "Engine_Resources.h"
#include "Planet.h"
#include "GameCamera.h"
#include "Ship.h"
#include "Skybox.h"
#include "Light.h"

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
		//SolarSystem::_loadRandomly();
		SolarSystem::_loadTestSystem();
	}
	else{
		SolarSystem::_loadFromFile(file);
	}
}
SolarSystem::~SolarSystem(){
	for(auto star:m_Stars)    delete star.second;
	for(auto planet:m_Planets)delete planet.second;
	for(auto moon:m_Moons)    delete moon.second;
}
void SolarSystem::_loadTestSystem(){
	new Skybox("Basic",this);

	Star* sun = new Star(glm::vec3(1,0.6f,0),glm::vec3(228.0f/255.0f,228.0f/255.0f,1),glm::vec3(0,0,0),6958000,"Sun",this);
	m_Stars["Sun"] = sun;
	Planet* p = new Planet("Earth",PLANET_TYPE_ROCKY,glm::vec3(-80000,0,1499000000),63710,"Earth",this);
	m_Planets["Earth"] = p;

	player = new PlayerShip("Defiant","Defiant","USS Defiant",glm::vec3(0,0,1499000000),glm::vec3(1,1,1),nullptr,this);
	Ship* other = new Ship("Defiant","Defiant","USS Valiant",glm::vec3(3,0,1499000000),glm::vec3(1,1,1),nullptr,this);

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
				Planet* planetoid;

				std::string token;
				std::istringstream stream(line);

				std::string NAME;
				std::string PARENT = "";
				float R,G,B,   R1,G1,B1;
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

					if(key == "name")            NAME = value;
					else if(key == "radius")     RADIUS = stoull(value)*10;
					else if(key == "r")			 R = stof(value);
					else if(key == "g")			 G = stof(value);
					else if(key == "b")			 B = stof(value);
					else if(key == "r1")         R1 = stof(value);
					else if(key == "g1")         G1 = stof(value);
					else if(key == "b1")         B1 = stof(value);
					else if(key == "position")   POSITION = stoull(value)*10; 
					else if(key == "parent")     PARENT = value;
					else if(key == "type")       TYPE = value;
					else if(key == "texture"){    
						TEXTURE += value;
						MATERIAL_NAME = value.substr(0,value.size()-4);
					}

				}
				float randDegree = rand() * 360.0f;
				float xPos = sin(randDegree) * static_cast<float>(POSITION);
				float zPos = cos(randDegree) * static_cast<float>(POSITION);

				if(MATERIAL_NAME != "")
					Resources::addMaterial(MATERIAL_NAME,TEXTURE,"","");

				if(line[0] == 'S'){//Sun
					planetoid = new Star(glm::vec3(R,G,B),glm::vec3(R1,G1,B1),glm::vec3(xPos,0,zPos),static_cast<float>(RADIUS),NAME,this);
					m_Stars[NAME] = planetoid;
				}
				else if(line[0] == 'P'){//Planet
					PlanetType PLANET_TYPE;
					if(TYPE == "Rock") PLANET_TYPE = PLANET_TYPE_ROCKY;
					else if(TYPE == "Gas") PLANET_TYPE = PLANET_TYPE_GAS_GIANT;
					else if(TYPE == "Asteroid") PLANET_TYPE = PLANET_TYPE_ASTEROID;
					planetoid = new Planet(MATERIAL_NAME,PLANET_TYPE,glm::vec3(xPos,0,zPos),static_cast<float>(RADIUS),NAME,this);
					if(PARENT != ""){
						planetoid->translate(getObjects()[PARENT]->getPosition(),false);
					}
					m_Planets[NAME] = planetoid;
				}
				else if(line[0] == 'M'){//Moon
					PlanetType PLANET_TYPE;
					if(TYPE == "Rock") PLANET_TYPE = PLANET_TYPE_ROCKY;
					else if(TYPE == "Gas") PLANET_TYPE = PLANET_TYPE_GAS_GIANT;
					else if(TYPE == "Asteroid") PLANET_TYPE = PLANET_TYPE_ASTEROID;
					planetoid = new Planet(MATERIAL_NAME,PLANET_TYPE,glm::vec3(xPos,0,zPos),static_cast<float>(RADIUS),NAME,this);
					if(PARENT != ""){
						planetoid->translate(getObjects()[PARENT]->getPosition(),false);
					}
					m_Moons[NAME] = planetoid;
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
	//solar systems are normally mono - trinary. Load 1 - 3 stars

	//Then load planets. Generally the more stars, the more planets

	//Then load moons. Generally the number of moons depends on the type of planet. Gas giants have more moons than rocky planets.
}
void SolarSystem::update(float dt){
	if(Keyboard::isKeyDown("esc"))
		exit(EXIT_SUCCESS);
}
void SolarSystem::render()
{
}
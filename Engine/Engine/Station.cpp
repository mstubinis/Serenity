#include "Station.h"

Station::Station(std::string mesh,std::string mat,glm::dvec3 pos, glm::vec3 scl,std::string name,Engine::Physics::Collision* col,Scene* scene):ObjectDynamic(mesh,mat,pos,scl,name,col,scene){
}
Station::~Station(){
}
void Station::update(float dt){
}
void Station::render(){
}
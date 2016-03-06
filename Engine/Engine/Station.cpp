#include "Station.h"

Station::Station(std::string mesh,std::string mat,glm::v3 pos, glm::vec3 scl,std::string name,Collision* col,Scene* scene):ObjectDynamic(mesh,mat,pos,scl,name,col,scene){
}
Station::~Station(){
}
void Station::update(float dt){
}
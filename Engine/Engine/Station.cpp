#include "Station.h"

Station::Station(std::string mesh,std::string mat,glm::vec3 pos, glm::vec3 scl,std::string name,btCollisionShape* col,Scene* scene):ObjectDynamic(mesh,mat,pos,scl,name,col,scene){
}
Station::~Station(){
}
void Station::Update(float dt){
}
void Station::Render(){
}
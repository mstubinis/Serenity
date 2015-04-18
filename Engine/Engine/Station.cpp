#include "Station.h"

Station::Station(std::string mesh,std::string mat,glm::vec3 pos, glm::vec3 scl,std::string name,btCollisionShape* col):ObjectDynamic(mesh,mat,pos,scl,name,col){
}
Station::~Station(){
}
void Station::Update(float dt){
}
void Station::Render(){
}
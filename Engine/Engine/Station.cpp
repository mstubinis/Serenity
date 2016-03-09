#include "Station.h"
#include "Engine_Math.h"


Station::Station(std::string mesh,std::string mat,glm::v3 pos, glm::vec3 scl,std::string name,Collision* col,Scene* scene):ObjectDynamic(mesh,mat,pos,scl,name,col,scene){
}
Station::~Station(){
}
void Station::update(float dt){
}
void Station::addStationPart(StationPart* _part, unsigned int _part_port, StationPart* _parent, unsigned int _parent_port){
	if(m_StationParts.size() == 0){
		_part->parent = nullptr;
	}
	else{
		// modify position of part
		_part->displayItem->position = _parent->connectorsInfo->connectorLocations.at(_parent_port);
		_part->displayItem->position += _part->connectorsInfo->connectorLocations.at(_part_port);

		//"align-to" the part to the parent properly
		Engine::Math::alignTo(_part->displayItem->orientation,_part->connectorsInfo->connectorDirections.at(_parent_port));
		Engine::Math::alignTo(_part->displayItem->orientation,_part->connectorsInfo->connectorDirections.at(_part_port));

		_part->usedPartsIndices.push_back(_part_port);
		_parent->usedPartsIndices.push_back(_parent_port);
		_part->parent = _parent;
	}
	m_StationParts.push_back(_part);
}
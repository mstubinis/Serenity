#include "Engine_ResourceBasic.h"

EngineResource::EngineResource(std::string name){
	m_Name = new std::string(name);
}
EngineResource::~EngineResource(){
	delete m_Name;
}
void EngineResource::setName(std::string name){
	delete m_Name;
	m_Name = new std::string(name);
}
std::string& EngineResource::name(){ 
	return (*m_Name); 
}
std::string* EngineResource::namePtr(){
	return m_Name;
}
#include "Engine_ResourceBasic.h"

EngineResource::EngineResource(std::string name){
	m_Name = boost::make_shared<std::string>(name);
}
EngineResource::~EngineResource(){
	m_Name.reset();
}
void EngineResource::setName(std::string name){	
	*(m_Name.get()) = name;
}
std::string& EngineResource::name(){
	return *(m_Name.get()); 
}
boost::shared_ptr<std::string>& EngineResource::namePtr(){
	return m_Name;
}
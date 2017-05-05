#include "Engine_ResourceBasic.h"

EngineResource::EngineResource(std::string name){
    m_Name = name;
    m_IsLoaded = false;
	m_UsageCount = 0;
}
EngineResource::~EngineResource(){
}
void EngineResource::setName(std::string name){	
    m_Name = name;
}
std::string& EngineResource::name(){
    return m_Name;
}
void EngineResource::load(){
	if(!m_IsLoaded){
		m_IsLoaded = true;
	}
}
void EngineResource::unload(){
	if(m_IsLoaded && m_UsageCount == 0){
		m_IsLoaded = false;
	}
}
bool EngineResource::isLoaded(){ return m_IsLoaded; }
uint EngineResource::useCount(){ return m_UsageCount; }
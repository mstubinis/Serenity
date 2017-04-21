#include "Engine_ResourceBasic.h"

EngineResource::EngineResource(std::string name){
    m_Name = name;
    m_IsLoaded = false;
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
    m_IsLoaded = true;
}
void EngineResource::unload(){
    m_IsLoaded = false;
}
bool EngineResource::isLoaded(){ return m_IsLoaded; }
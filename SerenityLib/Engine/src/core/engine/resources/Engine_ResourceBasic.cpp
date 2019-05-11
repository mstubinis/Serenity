#include "core/engine/resources/Engine_ResourceBasic.h"
#include "core/engine/Engine_Utils.h"

#include <iostream>

using namespace std;

EngineResource::EngineResource(string name){
    m_Name = name;
    m_IsLoaded = false;
    m_UsageCount = 0;
}
EngineResource::~EngineResource(){
}
void EngineResource::setName(string name){ 
    m_Name = name;
}
string& EngineResource::name(){
    return m_Name;
}
void EngineResource::load(){
    if(!m_IsLoaded){
        m_IsLoaded = true;
        std::cout << ": " << m_Name << " - loaded." << std::endl;
    }
}
void EngineResource::unload(){
    if(m_IsLoaded /*&& m_UsageCount == 0*/){
        m_IsLoaded = false;
        std::cout << ": " << m_Name << " - unloaded." << std::endl;
    }
}
bool EngineResource::isLoaded(){ return m_IsLoaded; }
uint EngineResource::useCount(){ return m_UsageCount; }
void EngineResource::incrementUseCount(){ ++m_UsageCount; }
void EngineResource::decrementUseCount(){ if(m_UsageCount > 0) --m_UsageCount; }
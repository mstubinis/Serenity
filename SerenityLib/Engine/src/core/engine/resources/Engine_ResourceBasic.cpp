#include <core/engine/resources/Engine_ResourceBasic.h>

#include <iostream>

using namespace std;

EngineResource::EngineResource(const string& name){
    m_Name = name;
    m_IsLoaded = false;
    m_UsageCount = 0;
}
EngineResource::~EngineResource(){
}
void EngineResource::setName(const string& name){
    m_Name = name;
}
const string& EngineResource::name() const {
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
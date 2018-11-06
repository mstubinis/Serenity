#include "Engine_ResourceBasic.h"
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
        cout << ": " << m_Name << " - loaded." << endl;
    }
}
void EngineResource::unload(){
    if(m_IsLoaded && m_UsageCount == 0){
        m_IsLoaded = false;
        cout << ": " << m_Name << " - unloaded." << endl;
    }
}
bool EngineResource::isLoaded(){ return m_IsLoaded; }
uint EngineResource::useCount(){ return m_UsageCount; }
void EngineResource::incrementUseCount(){ ++m_UsageCount; }
void EngineResource::decrementUseCount(){ if(m_UsageCount > 0) --m_UsageCount; }
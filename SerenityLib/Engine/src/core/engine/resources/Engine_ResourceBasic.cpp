#include <core/engine/resources/Engine_ResourceBasic.h>
#include <core/engine/utils/Logger.h>

using namespace std;

EngineResource::EngineResource(const ResourceType::Type type, const string& name){
    m_Name         = name;
    m_IsLoaded     = false;
    m_UsageCount   = 0;
    m_ResourceType = type;
}
EngineResource::~EngineResource(){
}
void EngineResource::setName(const string& name){
    m_Name = name;
}
const ResourceType::Type& EngineResource::type() const {
    return m_ResourceType;
}
const string& EngineResource::name() const {
    return m_Name;
}
void EngineResource::load(){
    if(!m_IsLoaded){
        m_IsLoaded = true;
        ENGINE_LOG(typeid(*this).name() << ": " << m_Name << " - loaded.");
    }
}
void EngineResource::unload(){
    if(m_IsLoaded /*&& m_UsageCount == 0*/){
        m_IsLoaded = false;
        ENGINE_LOG(typeid(*this).name() << ": " << m_Name << " - unloaded.");
    }
}
const bool EngineResource::isLoaded() const {
    return m_IsLoaded; 
}
const unsigned int EngineResource::useCount() const {
    return m_UsageCount; 
}
void EngineResource::incrementUseCount(){ 
    ++m_UsageCount; 
}
void EngineResource::decrementUseCount(){ 
    if (m_UsageCount > 0) {
        --m_UsageCount;
    }
}
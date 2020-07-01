#include <core/engine/resources/Resource.h>
#include <core/engine/utils/Logger.h>

using namespace std;

void Resource::load(){
    if(!m_IsLoaded){
        m_IsLoaded = true;
        ENGINE_LOG(typeid(*this).name() << ": " << m_Name << " - loaded.");
    }
}
void Resource::unload(){
    if(m_IsLoaded /*&& m_UsageCount == 0*/){
        m_IsLoaded = false;
        ENGINE_LOG(typeid(*this).name() << ": " << m_Name << " - unloaded.");
    }
}

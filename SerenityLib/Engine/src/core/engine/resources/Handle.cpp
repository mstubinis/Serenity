#include <core/engine/resources/Handle.h>
#include <core/engine/Engine.h>

using namespace Engine;
using namespace std;

Handle::Handle() { 
    index   = 0; 
    version = 0; 
    type    = 0; 
}
Handle::Handle(uint32_t _index, uint32_t _version, uint32_t _type) {
    index   = _index; 
    version = _version; 
    type    = _type; 
}
Handle::operator uint32_t() const {
    return type << 27 | version << 12 | index; 
}
const bool Handle::null() const { 
    return (type == ResourceType::Empty) ? true : false; 
}
const EngineResource* Handle::get() const {
    if (null()) 
        return nullptr;
    return epriv::Core::m_Engine->m_ResourceManager.m_Resources->getAsFast<EngineResource>(index);
}
const EngineResource* Handle::operator ->() const { 
    return get(); 
}

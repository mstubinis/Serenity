#include <core/engine/resources/Handle.h>
#include <core/engine/system/Engine.h>
#include <core/engine/resources/ResourcesIncludes.h>

using namespace Engine;
using namespace std;

Handle::Handle() { 
    index   = 0; 
    version = 0; 
    type    = 0; 
}
Handle::Handle(const std::uint32_t index_, const std::uint32_t version_, const std::uint32_t type_) {
    index   = index_; 
    version = version_; 
    type    = type_; 
}
Handle::operator std::uint32_t() const {
    return type << 27 | version << 12 | index; 
}
const bool Handle::null() const { 
    return (type == ResourceType::Empty || index == 0); 
}
const EngineResource* Handle::get() const {
    return (null()) ? nullptr : priv::Core::m_Engine->m_ResourceManager.m_Resources.getAsFast<EngineResource>(index);
}
const EngineResource* Handle::operator ->() const { 
    return get(); 
}
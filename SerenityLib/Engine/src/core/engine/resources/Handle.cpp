#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/resources/Handle.h>
#include <core/engine/system/Engine.h>
#include <core/engine/resources/ResourcesIncludes.h>

Resource* Handle::internal_get_base() const noexcept {
    return (null()) ? nullptr : Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourcePool.getAsFast<Resource>(m_Index);
}
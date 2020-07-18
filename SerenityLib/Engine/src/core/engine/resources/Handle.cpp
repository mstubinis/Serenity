#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/resources/Handle.h>
#include <core/engine/system/Engine.h>
#include <core/engine/resources/ResourcesIncludes.h>

using namespace Engine;
using namespace std;

Resource* Handle::get_base() const noexcept {
    return (null()) ? nullptr : priv::Core::m_Engine->m_ResourceManager.m_Resources.getAsFast<Resource>(m_Index);
}
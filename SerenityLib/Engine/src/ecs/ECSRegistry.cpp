#include <core/engine/utils/PrecompiledHeader.h>
#include <ecs/ECSRegistry.h>

using namespace Engine::priv;

std::uint32_t ECSRegistry::m_LastIndex = 0U;
boost::unordered_map<boost_type_index, std::uint32_t> ECSRegistry::m_SlotMap;
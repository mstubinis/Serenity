#include <ecs/ECSRegistry.h>

using namespace Engine::priv;

unsigned int ECSRegistry::lastIndex = 0;
boost::unordered_map<boost_type_index, unsigned int> ECSRegistry::slotMap;
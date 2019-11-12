#include <ecs/ECSRegistry.h>

using namespace Engine::epriv;

unsigned int ECSRegistry::lastIndex = 0;
boost::unordered_map<boost_type_index, unsigned int> ECSRegistry::slotMap;
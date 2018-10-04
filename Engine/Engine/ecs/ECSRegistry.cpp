#include "ECSRegistry.h"

using namespace Engine::epriv;

uint ECSRegistry::lastIndex = 0;
boost::unordered_map<boost_type_index, uint> ECSRegistry::slotMap;
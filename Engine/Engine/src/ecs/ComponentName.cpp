#include "ComponentName.h"

using namespace Engine;
using namespace std;

#pragma region Component

ComponentName::ComponentName(Entity& _entity) : ComponentBaseClass(_entity){
    _data = "";
}
ComponentName::ComponentName(Entity& _entity, std::string& _name) : ComponentBaseClass(_entity) {
    _data = _name;
}
ComponentName::ComponentName(Entity& _entity, const char* _name) : ComponentBaseClass(_entity) {
    _data = _name;
}
ComponentName::~ComponentName() {
    _data = "";
}
const std::string& ComponentName::name() { return _data; }
const uint ComponentName::size() { return _data.size(); }

#pragma endregion


#pragma region System



#pragma endregion
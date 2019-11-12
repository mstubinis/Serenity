#include <ecs/ComponentBaseClass.h>

using namespace std;

ComponentBaseClass::ComponentBaseClass() {
    m_Owner = Entity::_null; 
}
ComponentBaseClass::ComponentBaseClass(const Entity& p_Owner) {
    m_Owner.data = p_Owner.data; 
}
Entity& ComponentBaseClass::getOwner() { 
    return m_Owner; 
}
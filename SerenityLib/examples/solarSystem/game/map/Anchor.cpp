#include "Anchor.h"
#include "Map.h"
#include <ecs/ComponentBody.h>

using namespace std;

Anchor::Anchor(const string& name, Map& map, const decimal& x, const decimal& y, const decimal& z) : EntityWrapper(map){
    auto& body = *addComponent<ComponentBody>();
    body.setPosition(x, y, z);
    m_Name = name;
}
Anchor::Anchor(const string& name, Map& map, const glm_vec3& position) : Anchor(name, map, position.x, position.y, position.z) {
}
Anchor::~Anchor() {

}

void Anchor::update(const double& dt) {

}
void Anchor::addChild(Anchor* anchor) {
    m_Children.push_back(anchor); 
}
const glm_vec3 Anchor::getPosition() {
    return getComponent<ComponentBody>()->position();
}
const vector<Anchor*>& Anchor::getChildren() const {
    return m_Children;
}
const string& Anchor::getName() const {
    return m_Name;
}
void Anchor::setName(const string& name) {
    m_Name = name;
}
Anchor* Anchor::getChild(const string& childName) {
    for (auto& child_itr : m_Children) {
        if (child_itr->getName() == childName) {
            return child_itr;
        }
    }
    return nullptr;
}
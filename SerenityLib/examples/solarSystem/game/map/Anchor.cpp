#include "Anchor.h"
#include "Map.h"
#include <ecs/ComponentBody.h>

using namespace std;

Anchor::Anchor(Map& map, const float& x, const float& y, const float& z) : EntityWrapper(map){
    //m_Midpoint = glm::vec3(x, y, z);
    auto& body = *addComponent<ComponentBody>();
    body.setPosition(x, y, z);
}
Anchor::Anchor(Map& map, const glm::vec3& position) : Anchor(map, position.x, position.y, position.z) {
}
Anchor::~Anchor() {

}

void Anchor::update(const double& dt) {

}
void Anchor::addChild(const string& key, Anchor* anchor) {
    if (!m_Children.count(key)) {
        m_Children.emplace(key, anchor);
    }
}
const glm::vec3& Anchor::getPosition() {
    return getComponent<ComponentBody>()->position();
}
const std::unordered_map<std::string, Anchor*>& Anchor::getChildren() const {
    return m_Children;
}
void Anchor::finalize_all() {
    //finalize_this_only();
    //for (auto& child : m_Children) {
    //    child.second->finalize_all();
    //}
}

void Anchor::finalize_this_only() {
    //m_Midpoint = glm::vec3(0.0f);
    //m_Midpoint += getPosition();
    //unsigned int i = 1;
    //for (auto& child : m_Children) {
    //    m_Midpoint += child.second->getPosition();
    //    ++i;
    //}
    //m_Midpoint /= i;
}
//const glm::vec3& Anchor::getMidpoint() const {
//    return m_Midpoint;
//}
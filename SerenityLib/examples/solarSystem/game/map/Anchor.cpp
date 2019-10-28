#include "Anchor.h"
#include "Map.h"
#include <ecs/ComponentBody.h>

using namespace std;

Anchor::Anchor(Map& map, const decimal& x, const decimal& y, const decimal& z) : EntityWrapper(map){
    auto& body = *addComponent<ComponentBody>();
    body.setPosition(x, y, z);
}
Anchor::Anchor(Map& map, const glm_vec3& position) : Anchor(map, position.x, position.y, position.z) {
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
const glm_vec3 Anchor::getPosition() {
    return getComponent<ComponentBody>()->position();
}
const unordered_map<std::string, Anchor*>& Anchor::getChildren() const {
    return m_Children;
}

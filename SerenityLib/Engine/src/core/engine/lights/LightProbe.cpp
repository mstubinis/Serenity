#include "LightProbe.h"
#include <ecs/ComponentBody.h>

using namespace std;

LightProbe::LightProbe(ProbeType::Type type, const glm::vec3& position, Scene* scene) : Entity(*scene), m_Camera(glm::radians(90.0f), 1.0f, 0.1f, 3000000.0f, scene), m_Viewport() {
    m_ProbeType  = type;

    addComponent<ComponentBody>();
    getComponent<ComponentBody>()->setPosition(position);

    m_Viewport.setCamera(m_Camera);
}
LightProbe::~LightProbe() {
    destroy();
}

void LightProbe::addIgnoredEntity(Entity entity) {
    if (!entity.null()) {
        for (auto& itr : m_Ignored) {
            if (entity == itr) {
                return;
            }
        }
    }
    m_Ignored.push_back(entity);
}
const Camera& LightProbe::getCamera() const {
    return m_Camera;
}
const Viewport& LightProbe::getViewport() const {
    return m_Viewport;
}
void LightProbe::update(const float dt) {
    m_FrameCount += 1_uc;
    if (m_FrameCount > 5_uc) {
        m_FrameCount = 0_uc;
    }
}
void LightProbe::activate(bool active) {
    m_Active = active;
}
void LightProbe::deactivate() {
    m_Active = false;
}
void LightProbe::setProbeType(ProbeType::Type type) {
    m_ProbeType = type;
}
LightProbe::ProbeType::Type LightProbe::getProbeType() const {
    return m_ProbeType;
}
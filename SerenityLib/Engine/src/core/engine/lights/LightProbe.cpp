#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lights/LightProbe.h>
#include <ecs/ComponentBody.h>

LightProbe::LightProbe(ProbeType::Type type, const glm::vec3& position, Scene* scene) 
    : Entity{ *scene }
    , m_Camera{ glm::radians(90.0f), 1.0f, 0.1f, 3000000.0f, scene }
    , m_Viewport{}
    , m_ProbeType{ type }
{
    addComponent<ComponentBody>();
    getComponent<ComponentBody>()->setPosition(position);

    m_Viewport.setCamera(m_Camera);
}
LightProbe::~LightProbe() {
    destroy();
}
void LightProbe::addIgnoredEntity(Entity entity) {
    if (!entity.null()) {
        for (auto itr : m_Ignored) {
            if (entity == itr) {
                return;
            }
        }
    }
    m_Ignored.emplace_back(entity);
}
void LightProbe::update(const float dt) {
    m_FrameCount += 1_uc;
    if (m_FrameCount > 5_uc) {
        m_FrameCount = 0_uc;
    }
}
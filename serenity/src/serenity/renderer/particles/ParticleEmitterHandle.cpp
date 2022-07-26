#include <serenity/renderer/particles/ParticleEmitterHandle.h>

#include <serenity/system/Engine.h>


ParticleEmitterHandle::ParticleEmitterHandle(uint32_t id, uint32_t sceneID)
	: m_ID{ id }
	, m_SceneID{ sceneID }
{}
ParticleEmitterHandle::ParticleEmitterHandle(uint32_t id, Scene& scene)
	: ParticleEmitterHandle{ id, scene.id() }
{}

void ParticleEmitterHandle::setPosition(decimal x, decimal y, decimal z) {
	auto scene = Engine::getResourceManager().getSceneByID(m_SceneID);
	assert(scene != nullptr);
	auto transform = Engine::priv::PublicScene::GetParticleEmitters(*scene).get(*this)->getComponent<ComponentTransform>();
	assert(transform != nullptr);
	transform->setPosition(x, y, z);
}
void ParticleEmitterHandle::setPosition(const glm_vec3& position) {
	setPosition(position[0], position[1], position[2]);
}
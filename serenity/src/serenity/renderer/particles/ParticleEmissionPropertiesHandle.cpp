#include <serenity/renderer/particles/ParticleEmissionPropertiesHandle.h>

#include <serenity/resources/Engine_Resources.h>

namespace {
	inline Engine::view_ptr<ParticleEmissionProperties> get_prop(ParticleEmissionPropertiesHandle handle) noexcept {
		return Engine::Resources::getParticleEmissionProperties(handle);
	}
}

float ParticleEmissionPropertiesHandle::getTimer() const noexcept {
	return (*this) ? get_prop(*this)->getTimer() : 0.0f;
}
float ParticleEmissionPropertiesHandle::getLifetime() const noexcept {
	return (*this) ? get_prop(*this)->getLifetime() : 0.0f;
}
float ParticleEmissionPropertiesHandle::getSpawnRate() const noexcept {
	return (*this) ? get_prop(*this)->getSpawnRate() : 0.0f;
}
float ParticleEmissionPropertiesHandle::getSpawnTimer() const noexcept {
	return (*this) ? get_prop(*this)->getSpawnTimer() : 0.0f;
}
Engine::view_ptr<Material> ParticleEmissionPropertiesHandle::getParticleMaterialRandom() noexcept {
	return (*this) ? &get_prop(*this)->getParticleMaterialRandom() : nullptr;
}
Engine::view_ptr<Material> ParticleEmissionPropertiesHandle::getParticleMaterial(size_t index) noexcept {
	return (*this) ? &get_prop(*this)->getParticleMaterial(index) : nullptr;
}
bool ParticleEmissionPropertiesHandle::addMaterial(Handle materialHandle) {
	return (*this) ? get_prop(*this)->addMaterial(materialHandle) : false;
}
bool ParticleEmissionPropertiesHandle::addMaterial(Material& material) {
	return (*this) ? get_prop(*this)->addMaterial(material) : false;
}
bool ParticleEmissionPropertiesHandle::setSpawnTimer(const float amount) noexcept {
	if (*this) {
		get_prop(*this)->setSpawnTimer(amount);
		return true;
	}
	return false;
}
bool ParticleEmissionPropertiesHandle::setColorFunctor(ParticleEmissionProperties::color_func&& functor) noexcept {
	if (*this) {
		get_prop(*this)->setColorFunctor(std::move(functor));
		return true;
	}
	return false;
}
bool ParticleEmissionPropertiesHandle::setChangeInAngularVelocityFunctor(ParticleEmissionProperties::change_in_angular_velocity_func&& functor) noexcept {
	if (*this) {
		get_prop(*this)->setChangeInAngularVelocityFunctor(std::move(functor));
		return true;
	}
	return false;
}
bool ParticleEmissionPropertiesHandle::setChangeInVelocityFunctor(ParticleEmissionProperties::change_in_velocity_func&& functor) noexcept {
	if (*this) {
		get_prop(*this)->setChangeInVelocityFunctor(std::move(functor));
		return true;
	}
	return false;
}
bool ParticleEmissionPropertiesHandle::setChangeInScaleFunctor(ParticleEmissionProperties::change_in_scale_func&& functor) noexcept {
	if (*this) {
		get_prop(*this)->setChangeInScaleFunctor(std::move(functor));
		return true;
	}
	return false;
}
bool ParticleEmissionPropertiesHandle::setInitialVelocityFunctor(ParticleEmissionProperties::initial_velocity_func&& functor) noexcept {
	if (*this) {
		get_prop(*this)->setInitialVelocityFunctor(std::move(functor));
		return true;
	}
	return false;
}
bool ParticleEmissionPropertiesHandle::setInitialScaleFunctor(ParticleEmissionProperties::initial_scale_func&& functor) noexcept {
	if (*this) {
		get_prop(*this)->setInitialScaleFunctor(std::move(functor));
		return true;
	}
	return false;
}
bool ParticleEmissionPropertiesHandle::setInitialAngularVelocityFunctor(ParticleEmissionProperties::initial_angular_velocity_func&& functor) noexcept {
	if (*this) {
		get_prop(*this)->setInitialAngularVelocityFunctor(std::move(functor));
		return true;
	}
	return false;
}

#include <serenity/renderer/particles/ParticleContainer.h>
#include <serenity/scene/Camera.h>
#include <serenity/math/MathCompression.h>

Engine::priv::ParticleContainer::ParticleContainer(uint32_t maxParticles) {
	m_ParticlesData.reserve(maxParticles);

	const auto packedZero = Engine::Math::PackFloat<ParticleFloatType>(0.0f);
	const auto packedOne = Engine::Math::PackFloat<ParticleFloatType>(1.0f);
	for (size_t id = 0; id < m_ParticlesData.capacity(); ++id) {
		m_ParticlesData.emplace_back(
			glm::vec<3, ParticleFloatType, glm::packed_highp>{packedZero, packedZero, packedZero}
		  , glm::vec<2, ParticleFloatType, glm::packed_highp>{packedOne, packedOne}
		  , packedZero
		  , std::numeric_limits<ParticleIDType>().max()
		  , Engine::Compression::pack4ColorsInto16Int(255, 255, 255, 255) //4 bits per color channel
		  , glm::vec3{0.0f, 0.0f, 0.0f}
		  , 0.0f
		  , 0.0f
		  , ParticleID(id)
		  , ParticleID(id)
		);
	}
}
ParticleID Engine::priv::ParticleContainer::addParticle(float x, float y, float z, float scaleX, float scaleY, ParticleIDType matID, uint8_t r, uint8_t g, uint8_t b, uint8_t a, const glm::vec3& velocity, float angularVelocity) {
	assert(m_ActiveEnd < m_ParticlesData.size());
	if (m_ActiveEnd >= m_ParticlesData.size()) {
		return std::numeric_limits<ParticleIDType>().max();
	}
	ParticleID freeID = m_ParticlesData.get<8>(m_ActiveEnd);

	const auto packedZero = Engine::Math::PackFloat<ParticleFloatType>(0.0f);
	const auto packedOne = Engine::Math::PackFloat<ParticleFloatType>(1.0f);

	setPosition(freeID, x, y, z);
	setScale(freeID, scaleX, scaleY);
	setColor(freeID, r, g, b, a);
	setVelocity(freeID, velocity.x, velocity.y, velocity.z);
	setAngularVelocity(freeID, angularVelocity);

	++m_ActiveEnd;
	return freeID;
}
bool Engine::priv::ParticleContainer::recycleParticle(ParticleID id) {
	if (m_ActiveEnd == 0) {
		return false;
	}
	if (m_ParticlesData.size() > 1) {
		const auto& index = getDataMember<8>(id);
		setTimer(id, 0.0f);
		m_ParticlesData.swap(index, m_ActiveEnd - 1);
		remapIndex(index);
	}
	m_ParticlesData.pop_back();
	--m_ActiveEnd;
	return true;
}

void Engine::priv::ParticleContainer::sort_particles(const Camera& camera) {
	sort_particles(camera.getPosition());
}
void Engine::priv::ParticleContainer::sort_particles(const glm::vec3& cameraPosition) {
	if (m_ActiveEnd == 0) {
		return;
	}
	auto sorter = [&cameraPosition](const auto& lhs, const auto& rhs) -> bool {
#if defined(ENGINE_PARTICLES_HALF_SIZE)
		const glm::vec3 lPos{ Math::ToFloat(lhs.x), Math::ToFloat(lhs.y), Math::ToFloat(lhs.z) };
		const glm::vec3 rPos{ Math::ToFloat(rhs.x), Math::ToFloat(rhs.y), Math::ToFloat(rhs.z) };
		return glm::length2(lPos - cameraPosition) > glm::length2(rPos - cameraPosition);
#else
		return glm::length2(lhs - cameraPosition) > glm::length2(rhs - cameraPosition);
#endif
	};
	m_ParticlesData.sort<0, std::execution::parallel_unsequenced_policy>(sorter, 0, m_ActiveEnd - 1, 3000);

	for (size_t i = 0; i < m_ActiveEnd; ++i) {
		remapIndex(i);
	}
}


void Engine::priv::ParticleContainer::translate(ParticleID id, const glm::vec3& translation) noexcept {
	setPosition(id, translation.x, translation.y, translation.z);
}
void Engine::priv::ParticleContainer::translate(ParticleID id, float x, float y, float z) noexcept {
	auto& item = getDataMember<0>(id);
	const glm::vec3 translation{ x, y, z };
	const glm::vec3 position{
		 Engine::Math::ToFloat<ParticleFloatType>(item.x)
		,Engine::Math::ToFloat<ParticleFloatType>(item.y)
		,Engine::Math::ToFloat<ParticleFloatType>(item.z)
	};
	item = glm::vec<3, ParticleFloatType, glm::packed_highp> {
		 Engine::Math::PackFloat<ParticleFloatType>(position.x + translation.x)
		,Engine::Math::PackFloat<ParticleFloatType>(position.y + translation.x)
		,Engine::Math::PackFloat<ParticleFloatType>(position.z + translation.x)
	};
}

void Engine::priv::ParticleContainer::setScale(ParticleID id, const glm::vec2& newScale) noexcept {
	setScale(id, newScale.x, newScale.y);
}
void Engine::priv::ParticleContainer::setScale(ParticleID id, float x, float y) noexcept {
	auto& item = getDataMember<1>(id);
	item = glm::vec<2, ParticleFloatType, glm::packed_highp>{
		 Engine::Math::PackFloat<ParticleFloatType>(x)
		,Engine::Math::PackFloat<ParticleFloatType>(y)
	};
}

void Engine::priv::ParticleContainer::setPosition(ParticleID id, const glm::vec3& newPosition) noexcept {
	setPosition(id, newPosition.x, newPosition.y, newPosition.z);
}
void Engine::priv::ParticleContainer::setPosition(ParticleID id, float x, float y, float z) noexcept {
	auto& item = getDataMember<0>(id);
	item = glm::vec<3, ParticleFloatType, glm::packed_highp>{
		 Engine::Math::PackFloat<ParticleFloatType>(x)
		,Engine::Math::PackFloat<ParticleFloatType>(y)
		,Engine::Math::PackFloat<ParticleFloatType>(z)
	};
}
glm::vec3 Engine::priv::ParticleContainer::getPosition(ParticleID id) const noexcept {
	const auto& item = getDataMember<0>(id);
	return glm::vec3{
		Engine::Math::ToFloat<ParticleFloatType>(item.x)
		, Engine::Math::ToFloat<ParticleFloatType>(item.y)
		, Engine::Math::ToFloat<ParticleFloatType>(item.z)
	};
}
/*
glm::vec3 Engine::priv::ParticleContainer::getPositionFromIndex(size_t index) const noexcept {
	const auto& item = m_ParticlesData.get<0>(index);
	return glm::vec3{
		Engine::Math::ToFloat<ParticleFloatType>(item.x)
		, Engine::Math::ToFloat<ParticleFloatType>(item.y)
		, Engine::Math::ToFloat<ParticleFloatType>(item.z)
	};
}
*/
void Engine::priv::ParticleContainer::setColor(ParticleID id, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	auto& item = getDataMember<4>(id);
	item = Engine::Compression::pack4ColorsInto16Int(r, g, b, a);
}
void Engine::priv::ParticleContainer::setVelocity(ParticleID id, float vx, float vy, float vz) {
	auto& item = getDataMember<5>(id);
	item = glm::vec3{ vx, vy, vz };
}
void Engine::priv::ParticleContainer::setAngularVelocity(ParticleID id, float av) {
	auto& item = getDataMember<6>(id);
	item = av;
}
void Engine::priv::ParticleContainer::setTimer(ParticleID id, float time) {
	auto& timer = getDataMember<7>(id);
	timer = time;
}
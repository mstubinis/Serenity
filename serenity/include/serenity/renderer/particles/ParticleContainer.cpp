#include <serenity/renderer/particles/ParticleContainer.h>
#include <serenity/renderer/particles/ParticleEmissionProperties.h>
#include <serenity/scene/Camera.h>
#include <serenity/math/MathCompression.h>
#include <serenity/renderer/culling/Culling.h>
#include <serenity/renderer/culling/SphereIntersectTest.h>


#include <serenity/utils/BlockProfiler.h>
#include <boost/sort/spreadsort/float_sort.hpp>
#include <serenity/algorithms/EngineSort.h>


namespace {
	constexpr const float PLANE_RADIUS = 0.5f;

	void sort_permutation_std(auto& indices, auto& distances, const auto& particles, auto& cameraPosition, size_t size, auto&& compare, auto executionPolicy = std::execution::unseq) {
		indices.resize(size);
		distances.resize(size);
		std::iota(std::begin(indices), std::end(indices), 0);

		//700,000 - 1 mil nanos
		std::transform(std::execution::par_unseq, std::begin(particles), std::begin(particles) + size, std::begin(distances), [&cameraPosition](const auto& particle) {
			return glm::length2(Engine::Math::ToFloatVec3(particle.m_position) - cameraPosition);
		});

		std::sort(executionPolicy, std::begin(indices), std::end(indices), [&](std::size_t i, std::size_t j) {
			return compare(distances[i], distances[j]);
		});
	}
	void sort_permutation_engine(auto& indices, auto& distances, const auto& particles, auto& cameraPosition, size_t size, auto&& compare, auto executionPolicy = std::execution::unseq) {
		indices.resize(size);
		distances.resize(size);
		std::iota(std::begin(indices), std::end(indices), 0);

		//800,000 - 1.3 mil nanos
		Engine::priv::threading::addJobSplitVectored([&particles, &cameraPosition](auto& distance, size_t index, int32_t jobIndex) {
			distance = glm::length2(Engine::Math::ToFloatVec3(particles[index].m_position) - cameraPosition);
		}, distances, true, true, 0);

		engine_sort(executionPolicy, std::begin(indices), std::end(indices), [&](std::size_t i, std::size_t j) {
			return compare(distances[i], distances[j]);
		}, 512);
	}



	size_t partition_permutation(auto& indices, const auto& particles, size_t size, auto&& compare, auto executionPolicy = std::execution::unseq) {
		indices.resize(size);
		std::iota(std::begin(indices), std::end(indices), 0);
		auto it = std::partition(executionPolicy, std::begin(indices), std::end(indices), [&](std::size_t i) {
			return compare(particles[i]);
	    });
		return std::distance(std::begin(indices), it);
	}
	void apply_permutation_in_place(auto& auxBooleans, auto& data, const std::vector<std::size_t>& partitions) {
		auxBooleans.resize(partitions.size(), false);
		for (std::size_t i = 0; i < partitions.size(); ++i) {
			if (auxBooleans[i]) {
				continue;
			}
			auxBooleans[i] = true;
			std::size_t prev_j = i;
			std::size_t j = partitions[i];
			while (i != j) {
				std::swap(data[prev_j], data[j]);
				auxBooleans[j] = true;
				prev_j = j;
				j = partitions[j];
			}
		}
	}
}

Engine::priv::ParticleContainer::ParticleContainer(uint32_t maxParticles) {
	m_ParticlesData.reserve(maxParticles);
	m_MaterialIDs.reserve(maxParticles);

	const auto packedZero = Engine::Math::PackFloat<ParticleFloatType>(0.0f);
	const auto packedOne  = Engine::Math::PackFloat<ParticleFloatType>(1.0f);
	for (size_t particleID = 0; particleID < m_ParticlesData.capacity(); ++particleID) {
		m_ParticlesData.emplace_back(
			glm::vec<3, ParticleFloatType, glm::packed_highp>{packedZero, packedZero, packedZero}       //positions        [0]
		  , glm::vec<2, ParticleFloatType, glm::packed_highp>{packedOne, packedOne}                     //scaleXY          [1]
		  , packedZero                                                                                  //angle            [2]
		  , Engine::Compression::pack4ColorsInto16Int(255, 255, 255, 255) //4 bits per color channel    //packedColor      [4]
		  , glm::vec<3, ParticleFloatType, glm::packed_highp>{packedZero, packedZero, packedZero}       //velocity         [5]
		  , 0.0f                                                                                        //angularVelocity  [6]
		  , 0.0f                                                                                        //timer            [7]
	    );
		m_MaterialIDs.emplace_back(std::numeric_limits<ParticleMaterialIDType>().max());
	}
}

bool Engine::priv::ParticleContainer::addParticle(const ParticleEmissionProperties& props, ParticleEmissionPropertiesHandle propHandle, const glm_vec3& emitterPosition, const glm::quat& emitterRotation, float scaleX, float scaleY, ParticleIDType matID, uint8_t r, uint8_t g, uint8_t b, uint8_t a, const glm::vec3& velocity, float angularVelocity) {
	auto rotated_initial_velocity = Engine::Math::rotate_vec3(emitterRotation, velocity);
	return addParticle(propHandle, float(emitterPosition.x), float(emitterPosition.y), float(emitterPosition.z), scaleX, scaleY, matID, r, g, b, a, rotated_initial_velocity, angularVelocity);
}

bool Engine::priv::ParticleContainer::addParticle(ParticleEmissionPropertiesHandle propHandle, float x, float y, float z, float scaleX, float scaleY, ParticleIDType matID, uint8_t r, uint8_t g, uint8_t b, uint8_t a, const glm::vec3& velocity, float angularVelocity) {
	assert(m_ActiveEnd < m_ParticlesData.size());
	if (m_ActiveEnd >= m_ParticlesData.size()) {
		return false;
	}
	const auto packedZero      = Engine::Math::PackFloat(0.0f);
	const auto packedOne       = Engine::Math::PackFloat(1.0f);
	auto& particle             = m_ParticlesData[m_ActiveEnd];
	particle.m_propHandle      = propHandle;
	particle.m_position        = Engine::Math::PackFloatVec3(glm::vec3{x, y, z});
	particle.m_scale           = Engine::Math::PackFloatVec2(glm::vec2{ scaleX, scaleY });
	particle.m_packedColor     = Engine::Compression::pack4ColorsInto16Int(r, g, b, a);
	particle.m_velocity        = Engine::Math::PackFloatVec3(velocity);
	particle.m_angularVelocity = angularVelocity;
	particle.m_timer           = 0.01f;
	++m_ActiveEnd;
	return true;
}
bool Engine::priv::ParticleContainer::recycleParticle(size_t particleIndex) {
	if (m_ActiveEnd == 0 || particleIndex >= m_ActiveEnd) {
		return false;
	}
	assert(m_ActiveEnd > 0);
	m_ParticlesData[particleIndex].m_timer = 0.0f; //TODO: is this needed?

	std::swap(m_ParticlesData[particleIndex], m_ParticlesData[m_ActiveEnd - 1]);
	std::swap(m_MaterialIDs[particleIndex], m_MaterialIDs[m_ActiveEnd - 1]);

	--m_ActiveEnd;
	return true;
}

size_t Engine::priv::ParticleContainer::cull_particles(const Camera& camera) {
	if (!m_ParticlesData.empty()) {
		//Engine::block_profiler block(std::cout, "Culling: ");
		const glm::vec3 camPos = camera.getPosition();
		auto lambda = [&](const Engine::priv::ParticleContainer::Particle& particle) {
			const float radius     = PLANE_RADIUS * std::max(Engine::Math::ToFloat(particle.m_scale.x), Engine::Math::ToFloat(particle.m_scale.y));
			const glm::vec3& pos   = Engine::Math::ToFloatVec3(particle.m_position);
			const int sphereTest   = Engine::priv::Culling::sphereIntersectTest(pos, radius, camera);
			const float comparison = radius * 3100.0f;
			return particle.m_timer > 0.0f && (glm::distance2(pos, camPos) <= (comparison * comparison)) && sphereTest > 0;
		};
		//2.5 - 2.6 mil nanoseconds
		const auto distance = partition_permutation(m_CullingAuxVector, m_ParticlesData, size(), lambda, std::execution::par_unseq);
		apply_permutation_in_place(m_CullingAuxVector2, m_ParticlesData, m_CullingAuxVector);
		apply_permutation_in_place(m_CullingAuxVector2, m_MaterialIDs,   m_CullingAuxVector);
		return distance;	
	}
	return 0;
}


void Engine::priv::ParticleContainer::sort_particles(const Camera& camera, size_t numParticles) {
	sort_particles(camera.getPosition(), numParticles);
}
void Engine::priv::ParticleContainer::sort_particles(const glm::vec3& cameraPosition, size_t numParticles) {
	if (m_ActiveEnd == 0 || numParticles <= 1) {
		return;
	}

	{
		//Engine::block_profiler block(std::cout, "sort_permutation_std:    ");
		sort_permutation_std(m_CullingAuxVector, m_SortingDistances, m_ParticlesData, cameraPosition, numParticles, [](const auto lhs, const auto rhs) {
			return lhs > rhs;
	    }, std::execution::par_unseq);
	}
	{
		/*
		//Engine::block_profiler block(std::cout, "sort_permutation_engine: ");
		sort_permutation_engine(m_CullingAuxVector, m_SortingDistances, m_ParticlesData, cameraPosition, numParticles, [](const auto lhs, const auto rhs) {
			return lhs > rhs;
		}, std::execution::par_unseq);
		*/
	}
	apply_permutation_in_place(m_CullingAuxVector2, m_ParticlesData, m_CullingAuxVector);
	apply_permutation_in_place(m_CullingAuxVector2, m_MaterialIDs, m_CullingAuxVector);
}


void Engine::priv::ParticleContainer::associate_materials() {
	m_Bimap.clear();
	/*
for (const auto matID : m_MaterialIDs) {
	//its just pretty expensive in general...
	auto itr = m_Bimap.find_key(particle.m_Material);
	if (itr == m_Bimap.end()) {
		itr = m_Bimap.emplace(particle.m_Material, matID).first;
	}
	///////////////////////////////////////////
}
*/
}







/*

	float maxFloat = std::numeric_limits<float>().max();
	setPosition(maxFloat, maxFloat, maxFloat);

	m_Timer                       = 0.001f;
	m_Position                    = emitterPosition;

	m_EmitterSource               = &emitter;
	m_Material                    = &const_cast<Material&>(emitter.m_Properties->getParticleMaterialRandom());

	auto& emitterTransform        = *emitter.getComponent<ComponentTransform>();
	auto emitterScale             = emitterTransform.getScale();

	m_Velocity                    = emitter.m_Properties->m_InitialVelocityFunctor(*this) * emitterScale;
	auto rotated_initial_velocity = Engine::Math::rotate_vec3(emitter.getComponent<ComponentTransform>()->getRotation(), m_Velocity);
	if (emitter.m_Parent) {
		auto parentBody = emitter.m_Parent.getComponent<ComponentTransform>();
		if (parentBody) {
			//m_Velocity = glm::vec3(parentBody->getLinearVelocity());
		}
	}
	m_Velocity += rotated_initial_velocity;

	m_Scale = emitter.m_Properties->m_InitialScaleFunctor(*this) * Engine::Math::Max(emitterScale.x, emitterScale.y, emitterScale.z);
	m_AngularVelocity = emitter.m_Properties->m_InitialAngularVelocityFunctor(*this);
	m_Color = emitter.m_Properties->m_ColorFunctor(0.0f, *this);


*/
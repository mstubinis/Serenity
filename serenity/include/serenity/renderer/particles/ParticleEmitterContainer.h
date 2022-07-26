#pragma once
#ifndef ENGINE_PARTICLE_EMITTER_CONTAINER_H
#define ENGINE_PARTICLE_EMITTER_CONTAINER_H

class Scene;

#include <vector>

namespace Engine::priv {
	template<class T, class HANDLE>
	class ParticleEmitterContainer {
	    private:
			struct Impl {
				static void setCapacity(size_t capacity, auto& data, auto& mapping, auto& mappingBackwards) {
					size_t oldCapacity = data.size();
					data.reserve(capacity);
					mapping.reserve(capacity);
					mappingBackwards.reserve(capacity);
					for (size_t i = oldCapacity; i < capacity; ++i) {
						mappingBackwards.emplace_back(uint32_t(i));
						mapping.emplace_back(uint32_t(i));
					}
				}
			};
	    private:
			constexpr static const uint32_t NULL_IDX = std::numeric_limits<uint32_t>().max();
		private:
			std::vector<T>         m_Data;
			std::vector<uint32_t>  m_MappingBackwards;
			std::vector<uint32_t>  m_Mapping;
			size_t                 m_LastIndex         = 0;
		public:
			ParticleEmitterContainer() = default;
			ParticleEmitterContainer(size_t capacity) {
				Impl::setCapacity(capacity, m_Data, m_Mapping, m_MappingBackwards);
			}


			inline T& operator[](size_t idx) noexcept { return m_Data[idx]; }
			inline const T& operator[](size_t idx) const noexcept { return m_Data[idx]; }
			inline size_t size() const noexcept { return m_LastIndex; }

			template<class ... ARGS>
			HANDLE add(Scene& scene, ARGS&&... args) {
				if (m_Data.capacity() == m_LastIndex) {
					Impl::setCapacity(m_Data.size() == 0 ? 1 : m_Data.size() * 2, m_Data, m_Mapping, m_MappingBackwards);
				}
				const auto freeIdx = m_MappingBackwards[m_LastIndex];
				m_Mapping[freeIdx] = uint32_t(m_LastIndex);
				if (m_Data.size() == m_LastIndex) {
					m_Data.emplace_back(scene, std::forward<ARGS>(args)...);
				} else {
					m_Data[m_LastIndex] = T(scene, std::forward<ARGS>(args)...);
				}
				++m_LastIndex;
				return HANDLE{ freeIdx, scene };
			}

			bool deactivate(size_t removedIndex) {
				if (removedIndex > m_LastIndex) { //TODO: check >= as well...
					return false;
				}
				assert(m_LastIndex > 0);
				--m_LastIndex;
				m_Mapping[m_MappingBackwards[removedIndex]] = NULL_IDX;
				m_Mapping[m_MappingBackwards[m_LastIndex]] = uint32_t(removedIndex);
				m_MappingBackwards[m_LastIndex] = uint32_t(removedIndex);
				m_MappingBackwards[removedIndex] = uint32_t(m_LastIndex);
				std::swap(m_Data[removedIndex], m_Data[m_LastIndex]);
				return true;
			}
			/*
			bool destroy(HANDLE handle) {
				const auto id = handle.id();
				if (id < m_Mapping.size()) {
					const auto removedComponentID = m_Mapping[id];
					if (removedComponentID == NULL_IDX) {
						return false;
					}
					assert(m_LastIndex > 0);
					const auto lastIndex = m_Data.size() - 1;
					m_Mapping[m_MappingBackwards[lastIndex]] = removedComponentID;
					m_MappingBackwards[lastIndex] = removedComponentID;
					m_MappingBackwards[removedComponentID] = uint32_t(lastIndex);
					m_Mapping[id] = NULL_IDX;
					m_Data[removedComponentID] = std::move(m_Data[lastIndex]);

					m_Data.back().destroy();
					m_Data.pop_back();

					--m_LastIndex;
					return true;
				}
				return false;
			}
			*/

			[[nodiscard]] Engine::view_ptr<T> get(HANDLE handle) noexcept {
				return m_Mapping[handle.id()] == NULL_IDX ? nullptr : &m_Data[m_Mapping[handle.id()]];
			}

			[[nodiscard]] inline typename std::vector<T>::iterator begin() noexcept { return m_Data.begin(); }
			[[nodiscard]] inline typename std::vector<T>::const_iterator begin() const noexcept { return m_Data.begin(); }
			[[nodiscard]] inline typename std::vector<T>::iterator end() noexcept { return m_Data.begin() + m_LastIndex; }
			[[nodiscard]] inline typename std::vector<T>::const_iterator end() const noexcept { return m_Data.begin() + m_LastIndex; }
			[[nodiscard]] inline typename const std::vector<T>::const_iterator cbegin() const noexcept { return m_Data.cbegin(); }
			[[nodiscard]] inline typename const std::vector<T>::const_iterator cend() const noexcept { return m_Data.cbegin() + m_LastIndex; }
	};

}

#endif
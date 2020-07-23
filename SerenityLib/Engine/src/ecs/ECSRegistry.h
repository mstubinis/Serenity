#pragma once
#ifndef ENGINE_ECS_REGISTRY_H
#define ENGINE_ECS_REGISTRY_H

#include <boost/type_index.hpp>
#include <boost/unordered_map.hpp>

using boost_type_index = boost::typeindex::type_index;

template <typename T> constexpr boost_type_index type_ID() noexcept { return boost::typeindex::type_id<T>();  }
template <typename T> constexpr boost_type_index type_ID(T* inType) noexcept { return boost::typeindex::type_id_runtime(*inType); }

namespace Engine::priv {
    class ECSRegistry final : public Engine::NonCopyable, public Engine::NonMoveable {
        private:
            static std::uint32_t                                          m_LastIndex;
            static boost::unordered_map<boost_type_index, std::uint32_t>  m_SlotMap;
        public:
            ECSRegistry() = default;
            ~ECSRegistry() = default;

            template <typename T> static CONSTEXPR std::uint32_t type_slot() noexcept {
                auto type = type_ID<T>();
                if (!m_SlotMap.count(type)) {
                    m_SlotMap.emplace(type, m_LastIndex);
                    ++m_LastIndex;
                }
                return m_SlotMap.at(type);
            }
            template <typename T> static CONSTEXPR std::uint32_t type_slot(T* t) noexcept {
                auto type = type_ID(t); 
                if (!m_SlotMap.count(type)) {
                    m_SlotMap.emplace(type, m_LastIndex);
                    ++m_LastIndex;
                }
                return m_SlotMap.at(type);
            }
			template <typename T> static CONSTEXPR std::uint32_t type_slot_fast() noexcept {
				return m_SlotMap.at(type_ID<T>());
			}
			template <typename T> static CONSTEXPR std::uint32_t type_slot_fast(T* t) noexcept {
				return m_SlotMap.at(type_ID(t));
			}
        };
};

#endif
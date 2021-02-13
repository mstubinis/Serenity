#pragma once
#ifndef ENGINE_CONTAINERS_TYPE_REGISTRY_H
#define ENGINE_CONTAINERS_TYPE_REGISTRY_H

#include <unordered_map>
#include <typeindex>

namespace Engine {
    class type_registry final {
        private:
            mutable std::unordered_map<std::type_index, uint32_t>  m_SlotMap;

            template <typename T> inline constexpr std::type_index internal_type_ID() const noexcept { return std::type_index(typeid(T)); }
            template <typename T> inline constexpr std::type_index internal_type_ID(T* inType) const noexcept { return std::type_index(typeid(*inType)); }

            uint32_t internal_calculate_type_slot(std::type_index&& inType) const noexcept {
                if (!m_SlotMap.contains(inType)) {
                    m_SlotMap.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(inType), 
                        std::forward_as_tuple((uint32_t)m_SlotMap.size())
                    );
                }
                return m_SlotMap.at(inType);
            }
        public:
            template<typename T> inline size_t count() const noexcept { return m_SlotMap.count(internal_type_ID<T>()); }
            template<typename T> inline bool contains() const noexcept { return m_SlotMap.contains(internal_type_ID<T>()); }

            template<typename T> inline uint32_t at() const noexcept {  return m_SlotMap.at(internal_type_ID<T>()); }

            inline size_t count(const std::type_index& typeIndex) const noexcept { return m_SlotMap.contains(typeIndex); }
            inline uint32_t operator[](const std::type_index& typeIndex) { return m_SlotMap[typeIndex]; }
            inline uint32_t at(const std::type_index& typeIndex) const noexcept { return m_SlotMap.at(typeIndex); }

            template <typename T> inline constexpr uint32_t type_slot() const noexcept { return internal_calculate_type_slot(internal_type_ID<T>()); }
            template <typename T> inline constexpr uint32_t type_slot(T* t) const noexcept { return internal_calculate_type_slot(internal_type_ID(t)); }
            template <typename T> inline constexpr uint32_t type_slot_fast() const noexcept { return m_SlotMap.at(internal_type_ID<T>()); }
            template <typename T> inline constexpr uint32_t type_slot_fast(T* t) const noexcept { return m_SlotMap.at(internal_type_ID(t)); }
    };
};

#endif
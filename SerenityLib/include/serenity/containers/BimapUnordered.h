#pragma once
#ifndef ENGINE_CONTAINERS_BIMAP_UNORDERED_H
#define ENGINE_CONTAINERS_BIMAP_UNORDERED_H

#include <unordered_map>

namespace Engine {

	template<class KEY, class VALUE>
	class unordered_bimap {
		private:
			std::unordered_map<KEY, VALUE> m_KeyToValue;
			std::unordered_map<VALUE, KEY> m_ValueToKey;
		public:
			constexpr unordered_bimap() = default;

			void reserve(size_t newCapacity) {
				m_KeyToValue.reserve(newCapacity);
				m_ValueToKey.reserve(newCapacity);
			}
			void clear() {
				m_KeyToValue.clear();
				m_ValueToKey.clear();
			}
			[[nodiscard]] inline constexpr bool empty() const noexcept { return m_KeyToValue.empty(); }
			[[nodiscard]] inline constexpr size_t size() const noexcept { return m_KeyToValue.size(); }


			[[nodiscard]] inline constexpr bool contains_key(const KEY& key) const noexcept { return m_KeyToValue.contains(key); }
			[[nodiscard]] inline constexpr bool contains_value(const VALUE& value) const noexcept { return m_ValueToKey.contains(value); }

			void merge(unordered_bimap& other) {
				m_KeyToValue.merge(other.m_KeyToValue);
				m_ValueToKey.merge(other.m_ValueToKey);
			}

			bool emplace(const KEY& key, const VALUE& value) {
				if (!m_KeyToValue.contains(key)) {
					m_KeyToValue.emplace(key, value);
					m_ValueToKey.emplace(value, key);
					return true;
				}
				return false;
			}

			[[nodiscard]] inline VALUE& operator[](const KEY& key) noexcept { return m_KeyToValue[key]; }
			[[nodiscard]] inline const VALUE& operator[](const KEY& key) const noexcept { return m_KeyToValue[key]; }
			[[nodiscard]] inline KEY& operator[](const VALUE& value) noexcept { return m_ValueToKey[value]; }
			[[nodiscard]] inline const KEY& operator[](const VALUE& value) const noexcept { return m_ValueToKey[value]; }

			[[nodiscard]] inline VALUE& at(const KEY& key) noexcept { return m_KeyToValue.at(key); }
			[[nodiscard]] inline const VALUE& at(const KEY& key) const noexcept { return m_KeyToValue.at(key); }
			[[nodiscard]] inline KEY& at(const VALUE& value) noexcept { return m_ValueToKey.at(value); }
			[[nodiscard]] inline const KEY& at(const VALUE& value) const noexcept { return m_ValueToKey.at(value); }

			[[nodiscard]] inline std::unordered_map<KEY, VALUE>::iterator begin() { return m_KeyToValue.begin(); }
			[[nodiscard]] inline std::unordered_map<KEY, VALUE>::iterator end() { return m_KeyToValue.end(); }
			[[nodiscard]] inline std::unordered_map<VALUE, KEY>::iterator rbegin() { return m_ValueToKey.begin(); }
			[[nodiscard]] inline std::unordered_map<VALUE, KEY>::iterator rend() { return m_ValueToKey.end(); }
	};
}

#endif
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

			decltype(auto) emplace(const KEY& key, const VALUE& value) {
				if (!m_KeyToValue.contains(key)) {
					m_ValueToKey.emplace(value, key);
					return m_KeyToValue.emplace(key, value);
				}
				return std::make_pair(end(), false);
			}

			[[nodiscard]] inline decltype(auto) find(const KEY& key) noexcept { return find_key(key); }
			[[nodiscard]] inline decltype(auto) find_key(const KEY& key) noexcept { return m_KeyToValue.find(key); }
			[[nodiscard]] inline decltype(auto) find_value(const VALUE& value) noexcept { return m_ValueToKey.find(value); }

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
			[[nodiscard]] inline const std::unordered_map<KEY, VALUE>::const_iterator begin() const { return m_KeyToValue.begin(); }
			[[nodiscard]] inline const std::unordered_map<KEY, VALUE>::const_iterator end() const { return m_KeyToValue.end(); }
			[[nodiscard]] inline const std::unordered_map<KEY, VALUE>::const_iterator cbegin() const { return m_KeyToValue.begin(); }
			[[nodiscard]] inline const std::unordered_map<KEY, VALUE>::const_iterator cend() const { return m_KeyToValue.end(); }
			[[nodiscard]] inline std::unordered_map<VALUE, KEY>::iterator rbegin() { return m_ValueToKey.begin(); }
			[[nodiscard]] inline std::unordered_map<VALUE, KEY>::iterator rend() { return m_ValueToKey.end(); }
			[[nodiscard]] inline const std::unordered_map<VALUE, KEY>::const_iterator rbegin() const { return m_ValueToKey.begin(); }
			[[nodiscard]] inline const std::unordered_map<VALUE, KEY>::const_iterator rend() const { return m_ValueToKey.end(); }
			[[nodiscard]] inline const std::unordered_map<VALUE, KEY>::const_iterator rcbegin() const { return m_ValueToKey.begin(); }
			[[nodiscard]] inline const std::unordered_map<VALUE, KEY>::const_iterator rcend() const { return m_ValueToKey.end(); }
	};
}

#endif
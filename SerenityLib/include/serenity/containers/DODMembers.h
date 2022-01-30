#pragma once
#ifndef ENGINE_CONTAINERS_DOD_MEMBERS_H
#define ENGINE_CONTAINERS_DOD_MEMBERS_H

#include <tuple>
#include <utility>
#include <serenity/system/Macros.h>

namespace Engine {
	template<class ... TYPES>
	class DODMembers {
	    private:
			template<class T>
			struct ARRAY_WRAPPER {
				using VAL_TYPE = T;
				T* data = nullptr;

				~ARRAY_WRAPPER() {
					if (data) {
						free(data);
					}
				}

				inline constexpr char& operator[](size_t index) noexcept { return data[index]; }
				inline constexpr const char& operator[](size_t index) const noexcept { return data[index]; }

				constexpr void moveMemory(size_t oldCapacity, size_t newCapacity) {
					void* newMemoryDst = malloc(sizeof(T) * newCapacity);
					std::memmove(newMemoryDst, (void*)data, sizeof(T) * oldCapacity);
					free(data);
					data = reinterpret_cast<T*>(newMemoryDst);
				}
				constexpr void destructMemory(size_t size) {
					for (size_t i = 0; i < size; ++i) {
						T* object = &data[i];
						object->~T();
					}
				}
#if 0
				constexpr void construct(size_t location, T&& arg) {
					if constexpr (std::is_placeholder_v<T>) {
						new (data + location) T{};
			        } else {
						new (data + location) T(std::forward<T>(args)...);
					}
				}
#endif
				template<class ... ARGS>
				constexpr void constructEmplace(size_t location, ARGS&&... args) {
					if constexpr (std::is_placeholder_v<ARGS...>) {
						new (data + location) T{};
					} else {
						new (data + location) T(std::forward<ARGS>(args)...);
					}
				}

			};
		public:
			using TUPLE_TYPE = std::tuple<ARRAY_WRAPPER<TYPES>...>;
		private:
			TUPLE_TYPE  m_Data;
			size_t      m_Size = 0;
			size_t      m_Capacity = 0;

			template<class CRTP>
			class IteratorBase {
				public:
					using counter_type      = std::make_signed_t<size_t>;
					using difference_type   = std::ptrdiff_t;
					using iterator_category = std::random_access_iterator_tag;
				protected:
					counter_type   m_I = counter_type(0);
				public:
					IteratorBase() = default;
					IteratorBase(counter_type inI)
						: m_I{ inI }
					{}

					inline bool operator<(IteratorBase const& r)  const { return m_I < r.m_I; }
					inline bool operator<=(IteratorBase const& r) const { return m_I <= r.m_I; }
					inline bool operator>(IteratorBase const& r)  const { return m_I > r.m_I; }
					inline bool operator>=(IteratorBase const& r) const { return m_I >= r.m_I; }
					inline bool operator!=(const IteratorBase& r) const { return m_I != r.m_I; }
					inline bool operator==(const IteratorBase& r) const { return m_I == r.m_I; }

					CRTP& operator++() { ++m_I; return static_cast<CRTP&>(*this); }
					CRTP& operator--() { --m_I; return static_cast<CRTP&>(*this); }
					CRTP  operator++(int) { CRTP r(*this); ++m_I; return r; }
					CRTP  operator--(int) { CRTP r(*this); --m_I; return r; }
					CRTP& operator+=(counter_type n) { m_I += n; return static_cast<CRTP&>(*this); }
					CRTP& operator-=(counter_type n) { m_I -= n; return static_cast<CRTP&>(*this); }
					CRTP operator+(counter_type n) const { CRTP r(*this); return r += n; }
					CRTP operator-(counter_type n) const { CRTP r(*this); return r -= n; }

					difference_type operator-(const CRTP& r) const { return m_I - r.m_I; }
			};
		public:


			template<size_t ... INDICES>
			class Iterator : public IteratorBase<Iterator<INDICES...>> {
				public:
					using IteratorBaseType = IteratorBase<Iterator<INDICES...>>;
					using tuple_type       = std::tuple<typename std::tuple_element<INDICES, TUPLE_TYPE>::type::VAL_TYPE*...>;
				private:
					Engine::DODMembers<TYPES...>* m_Vector = nullptr;

					template<size_t I> inline void fillImpl(tuple_type& t, IteratorBaseType::counter_type offset) {
						using TYPE      = typename std::tuple_element<I, TUPLE_TYPE>::type::VAL_TYPE;
						auto& arrayData = std::get<I>(m_Vector->data()).data;
						TYPE* object    = &arrayData[IteratorBaseType::m_I + offset];
						std::get<I>(t)  = object;
					}
					auto fill(IteratorBaseType::counter_type offset) {
						tuple_type res;
						((void)fillImpl<INDICES>(res, offset), ...);
						return res;
					}
				public:

					Iterator() = default;
					Iterator(Engine::DODMembers<TYPES...>* vector, IteratorBaseType::counter_type inI)
						: IteratorBaseType{ inI }
						, m_Vector(vector)
					{}

					inline auto operator*() { return fill(0); }
					inline const auto operator*() const { return fill(0); }
					inline auto operator->() { return fill(0); }
					inline const auto operator->() const { return fill(0); }
					inline auto operator[](IteratorBaseType::counter_type m) { return fill(m); }
					inline const auto operator[](IteratorBaseType::counter_type m) const { return fill(m); }
			};


			template<size_t INDEX>
			class Iterator<INDEX> : public IteratorBase<Iterator<INDEX>> {
				public:
					using IteratorBaseType = IteratorBase<Iterator<INDEX>>;
				private:
					Engine::DODMembers<TYPES...>* m_Vector = nullptr;

					auto& fill(IteratorBaseType::counter_type offset) {
						using TYPE      = typename std::tuple_element<INDEX, TUPLE_TYPE>::type::VAL_TYPE;
						auto& arrayData = std::get<INDEX>(m_Vector->data()).data;
						TYPE* object    = &arrayData[IteratorBaseType::m_I + offset];
						return *object;
					}
				public:

					Iterator() = default;
					Iterator(Engine::DODMembers<TYPES...>* vector, IteratorBaseType::counter_type inI)
						: IteratorBaseType{ inI }
						, m_Vector(vector)
					{}
					inline auto& operator*() { return fill(0); }
					inline const auto& operator*() const { return fill(0); }
					inline auto& operator->() { return fill(0); }
					inline const auto& operator->() const { return fill(0); }
					inline auto& operator[](IteratorBaseType::counter_type m) { return fill(m); }
					inline const auto& operator[](IteratorBaseType::counter_type m) const { return fill(m); }
				};


		public:


			constexpr DODMembers() = default;

			~DODMembers() {
				clear();
			}

			constexpr void clear() {
				std::apply([this](auto&&... args) { (args.destructMemory(m_Size), ...); }, m_Data);
				m_Size = 0;
			}

			constexpr void reserve(size_t newCapacity) {
				if (newCapacity > m_Capacity) {
					std::apply([this, newCapacity](auto&&... args) {( args.moveMemory(m_Capacity, newCapacity), ...); }, m_Data);
					m_Capacity = newCapacity;
				}
			}
#if 0
			constexpr size_t push_back(TYPES&&... args) {
				if (m_Size == m_Capacity) {
					reserve(m_Size == 0 ? 1 : m_Size * 2);
				}
				auto lambda = [this, ... args = std::forward<TYPES>(args)](auto&&... data) mutable {
					(data.construct(m_Size, std::forward<std::decay_t<TYPES>>(args)), ...);
				};
				std::apply(std::move(lambda), m_Data);
				m_Size++;
				return m_Size;
			}
#endif
			template<class ... ARGS>
			constexpr size_t emplace_back(ARGS&&... args) {
				if (m_Size == m_Capacity) {
					reserve(m_Size == 0 ? 1 : m_Size * 2);
				}
				auto lambda = [this, ... args = std::forward<ARGS>(args)](auto&&... data) mutable {
					(data.constructEmplace(m_Size, std::forward<std::decay_t<ARGS>>(args)), ...);
				};
				std::apply(std::move(lambda), m_Data);
				m_Size++;
				return m_Size;
			}


			template<size_t MEMBER_INDEX>
			inline constexpr auto& get(size_t index) noexcept {
				return std::get<MEMBER_INDEX>(m_Data).data[index];
			}


			constexpr void shrink_to_fit() {
				if (m_Capacity > m_Size) {
					std::apply([this](auto&&... args) {( args.moveMemory(m_Size, m_Size), ...); }, m_Data);
					m_Capacity = m_Size;
				}
			}

			template<size_t MEMBER_INDEX> [[nodiscard]] inline constexpr auto& front() noexcept { return get<MEMBER_INDEX>(0); }
			template<size_t MEMBER_INDEX> [[nodiscard]] inline constexpr auto& back() noexcept { return get<MEMBER_INDEX>(m_Size - 1); }
			template<size_t MEMBER_INDEX> [[nodiscard]] inline constexpr const auto& front() const noexcept { return get<MEMBER_INDEX>(0); }
			template<size_t MEMBER_INDEX> [[nodiscard]] inline constexpr const auto& back() const noexcept { return get<MEMBER_INDEX>(m_Size - 1); }
			[[nodiscard]] inline constexpr size_t size() const noexcept { return m_Size; }
			[[nodiscard]] inline constexpr size_t capacity() const noexcept { return m_Capacity; }
			[[nodiscard]] inline constexpr bool empty() const noexcept { return size() == 0; }
			[[nodiscard]] inline constexpr TUPLE_TYPE& data() noexcept { return m_Data; }
			[[nodiscard]] inline constexpr const TUPLE_TYPE& data() const noexcept { return m_Data; }

			template<size_t ... DATA_INDEX_TYPES>
			[[nodiscard]] inline constexpr Iterator<DATA_INDEX_TYPES...> begin() {
				return Iterator<DATA_INDEX_TYPES...>(this, 0);
			}
			template<size_t ... DATA_INDEX_TYPES>
			[[nodiscard]] inline constexpr Iterator<DATA_INDEX_TYPES...> end() {
				return Iterator<DATA_INDEX_TYPES...>(this, size());
			}

			template<size_t INDEX>
			void swap(size_t index1, size_t index2) {
				using TYPE      = typename std::tuple_element<INDEX, TUPLE_TYPE>::type::VAL_TYPE;
				auto& arrayData = std::get<INDEX>(m_Data).data;
				assert(index1 >= 0 && index1 < size());
				assert(index2 >= 0 && index2 < size());
				char tempBuffer[sizeof(TYPE)];
				std::memmove(&tempBuffer[0], &arrayData[index1], sizeof(TYPE));
				std::memmove(&arrayData[index1], &arrayData[index2], sizeof(TYPE));
				std::memmove(&arrayData[index2], &tempBuffer[0], sizeof(TYPE));
			}

	};
}

#endif
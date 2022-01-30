#pragma once
#ifndef ENGINE_CONTAINERS_DOD_MEMBERS_H
#define ENGINE_CONTAINERS_DOD_MEMBERS_H

#include <tuple>
#include <utility>
#include <serenity/system/Macros.h>

namespace Engine::priv {
	struct no_args_t {};

	namespace detail {
		template<std::size_t I = 0, class TUPLE_TYPE, class ... DATA_TYPES>
		constexpr void insertion_sort_assignment_part_1(std::tuple<DATA_TYPES...>& data, auto& key, const auto i) {
			if constexpr (I < sizeof...(DATA_TYPES)) {
				using TYPE = typename std::tuple_element<I, TUPLE_TYPE>::type::VAL_TYPE;
				std::memmove(&std::get<I>(key)[0], &std::get<I>(data)[i], sizeof(TYPE));
				insertion_sort_assignment_part_1<I + 1, TUPLE_TYPE, DATA_TYPES...>(data, key, i);
			}
		}

		template<std::size_t I = 0, class TUPLE_TYPE, class ... DATA_TYPES>
		constexpr void insertion_sort_assignment_part_2(std::tuple<DATA_TYPES...>& data, auto& key, const auto i) {
			if constexpr(I < sizeof...(DATA_TYPES)) {
				using TYPE = typename std::tuple_element<I, TUPLE_TYPE>::type::VAL_TYPE;
				std::memmove(&std::get<I>(data)[i], &std::get<I>(key)[0], sizeof(TYPE));
				insertion_sort_assignment_part_2<I + 1, TUPLE_TYPE, DATA_TYPES...>(data, key, i);
			}
		}
	}
}
namespace Engine {
	constexpr Engine::priv::no_args_t no_args{};
}


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

				inline constexpr T& operator[](size_t index) noexcept { return data[index]; }
				inline constexpr const T& operator[](size_t index) const noexcept { return data[index]; }

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
					if constexpr (std::is_placeholder_v<T> || std::is_same_v<ARGS..., Engine::priv::no_args_t>) {
						new (data + location) T{};
			        } else {
						new (data + location) T(std::forward<T>(args)...);
					}
				}
#endif
				template<class ... ARGS>
				constexpr void constructEmplace(size_t location, ARGS&&... args) {
					if constexpr (std::is_placeholder_v<ARGS...> || std::is_same_v<ARGS..., Engine::priv::no_args_t>) {
						new (data + location) T{};
					} else {
						new (data + location) T(std::forward<ARGS>(args)...);
					}
				}

				void swap(size_t index1, size_t index2) {
					char tempBuffer[sizeof(T)];
					std::memmove(&tempBuffer[0], &data[index1], sizeof(T));
					std::memmove(&data[index1], &data[index2], sizeof(T));
					std::memmove(&data[index2], &tempBuffer[0], sizeof(T));
				}
				void move(size_t indexSrc, size_t indexDest) {
					std::memmove(&data[indexDest], &data[indexSrc], sizeof(T));
				}
				void move(void* srcData, size_t indexDest) {
					std::memmove(&data[indexDest], srcData, sizeof(T));
				}
			};
		public:
			using TUPLE_TYPE        = std::tuple<ARRAY_WRAPPER<TYPES>...>;
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
					template<size_t I>
					void printImpl(auto& stream) const {
						if constexpr (I < sizeof...(INDICES) - 1) {
							stream << std::get<I>(m_Vector->data()).data[IteratorBaseType::m_I] << ", ";
						} else {
							stream << std::get<I>(m_Vector->data()).data[IteratorBaseType::m_I];
						}
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

					template<class STREAM>
					void print(STREAM& stream) const {
						((printImpl<INDICES>(stream)), ...);
						stream << '\n';
					}
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

					template<class STREAM> inline void print(STREAM& stream) const { stream << std::get<INDEX>(m_Vector->data()).data[IteratorBaseType::m_I] << '\n'; }
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
				auto& arrayData = std::get<INDEX>(m_Data);
				assert(index1 >= 0 && index1 < size());
				assert(index2 >= 0 && index2 < size());
				arrayData.swap(index1, index2);
			}

#if 0
			template<size_t INDEX>
			constexpr void sort_bubble(const auto sorter, size_t firstIndex, size_t lastIndex) {
				bool swapped = false;
				auto& arrayData = std::get<INDEX>(m_Data).data;
				assert(firstIndex >= 0 && firstIndex < size());
				assert(lastIndex >= 0 && lastIndex < size());
				assert(lastIndex > firstIndex);
				size_t n = lastIndex - firstIndex;
				for (size_t i = 0; i < n; i++) {
					for (size_t j = firstIndex + 1; j < (lastIndex + 1) - i; j++) {
						if (!sorter(arrayData[j - 1], arrayData[j])) {
							std::apply([this, j](auto&&... args) {( args.swap(j - 1, j), ...); }, m_Data);
							swapped = true;
						}
					}
					if (!swapped) { break; }
				}
			}
			template<size_t INDEX> inline constexpr void sort_bubble(size_t firstIndex, size_t lastIndex) { 
				assert(firstIndex >= 0 && firstIndex < size());
				assert(lastIndex >= 0 && lastIndex < size());
				if (lastIndex <= firstIndex) { return; }
				sort_bubble<INDEX>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, firstIndex, lastIndex);
			}
			template<size_t INDEX> inline constexpr void sort_bubble() { 
				if (size() <= 1) { return; }
				sort_bubble<INDEX>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, 0, size() - 1);
			}
#endif

			template<size_t INDEX>
			void sort_insertion(const auto sorter, size_t firstIndex, size_t lastIndex) {
				auto& arrayData = std::get<INDEX>(m_Data);
				using ssize_t = std::make_signed_t<std::size_t>;
				ssize_t i = firstIndex + 1;
				TUPLE_TYPE key;

				std::apply([this](auto&&... args) { (args.moveMemory(0, 1), ...); }, key);
				while (i <= lastIndex) {
					Engine::priv::detail::insertion_sort_assignment_part_1<0, TUPLE_TYPE>(m_Data, key, i);
					ssize_t j = i - 1;
					while (j >= 0 && !sorter(arrayData[j], std::get<INDEX>(key)[0])) {
						std::apply([this, j](auto&&... args) {(args.move(j, j + 1), ...); }, m_Data);
						j -= 1;
					}
					Engine::priv::detail::insertion_sort_assignment_part_2<0, TUPLE_TYPE>(m_Data, key, j + 1);
					i += 1;
				}
			}

			template<size_t INDEX>
			void sort_insertion(size_t firstIndex, size_t lastIndex) {
				assert(firstIndex >= 0 && firstIndex < size());
				assert(lastIndex >= 0 && lastIndex < size());
				if (lastIndex <= firstIndex) { return; }
				sort_insertion<INDEX>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, firstIndex, lastIndex);
			}
			template<size_t INDEX>
			void sort_insertion() {
				if (size() <= 1) { return; }
				sort_insertion<INDEX>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, 0, size() - 1);
			}

			template<size_t INDEX>
			void sort_quick(const auto sorter, size_t firstIndex, size_t lastIndex) {
				assert(firstIndex >= 0 && firstIndex < size());
				assert(lastIndex >= 0 && lastIndex < size());
				if (lastIndex - firstIndex <= 1) {
					return;
				}
				auto median_of_three = [this](const auto& sorter, auto& data, int left, int right) {
					int mid = left + (right - left) / 2;
					if (!sorter(data[left], data[mid])) {
						std::apply([this, left, mid](auto&&... args) {(args.swap(left, mid), ...); }, m_Data);
					}
					if (!sorter(data[left], data[right])) {
						std::apply([this, left, right](auto&&... args) {(args.swap(left, right), ...); }, m_Data);
					}
					if (!sorter(data[mid], data[right])) {
						std::apply([this, mid, right](auto&&... args) {(args.swap(mid, right), ...); }, m_Data);
					}
					return data[mid];
				};
				auto partition_hoare_duplicates = [this, &median_of_three](const auto& sorter, auto& data, int left, int right) -> std::pair<int, int> {
					auto pivot = median_of_three(sorter, data, left, right);
					int i = left - 1;
					int j = right + 1;
					for (;;) {
						while (sorter(data[++i], pivot));
						while (sorter(pivot, data[--j]));
						if (i >= j) {
							break;
						}
						std::apply([this, i, j](auto&&... args) {(args.swap(i, j), ...); }, m_Data);
					}
					// exclude middle values == pivot
					i = j;
					j++;
					while (i > left && data[i] == pivot) i--;
					while (j < right && data[j] == pivot) j++;
					return { i, j };
				};
				auto quicksort = [&partition_hoare_duplicates](const auto& sorter, auto& data, int left, int right, const auto& quicksort_impl) -> void {
					if (left < right) {
						auto indices = partition_hoare_duplicates(sorter, data, left, right);
						quicksort_impl(sorter, data, left, indices.first,   quicksort_impl);
						quicksort_impl(sorter, data, indices.second, right, quicksort_impl);
					}
				};
				auto& arrayData = std::get<INDEX>(m_Data);
				quicksort(sorter, arrayData, int(firstIndex), int(lastIndex), quicksort);
			}
			template<size_t INDEX> inline constexpr void sort_quick(size_t firstIndex, size_t lastIndex) { 
				assert(firstIndex >= 0 && firstIndex < size());
				assert(lastIndex >= 0 && lastIndex < size());
				if (lastIndex <= firstIndex) { return; }
				sort_quick<INDEX>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, firstIndex, lastIndex); 
			}
			template<size_t INDEX> inline constexpr void sort_quick() {
				if (size() <= 1) { return; }
				sort_quick<INDEX>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, 0, size() - 1);
			}

			template<size_t INDEX>
			constexpr void sort(const auto sorter, size_t firstIndex, size_t lastIndex) {
				assert(firstIndex >= 0 && firstIndex < size());
				assert(lastIndex >= 0 && lastIndex < size());
				if (lastIndex <= firstIndex) { return; }
				if (lastIndex - firstIndex <= 32) {
					sort_insertion<INDEX>(sorter, firstIndex, lastIndex);
					return;
				}
				sort_quick<INDEX>(sorter, firstIndex, lastIndex);
			}
			template<size_t INDEX> inline constexpr void sort(size_t firstIndex, size_t lastIndex) {
				assert(firstIndex >= 0 && firstIndex < size());
				assert(lastIndex >= 0 && lastIndex < size());
				if (lastIndex <= firstIndex) { return; }
				sort<INDEX>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, firstIndex, lastIndex);
			}
			template<size_t INDEX> inline constexpr void sort() {
				if (size() <= 1) { return; }
				sort<INDEX>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, 0, size() - 1);
			}
	};
}

#endif
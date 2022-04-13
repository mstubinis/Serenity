#pragma once
#ifndef ENGINE_CONTAINERS_DOD_MEMBERS_H
#define ENGINE_CONTAINERS_DOD_MEMBERS_H

#include <tuple>
#include <utility>
#include <serenity/system/Macros.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/system/Engine.h>
#include <execution>

namespace Engine::priv {
	template<class T> struct no_args_t {};

	namespace detail {
		using ssize_t = std::make_signed_t<std::size_t>;


		template<std::size_t I = 0, class TUPLE_TYPE, class ... DATA_TYPES>
		constexpr void insertion_sort_assignment_part_1(std::tuple<DATA_TYPES...>& data, auto& key, const auto index) {
			if constexpr (I < sizeof...(DATA_TYPES)) {
				using TYPE = typename std::tuple_element<I, TUPLE_TYPE>::type::VAL_TYPE;
				std::memcpy(&std::get<I>(key)[0], &std::get<I>(data)[index], sizeof(TYPE));
				insertion_sort_assignment_part_1<I + 1, TUPLE_TYPE, DATA_TYPES...>(data, key, index);
			}
		}
		template<std::size_t I = 0, class TUPLE_TYPE, class ... DATA_TYPES>
		constexpr void insertion_sort_assignment_part_2(std::tuple<DATA_TYPES...>& data, auto& key, const auto index) {
			if constexpr(I < sizeof...(DATA_TYPES)) {
				using TYPE = typename std::tuple_element<I, TUPLE_TYPE>::type::VAL_TYPE;
				std::memcpy(&std::get<I>(data)[index], &std::get<I>(key)[0], sizeof(TYPE));
				insertion_sort_assignment_part_2<I + 1, TUPLE_TYPE, DATA_TYPES...>(data, key, index);
			}
		}

		auto quicksort_median_of_three(auto& arrays, const auto& sorter, auto& data, auto left, auto right) {
			const auto mid = left + (right - left) / 2;
			if (!sorter(data[left], data[mid])) {
				std::apply([&arrays, left, mid](auto&&... args) {(args.swap(left, mid), ...); }, arrays);
			}
			if (!sorter(data[left], data[right])) {
				std::apply([&arrays, left, right](auto&&... args) {(args.swap(left, right), ...); }, arrays);
			}
			if (!sorter(data[mid], data[right])) {
				std::apply([&arrays, mid, right](auto&&... args) {(args.swap(mid, right), ...); }, arrays);
			}
			return data[mid];
		}
		std::pair<ssize_t, ssize_t> quicksort_partition_hoare_duplicates(auto& arrays, const auto& sorter, auto& data, ssize_t left, ssize_t right) {
			const auto pivot = quicksort_median_of_three(arrays, sorter, data, left, right);
			ssize_t i = left - 1;
			ssize_t j = right + 1;
			for (;;) {
				while (sorter(data[++i], pivot));
				while (sorter(pivot, data[--j]));
				if (i >= j) {
					break;
				}
				std::apply([&arrays, i, j](auto&&... args) {(args.swap(i, j), ...); }, arrays);
			}
			// exclude middle values == pivot
			i = j;
			j++;
			while (i > left && data[i] == pivot) i--;
			while (j < right && data[j] == pivot) j++;
			return { i, j };
		}
		void quicksort_parallel_impl(auto& arrays, auto& data, ssize_t left, ssize_t right, auto sorter, size_t parallelThreshold) {
			const auto pivot = quicksort_partition_hoare_duplicates(arrays, sorter, data, left, right);
			auto job1 = [&arrays, &data, left, pivot, sorter, parallelThreshold]() {
				quicksort_parallel_impl(arrays, data, left, pivot.first, sorter, parallelThreshold);
			};
			auto job2 = [&arrays, &data, right, pivot, sorter, parallelThreshold]() {
				quicksort_parallel_impl(arrays, data, pivot.second, right, sorter, parallelThreshold);
			};
			if (left < pivot.first) {
				if (pivot.first - left > ssize_t(parallelThreshold)) {
					Engine::priv::threading::addJob(job1);
				} else {
					job1();
				}
			}
			if (pivot.second < right) {
				if (right - pivot.second > ssize_t(parallelThreshold)) {
					Engine::priv::threading::addJob(job2);
				} else {
					job2();
				}
			}
		}
		void quicksort_parallel(auto& arrays, auto& data, size_t left, size_t right, auto sorter, size_t parallelThreshold) {
			quicksort_parallel_impl(arrays, data, ssize_t(left), ssize_t(right), sorter, parallelThreshold);
			Engine::priv::threading::waitForAll();
		}
	}
}
namespace Engine {
	constexpr Engine::priv::no_args_t<void> no_args{};
}


namespace Engine {
	template<class ... TYPES>
	class DODMembers {
	    private:
			template<class T>
			struct ARRAY_WRAPPER {
				using VAL_TYPE = T;
				T* data = nullptr;

				ARRAY_WRAPPER() = default;
				ARRAY_WRAPPER(const ARRAY_WRAPPER&) = delete;
				ARRAY_WRAPPER& operator=(const ARRAY_WRAPPER&) = delete;
				ARRAY_WRAPPER(ARRAY_WRAPPER&&) noexcept = delete;
				ARRAY_WRAPPER& operator=(ARRAY_WRAPPER&&) noexcept = delete;

				~ARRAY_WRAPPER() {
					if (data) {
						free(data);
					}
				}

				inline constexpr T& operator[](size_t index) noexcept { return data[index]; }
				inline constexpr const T& operator[](size_t index) const noexcept { return data[index]; }

				constexpr void moveMemory(size_t oldCapacity, size_t newCapacity, size_t oldSize) {
					if (oldCapacity != newCapacity) {
						const auto oldCapInBytes = sizeof(T) * oldCapacity;
						const auto newCapInBytes = sizeof(T) * newCapacity;
						void* newMemoryDst       = malloc(newCapInBytes);
						if (newCapInBytes > oldCapInBytes) {
							std::memcpy(newMemoryDst, (void*)data, oldCapInBytes);
						} else {
							std::memcpy(newMemoryDst, (void*)data, newCapInBytes);
							if (oldSize > 0) {
								destructMemory(newCapacity, oldSize);
							}
						}
						free(data);
						data = reinterpret_cast<T*>(newMemoryDst);
					}
				}
				constexpr void destructMemory(size_t firstIndex, size_t lastIndexPlusOne) {
					if constexpr(!std::is_trivially_destructible_v<T>) {
						for (size_t i = firstIndex; i < lastIndexPlusOne; ++i) {
							T* object = &data[i];
							object->~T();
						}
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
					if constexpr (std::is_placeholder_v<ARGS...> || std::is_same_v<ARGS..., Engine::priv::no_args_t<T>> || std::is_same_v<ARGS..., Engine::priv::no_args_t<void>>) {
						new (data + location) T{};
					} else {
						new (data + location) T(std::forward<ARGS>(args)...);
					}
				}

				void swap(size_t index1, size_t index2) {
					if (index1 == index2) {
						return;
					}
					char tempBuffer[sizeof(T)];
					std::memcpy(&tempBuffer[0], &data[index1], sizeof(T));
					std::memcpy(&data[index1], &data[index2], sizeof(T));
					std::memcpy(&data[index2], &tempBuffer[0], sizeof(T));
				}
				void move(size_t indexSrc, size_t indexDest) {
					std::memcpy(&data[indexDest], &data[indexSrc], sizeof(T));
				}
				void move(void* srcData, size_t indexDest) {
					std::memcpy(&data[indexDest], srcData, sizeof(T));
				}
			};
		public:
			using TUPLE_TYPE        = std::tuple<ARRAY_WRAPPER<TYPES>...>;
			using TUPLE_TYPE_ONE    = std::tuple<TYPES...>;
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

			template<size_t INDEX>
			constexpr bool is_sorted() {
				auto& arrayData = std::get<INDEX>(m_Data);
				for (size_t i = 1; i < m_Size; ++i) {
					const auto& lhs = arrayData[i - 1];
					const auto& rhs = arrayData[i];
					if (lhs > rhs) {
						return false;
					}
				}
				return true;
			}

			constexpr void clear() {
				std::apply([this](auto&&... args) { (args.destructMemory(0, m_Size), ...); }, m_Data);
				m_Size = 0;
			}

			//Increase the capacity of the vector to a value that's greater or equal to newCapacity.
			//If newCapacity is greater than the current capacity(), new storage is allocated, otherwise the function does nothing.
			constexpr void reserve(size_t newCapacity) {
				if (newCapacity > m_Capacity) {
					std::apply([this, newCapacity](auto&&... args) {( args.moveMemory(m_Capacity, newCapacity, m_Size), ...); }, m_Data);
					m_Capacity = newCapacity;
				}
			}

			//Resizes the container to contain newSize elements.
			//If the current size is greater than newSize, the container is reduced to its first count elements.
			//	If the current size is less than newSize,
			//	    additional copies of value are appended.
			template<class ... ARGS>
			constexpr void resize(size_t newSize, ARGS&&... args) {
				if (newSize > m_Size) {
					//append more
					std::apply([this, newSize](auto&&... args) { (args.moveMemory(m_Capacity, newSize, m_Size), ...); }, m_Data);
					for (size_t i = m_Size; i < newSize; ++i) {
						auto lambda = [i, this, ... Args = args](auto&&... data) mutable {
							(data.constructEmplace(i, std::forward<std::decay_t<ARGS>>(Args)), ...);
						};
						std::apply(std::move(lambda), m_Data);
					}
					m_Size = newSize;
					m_Capacity = newSize;
				} else if (newSize < m_Size) {
					//reduce
					std::apply([this, newSize](auto&&... args) { (args.moveMemory(m_Capacity, newSize, m_Size), ...); }, m_Data);
					m_Size = newSize;
					m_Capacity = newSize;
				}
			}
			constexpr void resize(size_t newSize) {
				resize(newSize, Engine::priv::no_args_t<TYPES>()...);
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
			inline constexpr auto& getDataPtr() noexcept {
				return std::get<MEMBER_INDEX>(m_Data).data;
			}
			template<size_t MEMBER_INDEX>
			inline constexpr auto& get(size_t index) noexcept {
				return std::get<MEMBER_INDEX>(m_Data).data[index];
			}
			template<size_t MEMBER_INDEX>
			inline constexpr const auto& get(size_t index) const noexcept {
				return std::get<MEMBER_INDEX>(m_Data).data[index];
			}

			constexpr void shrink_to_fit() {
				if (m_Capacity > m_Size) {
					std::apply([this](auto&&... args) {( args.moveMemory(m_Size, m_Size, m_Size), ...); }, m_Data);
					m_Capacity = m_Size;
				}
			}
			constexpr void pop_back() {
				if (m_Size > 0) {
					std::apply([this](auto&&... args) { (args.destructMemory(m_Size - 1, m_Size), ...); }, m_Data);
					--m_Size;
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
				assert(index1 >= 0 && index1 < size());
				assert(index2 >= 0 && index2 < size());
				auto& arrayData = std::get<INDEX>(m_Data);
				arrayData.swap(index1, index2);
			}
			void swap(size_t index1, size_t index2) {
				assert(index1 >= 0 && index1 < size());
				assert(index2 >= 0 && index2 < size());
				std::apply([this, index1, index2](auto&&... args) {
					(args.swap(index1, index2), ...);
				}, m_Data);
			}

			template<size_t INDEX, class ExecPol = std::execution::sequenced_policy>
			void sort_insertion(const auto sorter, size_t firstIndex, size_t lastIndex, size_t parallelThreshold = 300) {
				auto& arrayData = std::get<INDEX>(m_Data);
				using ssize_t = std::make_signed_t<std::size_t>;
				ssize_t i = firstIndex + 1;
				TUPLE_TYPE key;

				std::apply([this](auto&&... args) { (args.moveMemory(0, 1, 0), ...); }, key); //build memory for one element per type in key
				while (i <= ssize_t(lastIndex)) {
					Engine::priv::detail::insertion_sort_assignment_part_1<0, TUPLE_TYPE>(m_Data, key, i); //key = data[i]
					ssize_t j = i - 1;
					while (j >= 0 && !sorter(arrayData[j], std::get<INDEX>(key)[0])) {
						std::apply([this, j](auto&&... args) {(args.move(j, j + 1), ...); }, m_Data);
						j -= 1;
					}
					Engine::priv::detail::insertion_sort_assignment_part_2<0, TUPLE_TYPE>(m_Data, key, j + 1); //data[j + 1] = key
					i += 1;
				}
			}

			template<size_t INDEX, class ExecPol = std::execution::sequenced_policy>
			void sort_insertion(size_t firstIndex, size_t lastIndex, size_t parallelThreshold = 300) {
				assert(firstIndex >= 0 && firstIndex < size());
				assert(lastIndex >= 0 && lastIndex < size());
				if (lastIndex <= firstIndex) { return; }
				sort_insertion<INDEX, ExecPol>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, firstIndex, lastIndex);
			}
			template<size_t INDEX, class ExecPol = std::execution::sequenced_policy>
			void sort_insertion(size_t parallelThreshold = 300) {
				if (size() <= 1) { return; }
				sort_insertion<INDEX, ExecPol>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, 0, size() - 1);
			}

			template<size_t INDEX, class ExecPol = std::execution::sequenced_policy>
			void sort_quick(const auto sorter, size_t left, size_t right, size_t parallelThreshold = 300) {
				using ssize_t = std::make_signed_t<std::size_t>;
				assert(left >= 0 && left < size());
				assert(right >= 0 && right < size());

				if (left < right) {
					if constexpr (std::is_same_v<ExecPol, std::execution::parallel_policy> || std::is_same_v<ExecPol, std::execution::parallel_unsequenced_policy>) {
						Engine::priv::detail::quicksort_parallel(m_Data, std::get<INDEX>(m_Data), left, right, sorter, parallelThreshold);
					} else {
						ssize_t top = -1;
						std::vector<std::pair<ssize_t, ssize_t>> stackOfIndices(ssize_t(right) - ssize_t(left) + 1, {0, 0});
						stackOfIndices[++top] = { left, right };
						while (top >= 0) {
							ssize_t L = stackOfIndices[top].first;
							ssize_t R = stackOfIndices[top].second;
							--top;
							if (L >= R) {
								continue;
							}
							const auto pivot = Engine::priv::detail::quicksort_partition_hoare_duplicates(m_Data, sorter, std::get<INDEX>(m_Data), L, R);
							stackOfIndices[++top] = { L, pivot.first };
							stackOfIndices[++top] = { pivot.second, R };
						}
					}
				}
			}
			template<size_t INDEX, class ExecPol = std::execution::sequenced_policy> inline constexpr void sort_quick(size_t firstIndex, size_t lastIndex, size_t parallelThreshold = 300) {
				assert(firstIndex >= 0 && firstIndex < size());
				assert(lastIndex >= 0 && lastIndex < size());
				if (lastIndex <= firstIndex) { return; }
				sort_quick<INDEX, ExecPol>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, firstIndex, lastIndex);
			}
			template<size_t INDEX, class ExecPol = std::execution::sequenced_policy> inline constexpr void sort_quick(size_t parallelThreshold = 300) {
				if (size() <= 1) { return; }
				sort_quick<INDEX, ExecPol>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, 0, size() - 1);
			}

			template<size_t INDEX, class ExecPol = std::execution::sequenced_policy>
			constexpr void sort(const auto sorter, size_t firstIndex, size_t lastIndex, size_t parallelThreshold = 300) {
				assert(firstIndex >= 0 && firstIndex < size());
				assert(lastIndex >= 0 && lastIndex < size());
				if (lastIndex <= firstIndex) { return; }
				if (lastIndex - firstIndex <= 32) {
					sort_insertion<INDEX, ExecPol>(sorter, firstIndex, lastIndex);
					return;
				}
				sort_quick<INDEX, ExecPol>(sorter, firstIndex, lastIndex);
			}
			template<size_t INDEX, class ExecPol = std::execution::sequenced_policy> inline constexpr void sort(size_t firstIndex, size_t lastIndex, size_t parallelThreshold = 300) {
				assert(firstIndex >= 0 && firstIndex < size());
				assert(lastIndex >= 0 && lastIndex < size());
				if (lastIndex <= firstIndex) { return; }
				sort<INDEX, ExecPol>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, firstIndex, lastIndex);
			}
			template<size_t INDEX, class ExecPol = std::execution::sequenced_policy> inline constexpr void sort(size_t parallelThreshold = 300) {
				if (size() <= 1) { return; }
				sort<INDEX, ExecPol>([](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; }, 0, size() - 1);
			}
	};
}

#endif
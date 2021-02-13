#pragma once
#ifndef ENGINE_TYPES_SMALL_MAP_H
#define ENGINE_TYPES_SMALL_MAP_H

#include <vector>

namespace Engine {

    /*
    An unordered key - value pair mapping container that uses arrays as the underlying structure.
    The container is meant to be used only for a small amount of items to take advantage of cache locality as the main factor for performance. 
    */
    template<typename KEY, typename VALUE, typename ALLOCATOR_KEY = std::allocator<KEY>, typename ALLOCATOR_VALUE = std::allocator<VALUE>>
    class map_small_unordered final {
        private:
            std::vector<KEY, ALLOCATOR_KEY>      m_Keys;
            std::vector<VALUE, ALLOCATOR_VALUE>  m_Values;
        public:
            map_small_unordered() = default;
            ~map_small_unordered() = default;

            void reserve(size_t newCapacity) noexcept {
                m_Keys.reserve(newCapacity);
                m_Values.reserve(newCapacity);
            }

            void shrink_to_fit() noexcept {
                m_Keys.shrink_to_fit();
                m_Values.shrink_to_fit();
            }

            //O(N)
            bool erase(const KEY& key) noexcept {
                for (size_t i = 0; i < m_Keys.size(); ++i) {
                    if (m_Keys[i] == key) {
                        m_Keys.erase(m_Keys.begin() + i);
                        m_Values.erase(m_Values.begin() + i);
                        return true;
                    }
                }
                return false;
            }

            //O(N)
            template<typename ... ARGS>
            bool emplace(const KEY& key, ARGS&&... args) noexcept {
                if (getIndexOfKey(key) == -1) {
                    m_Keys.emplace_back(key);
                    m_Values.emplace_back(std::forward<ARGS>(args)...);
                    return true;
                }
                return false;
            }

            //O(N)
            bool emplace(const KEY& key, const VALUE& value) noexcept {
                if (getIndexOfKey(key) == -1) {
                    m_Keys.emplace_back(key);
                    m_Values.emplace_back(value);
                    return true;
                }
                return false;
            }

            inline void clear() noexcept {
                m_Keys.clear();
                m_Values.clear();
            }

            //O(N)
            inline bool count(const KEY& key) const noexcept { return getIndexOfKey(key) != -1; }

            inline size_t size() const noexcept { return m_Keys.size(); }

            inline size_t capacity() const noexcept { return m_Keys.capacity(); }

            //O(N)
            int getIndexOfKey(const KEY& key) noexcept { 
                for (int i = 0; i < m_Keys.size(); ++i) { 
                    if (key == m_Keys[i]) {
                        return i;
                    }
                } 
                return -1; 
            }

            //O(N)
            const VALUE& operator[](const KEY& key) noexcept {
                const int keyIndex = getIndexOfKey(key);
                if (keyIndex != -1) {
                    return m_Values[keyIndex];
                }
                emplace(key, {});
            }



            //O(1)
            inline const KEY& getKeyFromIndex(size_t index) const noexcept { return m_Keys[index]; }

            //O(1)
            inline const VALUE& getValueFromIndex(size_t index) const noexcept { return m_Values[index]; }

            //O(1)
            inline KEY& getKeyFromIndex(size_t index) noexcept { return m_Keys[index]; }

            //O(1)
            inline VALUE& getValueFromIndex(size_t index) noexcept { return m_Values[index]; }

            //O(1)
            inline std::pair<KEY, VALUE> getDataFromIndex(size_t index) noexcept {
                return std::pair<KEY, VALUE>(
                    std::piecewise_construct, 
                    std::forward_as_tuple(getKeyFromIndex(index)), 
                    std::forward_as_tuple(getValueFromIndex(index))
                );
            }

            //O(N)
            inline const VALUE& at(const KEY& key) const noexcept {
                return m_Values[getIndexOfKey(key)];
            }
            //O(N)
            inline VALUE& at(const KEY& key) noexcept {
                return m_Values[getIndexOfKey(key)];
            }
    };




    /*
    An ordered key - value pair mapping container that uses arrays as the underlying structure.
    The container is meant to be used only for a small amount of items to take advantage of cache locality as the main factor for performance.
    */
    template<typename KEY, typename VALUE>
    class map_small final {
        private:
            std::vector<KEY>     m_Keys;
            std::vector<VALUE>   m_Values;

            int binary_search_for_index(const KEY& key) noexcept {
                int left = 0;
                int right = (int)m_Keys.size() - 1;
                while (left < right) {
                    int mid = left + (right - left) / 2;
                    if (m_Keys[mid] == key) {
                        return mid;
                    }else if (m_Keys[mid] < key) {
                        left = mid + 1;
                    }else {
                        right = mid - 1;
                    }
                }
                return -1;
            }
            size_t binary_search_for_index_lower_bound(const KEY& key) noexcept {
                size_t left  = 0;
                size_t right = m_Keys.size();
                while (left < right) {
                    const size_t mid = left + (right - left) / 2;
                    if (key <= m_Keys[mid]) {
                        right = mid;
                    }else{
                        left = mid + 1;
                    }
                }
                return left;
            }

            template<typename ...ARGS>
            void insert_into_vector(const KEY& key, ARGS&&... args) {
                const int index = binary_search_for_index_lower_bound(key);
                m_Keys.insert(m_Keys.begin() + index, key);
                m_Values.insert(m_Values.begin() + index, VALUE(std::forward<ARGS>(args)...));
            }
            void insert_into_vector(const KEY& key, const VALUE& value) {
                const int index = binary_search_for_index_lower_bound(key);
                m_Keys.insert(m_Keys.begin() + index, key);
                m_Values.insert(m_Values.begin() + index, value);
            }

        public:
            map_small() = default;
            ~map_small() = default;

            inline void reserve(size_t newCapacity) noexcept { 
                m_Keys.reserve(newCapacity);
                m_Values.reserve(newCapacity);
            }
            inline void shrink_to_fit() noexcept { 
                m_Keys.shrink_to_fit();
                m_Values.shrink_to_fit();
            }
            inline void clear() noexcept {
                m_Keys.clear();
                m_Values.clear();
            }

            bool erase(const KEY& key) noexcept {
                const int index = binary_search_for_index(key);
                if (index != -1) {
                    m_Keys.erase(m_Keys.begin() + index);
                    m_Values.erase(m_Values.begin() + index);
                    return true;
                }
                return false;
            }

            //O(og Nl + (possible n resize))
            template<typename ...ARGS>
            bool emplace(const KEY& key, ARGS&&... args) noexcept {
                if (getIndexOfKey(key) == -1) {
                    insert_into_vector(key, std::forward<ARGS>(args)...);
                    return true;
                }
                return false;
            }

            //O(log N + (possible n resize))
            bool emplace(const KEY& key, const VALUE& value) noexcept {
                if (getIndexOfKey(key) == -1) {
                    insert_into_vector(key, value);
                    return true;
                }
                return false;
            }

            //O(log N)
            inline bool count(const KEY& key) const noexcept { return getIndexOfKey(key) != -1; }

            inline size_t size() const noexcept { return m_Keys.size(); }

            inline size_t capacity() const noexcept { return m_Keys.capacity(); }

            //O(log N)
            inline int getIndexOfKey(const KEY& key) noexcept { return binary_search_for_index(key); }

            //O(log N)
            const VALUE& operator[](const KEY& key) noexcept {
                const int keyIndex = getIndexOfKey(key);
                if (keyIndex != -1) {
                    return m_Values[keyIndex];
                }
                insert_into_vector(key, {});
            }


            //O(log N)
            inline const VALUE& at(const KEY& key) const noexcept { return m_Values[getIndexOfKey(key)]; }

            //O(log N)
            inline VALUE& at(const KEY& key) noexcept { return m_Values[getIndexOfKey(key)]; }
        };

};

#endif
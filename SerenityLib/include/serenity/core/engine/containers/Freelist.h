#pragma once
#ifndef ENGINE_CONTAINERS_FREELIST_H
#define ENGINE_CONTAINERS_FREELIST_H

#include <vector>
#include <serenity/core/engine/system/TypeDefs.h>

namespace Engine {
    template<typename T> class freelist {
        private:
            std::vector<T>         m_Items;
            std::vector<uint32_t>  m_Freelist;
            size_t                 m_Size = 0U;
            uint32_t               m_Auto_Reserve_Count = 25U;

            void initialize(size_t inCapacity) {
                m_Items.reserve(inCapacity);
                m_Freelist.reserve(inCapacity);
                for (int i = (int)inCapacity - 1; i >= 0; --i) {
                    m_Freelist.emplace_back((uint32_t)i);
                }
            }

        public:
            freelist() = default;
            freelist(const size_t capacity_) 
                : freelist() 
            {
                initialize(capacity_);
            }
            virtual ~freelist() {
                clear(false);
            }
            inline void set_auto_reserve_count(uint32_t auto_reserve_count) noexcept {
                m_Auto_Reserve_Count = auto_reserve_count;
            }

            inline constexpr size_t size() const noexcept { return m_Size; }
            inline constexpr size_t capacity() const noexcept { return m_Items.capacity(); }
            //resets the element at the specified index to default constructor of T. if the index is invalid, returns false
            bool reset_element(const size_t& index) {
                if (m_Items.size() >= index) {
                    return false;
                }
                if (std::is_pointer<T>::value) {
                    SAFE_DELETE(m_Items[index]);
                }else{
                    m_Items[index] = T();
                }
                m_Freelist.emplace_back((uint32_t)index);
                return true;
            }
            //resets the element at the specified index to parameter: data. if the index is invalid, returns false
            bool reset_element(const T& data, size_t index) {
                if (m_Items.size() >= index) {
                    return false;
                }
                m_Items[index] = data;
                m_Freelist.emplace_back((uint32_t)index);
                return true;
            }
            //resets the element at the specified index to parameter: data. Frees the heap allocated memory before hand. if the index is invalid, returns false
            bool reset_and_delete_element(const T& data, size_t index) {
                if (m_Items.size() >= index) {
                    return false;
                }
                delete(m_Items[index]);
                m_Items[index] = data;
                m_Freelist.emplace_back((uint32_t)index);
                return true;
            }
            bool reset_and_delete_element(size_t index) {
                return reset_and_delete_element(nullptr, index);
            }

            //adds data to the container. if the container has an available index, it will use that, otherwise it will push this element in the container
            int add_element(T& data) {
                if (m_Freelist.size() == 0) {
                    return push_back(data);
                }
                auto available_index = m_Freelist[m_Freelist.size() - 1];
                m_Freelist.pop_back();
                m_Items[available_index] = data;
                return available_index;
            }
            //clears the elements of the container. will re-initialize the container's capacity if parameter = true. user must manually free heap allocated memory
            void clear(bool reinitialize = true) {
                auto old_capacity = m_Items.capacity();
                m_Items.clear();
                m_Freelist.clear();
                m_Size = 0;
                if (reinitialize) {
                    initialize(old_capacity);
                }
            }
            //adds data to the container by pushing this element in the container even if there is an available freelist index
            int push_back(T& data) {
                if (m_Items.size() >= m_Items.capacity()) {
                    m_Items.reserve(m_Items.capacity() + m_Auto_Reserve_Count);
                }
                if (m_Freelist.size() == 0) {
                    m_Items.push_back(std::move(data));
                    ++m_Size;
                    return (int)m_Items.size() - 1;
                }
                //get a freelist index
                auto available_index = m_Freelist[m_Freelist.size() - 1];
                m_Freelist.pop_back();
                m_Items.push_back(std::move(data));
                ++m_Size;
                return (int)available_index;
            }
            int push_back(const T& data) {
                if (m_Items.size() >= m_Items.capacity()) {
                    m_Items.reserve(m_Items.capacity() + m_Auto_Reserve_Count);
                }
                if (m_Freelist.size() == 0) {
                    m_Items.push_back(std::move(data));
                    ++m_Size;
                    return (int)m_Items.size() - 1;
                }
                //get a freelist index
                auto available_index = m_Freelist[m_Freelist.size() - 1];
                m_Freelist.pop_back();
                m_Items.push_back(std::move(data));
                ++m_Size;
                return (int)available_index;
            }
            //adds data to the container by emplacing this element in the container even if there is an available freelist index
            template<typename... ARGS> int emplace_back(ARGS&& ... args) {
                if (m_Items.size() >= m_Items.capacity()) {
                    m_Items.reserve(m_Items.capacity() + m_Auto_Reserve_Count);
                }
                if (m_Freelist.size() == 0) {
                    m_Items.emplace_back(std::forward<ARGS>(args)...);
                    ++m_Size;
                    return (int)m_Items.size() - 1;
                }
                //get a freelist index
                auto available_index = m_Freelist[m_Freelist.size() - 1];
                m_Freelist.pop_back();
                m_Items.emplace_back(std::forward<ARGS>(args)...);
                ++m_Size;
                return (int)available_index;
            }
            bool set_element(T& data, size_t index) noexcept {
                if (index >= m_Items.size()) {
                    return false;
                }
                m_Items[index] = std::move(data);
                return true;
            }
            inline int get_next_free_index() noexcept {
                return (m_Freelist.size() > 0) ? (int)m_Freelist[m_Freelist.size() - 1] : -1;
            }
            inline const T& get(size_t index) const noexcept { return m_Items[index]; }
            inline T& get(size_t index) noexcept { return m_Items[index]; }
            inline T& operator[](size_t index) noexcept { return m_Items[index]; }
            inline const T& operator[](const size_t& index) const noexcept { return m_Items[index]; }

            inline std::vector<T>& data() noexcept { return m_Items; }
            inline const std::vector<T>& data() const noexcept { return m_Items; }

            typename inline std::vector<T>::iterator begin() { return m_Items.begin(); }
            typename inline std::vector<T>::iterator end() { return m_Items.end(); }
            typename inline std::vector<T>::const_iterator begin() const { return m_Items.begin(); }
            typename inline std::vector<T>::const_iterator end() const { return m_Items.end(); }
    };
};
#endif
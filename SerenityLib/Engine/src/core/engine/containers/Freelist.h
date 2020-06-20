#pragma once
#ifndef ENGINE_CONTAINERS_FREELIST_H
#define ENGINE_CONTAINERS_FREELIST_H

#include <vector>
#include <type_traits>

namespace Engine {
    template<typename T> class freelist {
        private:
            std::vector<T>             m_Items;
            std::vector<std::uint32_t> m_Freelist;
            size_t                     m_Size = 0U;
            std::uint32_t              m_Auto_Reserve_Count = 25U;

            void initialize(const size_t capacity_) {
                m_Items.reserve(capacity_);
                m_Freelist.reserve(capacity_);
                for (int i = static_cast<int>(capacity_) - 1; i >= 0; --i) {
                    m_Freelist.push_back(static_cast<std::uint32_t>(i));
                }
            }

        public:
            freelist() {
            }
            freelist(const size_t capacity_) : freelist(){
                initialize(capacity_);
            }
            ~freelist() {
                clear(false);
            }
            void set_auto_reserve_count(std::uint32_t auto_reserve_count_) {
                m_Auto_Reserve_Count = auto_reserve_count_;
            }

            constexpr size_t size() const {
                return m_Size;
            }
            constexpr size_t capacity() const {
                return m_Items.capacity();
            }
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
                m_Freelist.push_back(static_cast<std::uint32_t>(index));
                return true;
            }
            //resets the element at the specified index to parameter: data. if the index is invalid, returns false
            bool reset_element(const T& data, const size_t& index) {
                if (m_Items.size() >= index) {
                    return false;
                }
                m_Items[index] = data;
                m_Freelist.push_back(static_cast<std::uint32_t>(index));
                return true;
            }
            //resets the element at the specified index to parameter: data. Frees the heap allocated memory before hand. if the index is invalid, returns false
            bool reset_and_delete_element(const T& data, const size_t& index) {
                if (m_Items.size() >= index) {
                    return false;
                }
                delete(m_Items[index]);
                m_Items[index] = data;
                m_Freelist.push_back(static_cast<std::uint32_t>(index));
                return true;
            }
            bool reset_and_delete_element(const size_t& index) {
                return reset_and_delete_element(nullptr, index);
            }


            //adds data to the container. if the container has an available index, it will use that, otherwise it will push this element in the container
            int add_element(T& data) {
                if (m_Freelist.size() == 0) {
                    return push_back(data);
                }
                const auto available_index = m_Freelist[m_Freelist.size() - 1];
                m_Freelist.pop_back();
                m_Items[available_index] = data;
                return available_index;
            }
            //clears the elements of the container. will re-initialize the container's capacity if parameter = true. user must manually free heap allocated memory
            void clear(const bool reinitialize = true) {
                const auto old_capacity = m_Items.capacity();
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
                    return static_cast<int>(m_Items.size()) - 1;
                }
                //get a freelist index
                const auto available_index = m_Freelist[m_Freelist.size() - 1];
                m_Freelist.pop_back();
                m_Items.push_back(std::move(data));
                ++m_Size;
                return static_cast<int>(available_index);
            }
            int push_back(const T& data) {
                if (m_Items.size() >= m_Items.capacity()) {
                    m_Items.reserve(m_Items.capacity() + m_Auto_Reserve_Count);
                }
                if (m_Freelist.size() == 0) {
                    m_Items.push_back(std::move(data));
                    ++m_Size;
                    return static_cast<int>(m_Items.size()) - 1;
                }
                //get a freelist index
                const auto available_index = m_Freelist[m_Freelist.size() - 1];
                m_Freelist.pop_back();
                m_Items.push_back(std::move(data));
                ++m_Size;
                return static_cast<int>(available_index);
            }
            //adds data to the container by emplacing this element in the container even if there is an available freelist index
            template<typename... ARGS> const int emplace_back(ARGS&& ... args) {
                if (m_Items.size() >= m_Items.capacity()) {
                    m_Items.reserve(m_Items.capacity() + m_Auto_Reserve_Count);
                }
                if (m_Freelist.size() == 0) {
                    m_Items.emplace_back(std::forward<ARGS>(args)...);
                    ++m_Size;
                    return static_cast<int>(m_Items.size()) - 1;
                }
                //get a freelist index
                const auto available_index = m_Freelist[m_Freelist.size() - 1];
                m_Freelist.pop_back();
                m_Items.emplace_back(std::forward<ARGS>(args)...);
                ++m_Size;
                return static_cast<int>(available_index);
            }
            bool set_element(T& data, const size_t index) {
                if (index >= m_Items.size()) {
                    return false;
                }
                m_Items[index] = std::move(data);
                return true;
            }
            int get_next_free_index() {
                return (m_Freelist.size() > 0) ? static_cast<int>(m_Freelist[m_Freelist.size() - 1]) : -1;
            }
            /*
            T* get_next_free() {
                if (m_Freelist.size() > 0) {
                    const auto available_index = m_Freelist[m_Freelist.size() - 1];
                    m_Freelist.pop_back();
                    T& item = const_cast<T&>(m_Items[available_index]);
                    return &item;
                }
                return nullptr;
            }
            */
            const T& get(const size_t& index) const {
                return m_Items[index];
            }
            T& get(const size_t& index) {
                return m_Items[index];
            }
            T& operator[](const size_t& index) {
                return m_Items[index];
            }
            const T& operator[](const size_t& index) const {
                return m_Items[index];
            }

            std::vector<T>& data() {
                return m_Items;
            }
           
            typename std::vector<T>::iterator begin() {
                return m_Items.begin();
            }
            typename std::vector<T>::iterator end() {
                return m_Items.end();
            }
            typename std::vector<T>::const_iterator begin() const {
                return m_Items.begin();
            }
            typename std::vector<T>::const_iterator end() const {
                return m_Items.end();
            }
    };
};
#endif
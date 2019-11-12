#pragma once
#ifndef GAME_FREELIST_H
#define GAME_FREELIST_H

#include <vector>
#include <stack>

template<typename T> class Freelist {
    private:
        std::vector<T>            m_Items;
        std::stack<unsigned int>  m_Freelist;
        size_t                    m_Size;
    public:
        Freelist() {
            m_Size = 0;
        }
        Freelist(const size_t _capacity) {
            initialize(_capacity);
        }
        ~Freelist() {
            clear(false);
        }
        void initialize(const size_t _capacity) {
            m_Items.reserve(_capacity);
            for (size_t i = 0; i < _capacity; ++i) {
                m_Items.push_back(nullptr);
            }
            for (int i = static_cast<int>(_capacity) - 1; i >= 0; --i) {
                m_Freelist.push(static_cast<unsigned int>(i));
            }
        }
        const size_t& size() const {
            return m_Size;
        }
        void clear(const bool reinit = true) {
            auto cap = m_Items.capacity();
            for (size_t i = 0; i < m_Items.size(); ++i) {
                if (m_Items[i]) {
                    delete m_Items[i];
                    m_Items[i] = nullptr;
                }
            }
            m_Items.clear();
            m_Freelist = std::stack<unsigned int>();
            m_Size = 0;
            if(reinit)
                initialize(cap);
        }
        const bool delete_data(T& data) {
            for (size_t i = 0; i < m_Items.size(); ++i) {
                if (m_Items[i] == data) {
                    delete m_Items[i];
                    m_Items[i] = nullptr;
                    m_Freelist.push(static_cast<unsigned int>(i));
                    --m_Size;
                    return true;
                }
            }
            return false;
        }
        const bool delete_data_index(const size_t index) {
            if (m_Items[index]) {
                delete m_Items[index];
                m_Items[index] = nullptr;
                m_Freelist.push(static_cast<unsigned int>(index));
                --m_Size;
                return true;
            }
            return false;
        }
        const int can_push() {
            if (m_Freelist.size() == 0)
                return -1;
            //get a freelist index
            const auto available_index = m_Freelist.top();
            return available_index;
        }
        const bool can_push_at_index(const size_t index) {
            if (!m_Items[index])
                return true;
            return false;
        }
        const int use_next_available_index() {
            if (m_Freelist.size() == 0)
                return -1;
            //get a freelist index
            const auto available_index = m_Freelist.top();
            m_Freelist.pop();
            return available_index;
        }
        const int push_back(T& data) {
            if (m_Freelist.size() == 0)
                return -1;
            //get a freelist index
            const auto available_index = m_Freelist.top();
            m_Freelist.pop();
            m_Items[available_index] = std::move(data);
            ++m_Size;
            return available_index;
        }
        const bool insert(T& data, const size_t index) {
            if (!m_Items[index]){
                m_Items[index] = std::move(data);
                ++m_Size;
                return true;
            }
            return false;
        }
        T& operator[](const size_t index) {
            return m_Items[index];
        }
        std::vector<T>& data() {
            return m_Items;
        }
};

#endif
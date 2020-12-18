#pragma once
#ifndef ENGINE_CONTAINERS_HEAP_H
#define ENGINE_CONTAINERS_HEAP_H

#include <functional>
#include <vector>

namespace Engine {
    template<typename T>
    class heap {
        struct node final {
            int weight = 0;
            T   data;

            node() = default;
            node(T&& inData, int inWeight) {
                data   = std::move(inData);
                weight = inWeight;
            }
            node(const node& other)            = default;
            node& operator=(const node& other) = default;
            node(node&& other)                 = default;
            node& operator=(node&& other)      = default;
        };
        private:
            node m_Dummy_Left  = {T(), 0};
            node m_Dummy_Right = {T(), 1};

            std::function<bool(node&, node&)> m_Rule = [](node& lhs, node& rhs) { return lhs.weight < rhs.weight; };
            std::vector<node>                 m_Items;

            inline size_t parent(size_t index) const noexcept { return (index - 1) / 2; }
            inline size_t left_child(size_t index) const noexcept { return (index * 2) + 1; }
            inline size_t right_child(size_t index) const noexcept { return (index * 2) + 2; }

            void heapify(size_t index) noexcept {
                size_t left     = left_child(index);
                size_t right    = right_child(index);
                size_t smallest = index;
                if (left < m_Items.size() && m_Rule(m_Items[left], m_Items[index])) {
                    smallest = left;
                }
                if (right < m_Items.size() && m_Rule(m_Items[right], m_Items[smallest])) {
                    smallest = right;
                }
                if (smallest != index){
                    std::swap(m_Items[index], m_Items[smallest]);
                    heapify(smallest);
                }
            }
            void bubble_up(size_t index) noexcept {
                while (index != 0 && !m_Rule(m_Items[parent(index)], m_Items[index])) {
                    std::swap(m_Items[parent(index)], m_Items[index]);
                    index = parent(index);
                }
            }
        public:
            heap(size_t capacity = 10) {
                m_Items.reserve(capacity);
            }
            heap(std::function<bool(node&, node&)> rule, size_t capacity = 10) {
                m_Rule = rule;
                m_Items.reserve(capacity);
            }
            heap(std::function<bool(T&, T&)> rule, size_t capacity = 10) {
                m_Rule = [rule](node& lhs, node& rhs) {
                    return rule(lhs.weight, rhs.weight);
                };
                m_Items.reserve(capacity);
            }
            virtual ~heap() {}

            inline size_t size() const noexcept { return m_Items.size(); }
            inline size_t capacity() const noexcept { return m_Items.capacity(); }

            T top() noexcept {
                T extraction = m_Items[0].data;
                return extraction;
            }
            void pop() noexcept {
                m_Items[0] = m_Items.back();
                m_Items.pop_back();
                heapify(0);
            }
            T extract() noexcept {
                T extraction = m_Items[0].data;
                pop();
                return extraction;
            }
            void push(T item) noexcept {
                push(std::move(item), (int)item);
            }
            void emplace(T&& item) noexcept {
                int newkey = (int)item;
                emplace(std::move(item), newkey);
            }
            void push(T item, int newWeight) noexcept {
                m_Items.emplace_back(std::move(item), newWeight);
                bubble_up(m_Items.size() - 1);
            }
            void emplace(T&& item, int newWeight) noexcept {
                int newkey = (int)item;
                m_Items.emplace_back(std::move(item), newWeight);
                bubble_up(m_Items.size() - 1);
            }




            void modify_push(size_t index, T newValue) noexcept {
                modify_push(index, newValue, (int)newValue);
            }
            void modify_emplace(size_t index, T&& newValue) noexcept {
                int newkey = (int)newValue;
                modify_emplace(index, std::move(newValue), newkey);
            }

            void modify_push(size_t index, T newValue, int newWeight) noexcept {
                m_Items[index].data   = newValue;
                m_Items[index].weight += (m_Rule(m_Dummy_Left, m_Dummy_Right) == false) ? -newWeight : newWeight;
                bubble_up(index);
            }
            void modify_emplace(size_t index, T&& newValue, int newWeight) noexcept {
                m_Items[index].data   = (newValue);
                m_Items[index].weight += (m_Rule(m_Dummy_Left, m_Dummy_Right) == false) ? -newWeight : newWeight;
                bubble_up(index);
            }
            void remove(size_t index) noexcept {
                modify_emplace(index, (m_Rule(m_Dummy_Left, m_Dummy_Right) == false) ? INT_MAX : INT_MIN);
                extract();
            }

            typename std::vector<T>::iterator begin() { return m_Items.begin(); }
            typename std::vector<T>::iterator end() { return m_Items.end(); }
            typename std::vector<T>::const_iterator begin() const { return m_Items.begin(); }
            typename std::vector<T>::const_iterator end() const { return m_Items.end(); }

    };
};

#endif
#ifndef ENGINE_CONTAINERS_SPARSE_SET_H
#define ENGINE_CONTAINERS_SPARSE_SET_H

#include <vector>
#include <typeinfo>
#include <serenity/system/Macros.h>

namespace Engine {
    namespace priv {
        class sparse_set_base {
            protected:
                inline static constexpr uint32_t NULL_IDX = std::numeric_limits<uint32_t>().max();

                std::vector<uint32_t>  m_Sparse;

                sparse_set_base() = default;
            public:
                sparse_set_base(uint32_t initial_capacity) {
                    m_Sparse.reserve(initial_capacity);
                }

                sparse_set_base(const sparse_set_base&)                = default;
                sparse_set_base& operator=(const sparse_set_base&)     = default;
                sparse_set_base(sparse_set_base&&) noexcept            = default;
                sparse_set_base& operator=(sparse_set_base&&) noexcept = default;
                virtual ~sparse_set_base() = default;

                [[nodiscard]] inline bool has(uint32_t id) noexcept { 
                    return id < m_Sparse.size() ? m_Sparse[id] != NULL_IDX : false;
                }

                virtual bool remove(uint32_t id) {
                    return false;
                }
                virtual void reserve(uint32_t amount) {
                    if (m_Sparse.size() == m_Sparse.capacity()) {
                        m_Sparse.reserve(m_Sparse.capacity() + amount);
                    }
                }
                virtual void clear() {
                    m_Sparse.clear();
                }
#ifndef ENGINE_PRODUCTION
                virtual const char* getComponentDebugName() const { return ""; }
#endif

            };
    };
    template <class T>
    class sparse_set : public Engine::priv::sparse_set_base {
        using super     = Engine::priv::sparse_set_base;
        using container = std::vector<T>;
        private:
            container                m_Dense;
            std::vector<uint32_t>    m_DenseMapping;
        public:
            sparse_set() = default;
            sparse_set(uint32_t initial_capacity) 
                : super{ initial_capacity }
            {
                m_DenseMapping.reserve(initial_capacity);
                m_Dense.reserve(initial_capacity);
            }

            sparse_set(const sparse_set&)                = default;
            sparse_set& operator=(const sparse_set&)     = default;
            sparse_set(sparse_set&&) noexcept            = default;
            sparse_set& operator=(sparse_set&&) noexcept = default;

            virtual ~sparse_set() = default;
            void clear() override {
                m_Dense.clear();
                m_DenseMapping.clear();
                super::clear();
            }

            void reserve(uint32_t amount) override {
                super::reserve(amount);
                if (m_Dense.size() == m_Dense.capacity()) {
                    m_Dense.reserve(m_Dense.capacity() + amount);
                    m_DenseMapping.reserve(m_DenseMapping.capacity() + amount);
                }
            }
            template<class ... ARGS> 
            T* add(uint32_t id, ARGS&&... args) {
                if (size_t(id) >= super::m_Sparse.size()) {
                    super::m_Sparse.resize(size_t(id) + 1, NULL_IDX);
                }
                if (super::m_Sparse[id] != NULL_IDX) {
                    return nullptr;
                }
                m_Dense.emplace_back(std::forward<ARGS>(args)...);
                m_DenseMapping.emplace_back(id);
                super::m_Sparse[id] = uint32_t(m_Dense.size()) - 1;
                return &m_Dense[super::m_Sparse[id]];
            }
            bool remove(uint32_t id) override {
                if (id >= super::m_Sparse.size()) {
                    return false;
                }
                const auto removedComponentID = super::m_Sparse[id];
                if (removedComponentID == NULL_IDX) {
                    return false;
                }
                const auto lastIndex = m_Dense.size() - 1;
                super::m_Sparse[m_DenseMapping[lastIndex]] = removedComponentID;
                super::m_Sparse[id] = NULL_IDX;
                if (removedComponentID != lastIndex) {
                    std::swap(m_Dense[removedComponentID], m_Dense[lastIndex]);
                    std::swap(m_DenseMapping[removedComponentID], m_DenseMapping[lastIndex]);
                }
                m_Dense.pop_back();
                m_DenseMapping.pop_back();
                return true;
            }
            [[nodiscard]] T* get(uint32_t id) {
                if (id >= super::m_Sparse.size() || super::m_Sparse[id] == NULL_IDX) {
                    return nullptr;
                }
                const auto retIndex = super::m_Sparse[id];
                return retIndex < m_Dense.size() ? &m_Dense[retIndex] : nullptr;
            }
#ifndef ENGINE_PRODUCTION
            const char* getComponentDebugName() const override { return typeid(T).name(); }
#endif

            inline T& operator[](size_t index) { return m_Dense[index]; }
            inline const T& operator[](size_t index) const noexcept { return m_Dense[index]; }
            inline size_t size() const noexcept { return m_Dense.size(); }
            inline container& data() noexcept { return m_Dense; }

            BUILD_TEMPLATE_BEGIN_END_ITR_CLASS_MEMBERS(container, m_Dense)
    };
};

#endif
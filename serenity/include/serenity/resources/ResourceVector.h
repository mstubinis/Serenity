#pragma once
#ifndef ENGINE_RESOURCES_RESOURCE_VECTOR_H
#define ENGINE_RESOURCES_RESOURCE_VECTOR_H

#include <serenity/threading/ThreadingModule.h>

namespace Engine::priv {
    class IResourceVector {
        public:
            virtual ~IResourceVector() {}

            [[nodiscard]] virtual std::mutex& getMutex() noexcept = 0;
            [[nodiscard]] virtual size_t size() const noexcept = 0;
            [[nodiscard]] virtual void* getVoid(const Handle inHandle) const noexcept = 0;

            virtual void reserve(const size_t inSize) = 0;
            virtual void lock() = 0;
            virtual void unlock() = 0;
            virtual void shrink_to_fit() = 0;
            //virtual void erase(const Handle inHandle) noexcept = 0;
    };
};

namespace Engine::priv {
    template<class RESOURCE>
    class ResourceVector final : public IResourceVector {
        private:
            std::vector<RESOURCE*>      m_Resources;
            mutable std::mutex          m_Mutex;
            bool                        m_Locked     = false;

            [[nodiscard]] inline Engine::view_ptr<RESOURCE> internal_get(const Handle inHandle) const noexcept { return m_Resources[inHandle.index()]; }

            template<class INTEGRAL>
            requires (std::is_integral_v<INTEGRAL>)
            [[nodiscard]] inline Engine::view_ptr<RESOURCE> internal_get(const INTEGRAL index) const noexcept { return m_Resources[static_cast<size_t>(index)]; }
        public:
            ResourceVector(size_t reserveSize = 1) {
                m_Resources.reserve(reserveSize);
            }
            ResourceVector(const ResourceVector&)                = delete;
            ResourceVector& operator=(const ResourceVector&)     = delete;
            ResourceVector(ResourceVector&&) noexcept            = delete;
            ResourceVector& operator=(ResourceVector&&) noexcept = delete;
            ~ResourceVector() {
                SAFE_DELETE_VECTOR(m_Resources);
            }

            [[nodiscard]] inline size_t size() const noexcept override {
                return m_Resources.size(); 
            }
            void shrink_to_fit() override {
                m_Resources.shrink_to_fit();
            }

            [[nodiscard]] inline std::mutex& getMutex() noexcept override { return m_Mutex; }

            void lock() override {
                m_Locked = true;
                shrink_to_fit();
            }
            void unlock() override {
                m_Locked = false;
            }

            [[nodiscard]] Engine::view_ptr<RESOURCE> get(const Handle inHandle) noexcept {
                ASSERT(inHandle.index() >= 0 && inHandle.index() < m_Resources.size(), __FUNCTION__ << "(): handle index was out of bounds!");
                return internal_get(inHandle);
            }
            template<class INTEGRAL>
            requires (std::is_integral_v<INTEGRAL>)
            [[nodiscard]] Engine::view_ptr<RESOURCE> get(const INTEGRAL index) noexcept {
                ASSERT(index >= 0 && index < m_Resources.size(), __FUNCTION__ << "(): handle index was out of bounds!");
                return internal_get(index);
            }
            void* getVoid(const Handle inHandle) const noexcept override {
                if (inHandle.null()) {
                    return nullptr;
                }
                ASSERT(inHandle.index() >= 0 && inHandle.index() < m_Resources.size(), __FUNCTION__ << "(): handle index was out of bounds!");
                return (void*)internal_get(inHandle);
            }
            //O(n) linear search
            [[nodiscard]] LoadedResource<RESOURCE> get(const std::string_view sv) noexcept {
                for (uint32_t i = 0; i < uint32_t(m_Resources.size()); ++i) {
                    if (m_Resources[i]->name() == sv) {
                        return { internal_get(i), Handle{i, RESOURCE::TYPE_ID} };
                    }
                }
                return {};
            }

            template<class ... ARGS>
            size_t emplace_back(ARGS&&... args) { 
                ASSERT(!m_Locked, __FUNCTION__ << "(): is locked!");
                if (m_Locked) {
                    return 0;
                }
                {
                    std::scoped_lock lock{ m_Mutex };
                    m_Resources.emplace_back(NEW RESOURCE( std::forward<ARGS>(args)... ));
                }
                return m_Resources.size() - 1;
            }
            /*
            size_t push_back(RESOURCE&& inResource) {
                ASSERT(!m_Locked, __FUNCTION__ << "(): is locked!");
                if (m_Locked) {
                    return 0;
                }
                {
                    std::scoped_lock lock{ m_Mutex };
                    m_Resources.push_back(std::move(inResource));
                }
                return m_Resources.size() - 1;
            }
            */
            /*
            inline void erase(const Handle inHandle) noexcept override {
                ASSERT(!m_Locked, __FUNCTION__ << "(): is locked!");
                if (m_Locked) {
                    return;
                }
                ASSERT(inHandle.index() >= 0 && inHandle.index() < m_Resources.size(), __FUNCTION__ << "(): handle index was out of bounds!");
                m_Resources[inHandle.index()].m_Version++;
            }
            */
            inline void reserve(const size_t inSize) override { 
                ASSERT(!m_Locked, __FUNCTION__ << "(): is locked!");
                if (m_Locked) {
                    return;
                }
                m_Resources.reserve(inSize); 
            }

            //O(n) linked-list assembly from array
            [[nodiscard]] std::list<Engine::view_ptr<RESOURCE>> getAsList() const {
                std::list<Engine::view_ptr<RESOURCE>> returnedList;
                for (const auto& entry : m_Resources) {
                    returnedList.push_back( entry );
                }
                return returnedList;
            }

    };
};

#endif
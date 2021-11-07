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
            virtual void erase(const Handle inHandle) noexcept = 0;
    };
};

namespace Engine::priv {
    template<class RESOURCE>
    class ResourceVector final : public IResourceVector {
        struct Entry final {
            RESOURCE*  m_Resource  = nullptr;
            uint32_t   m_Version   = 0;

            template<class ... ARGS>
            Entry(ARGS&&... args) {
                m_Resource = NEW RESOURCE(std::forward<ARGS>(args)...);
            }
            Entry(const Entry&)                = delete;
            Entry& operator=(const Entry&)     = delete;
            Entry(Entry&& other) noexcept
                : m_Resource{ std::exchange(other.m_Resource, nullptr) }
                , m_Version  { std::move(other.m_Version) }
            {}
            Entry& operator=(Entry&& other) noexcept {
                if (this != &other) {
                    m_Resource = std::exchange(other.m_Resource, nullptr);
                    m_Version  = std::move(other.m_Version);
                }
                return *this;
            }
            ~Entry() {
                SAFE_DELETE(m_Resource);
            }
        };

        private:
            std::vector<Entry>          m_Resources;
            mutable std::mutex          m_Mutex;
            bool                        m_Locked     = false;

            [[nodiscard]] inline Engine::view_ptr<RESOURCE> internal_get(const Handle inHandle) const noexcept { return m_Resources[inHandle.index()].m_Resource; }
            [[nodiscard]] inline Engine::view_ptr<RESOURCE> internal_get(const size_t index) const noexcept { return m_Resources[index].m_Resource; }
        public:
            ResourceVector(size_t reserveSize = 1) {
                m_Resources.reserve(reserveSize);
            }
            ResourceVector(const ResourceVector&)                = delete;
            ResourceVector& operator=(const ResourceVector&)     = delete;
            ResourceVector(ResourceVector&&) noexcept            = delete;
            ResourceVector& operator=(ResourceVector&&) noexcept = delete;

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
                if (inHandle.version() != m_Resources[inHandle.index()].m_Version) {
                    return nullptr;
                }
                ASSERT(inHandle.index() >= 0 && inHandle.index() < m_Resources.size(), __FUNCTION__ << "(): handle index was out of bounds!");
                return internal_get(inHandle);
            }
            void* getVoid(const Handle inHandle) const noexcept override {
                if (inHandle.null() || (inHandle.version() != m_Resources[inHandle.index()].m_Version)) {
                    return nullptr;
                }
                ASSERT(inHandle.index() >= 0 && inHandle.index() < m_Resources.size(), __FUNCTION__ << "(): handle index was out of bounds!");
                return (void*)internal_get(inHandle);
            }
            //O(n) linear search
            [[nodiscard]] LoadedResource<RESOURCE> get(const std::string_view sv) noexcept {
                for (uint32_t i = 0; i < (uint32_t)m_Resources.size(); ++i) {
                    if (m_Resources[i].m_Resource->name() == sv) {
                        return { internal_get(i), Handle{i, m_Resources[i].m_Version, RESOURCE::TYPE_ID} };
                    }
                }
                return {};
            }

            template<typename ...ARGS>
            size_t emplace_back(ARGS&&... args) { 
                ASSERT(!m_Locked, __FUNCTION__ << "(): is locked!");
                if (m_Locked) {
                    return 0;
                }
                {
                    std::lock_guard lock{ m_Mutex };
                    m_Resources.emplace_back(std::forward<ARGS>(args)...);
                }
                return m_Resources.size() - 1;
            }
            
            size_t push_back(RESOURCE&& inResource) {
                ASSERT(!m_Locked, __FUNCTION__ << "(): is locked!");
                if (m_Locked) {
                    return 0;
                }
                {
                    std::lock_guard lock{ m_Mutex };
                    m_Resources.push_back(std::move(inResource));
                }
                return m_Resources.size() - 1;
            }

            inline void erase(const Handle inHandle) noexcept override {
                ASSERT(!m_Locked, __FUNCTION__ << "(): is locked!");
                if (m_Locked) {
                    return;
                }
                ASSERT(inHandle.index() >= 0 && inHandle.index() < m_Resources.size(), __FUNCTION__ << "(): handle index was out of bounds!");
                m_Resources[inHandle.index()].m_Version++;
            }

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
                    returnedList.push_back( entry.m_Resource );
                }
                return returnedList;
            }

    };
};

#endif
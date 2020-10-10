#pragma once
#ifndef ENGINE_RESOURCES_RESOURCE_VECTOR_H
#define ENGINE_RESOURCES_RESOURCE_VECTOR_H

#include <core/engine/resources/IResourceVector.h>
#include <core/engine/threading/ThreadingModule.h>

namespace Engine::priv {
    template<typename TResource>
    class ResourceVector final : public IResourceVector {

        struct Entry final {
            std::unique_ptr<TResource>  m_Resource;
            uint32_t                    m_Version{ 0 };

            template<typename ... ARGS>
            Entry(ARGS&&... args){
                m_Resource = std::make_unique<TResource>(std::forward<ARGS>(args)...);
            }
            Entry(const Entry& other)                = delete;
            Entry& operator=(const Entry& other)     = delete;
            Entry(Entry&& other) noexcept
                : m_Version  { std::move(other.m_Version) }
                , m_Resource { std::move(other.m_Resource) }
            {}
            Entry& operator=(Entry&& other) noexcept {
                m_Version    = std::move(other.m_Version);
                m_Resource   = std::move(other.m_Resource);
                return *this;
            }
        };

        private:
            std::vector<Entry>      m_Resources;
            std::vector<uint32_t>   m_AvailableIndices;
            std::mutex              m_Mutex;
            bool                    m_Locked             = false;

            inline TResource* internal_get(const Handle inHandle) const noexcept { return m_Resources[inHandle.index()].m_Resource.get(); }
            inline TResource* internal_get(const size_t index) const noexcept { return m_Resources[index].m_Resource.get(); }
        public:
            ResourceVector(size_t reserveSize = 1) {
                m_Resources.reserve(reserveSize);
                m_AvailableIndices.reserve(reserveSize);
            }
            ResourceVector(const ResourceVector& other)                = delete;
            ResourceVector& operator=(const ResourceVector& other)     = delete;
            ResourceVector(ResourceVector&& other) noexcept            = delete;
            ResourceVector& operator=(ResourceVector&& other) noexcept = delete;

            std::mutex* getMutex() noexcept override { return &m_Mutex; }

            void lock() override {
                m_Locked = true;
                shrink_to_fit();
            }
            void unlock() override {
                m_Locked = false;
            }

            void shrink_to_fit() override {
                m_Resources.shrink_to_fit();
                m_AvailableIndices.shrink_to_fit();
            }

            uint32_t pop_index() noexcept {
                ASSERT(!m_Locked, __FUNCTION__ << "(): is locked!");
                if (m_Locked) {
                    return 0;
                }
                ASSERT(m_AvailableIndices.size() > 0, __FUNCTION__ << "(): m_AvailableIndices was empty!");
                auto index = m_AvailableIndices.back();
                m_AvailableIndices.pop_back();
                return index;
            }
            Engine::view_ptr<TResource> get(const Handle inHandle) noexcept {
                if (inHandle.version() != m_Resources[inHandle.index()].m_Version) {
                    return nullptr;
                }
                ASSERT(inHandle.index() >= 0 && inHandle.index() < m_Resources.size(), __FUNCTION__ << "(): handle index was out of bounds!");
                return internal_get(inHandle);
            }

            void get(void*& out, const Handle inHandle) const noexcept override {
                if (inHandle.version() != m_Resources[inHandle.index()].m_Version) {
                    out = nullptr;
                }
                ASSERT(inHandle.index() >= 0 && inHandle.index() < m_Resources.size(), __FUNCTION__ << "(): handle index was out of bounds!");
                out = (void*)internal_get(inHandle);
            }

            std::pair<Engine::view_ptr<TResource>, Handle> get(const std::string_view sv) noexcept {
                for (uint32_t i = 0; i < (uint32_t)m_Resources.size(); ++i) {
                    if (m_Resources[i].m_Resource->name() == sv) {
                        return std::make_pair(internal_get(i), Handle( i, m_Resources[i].m_Version, 0U ) );
                    }
                }
                return std::make_pair( nullptr, Handle{} );
            }

            template<typename ...ARGS>
            size_t emplace_back(ARGS&&... args) { 
                ASSERT(!m_Locked, __FUNCTION__ << "(): is locked!");
                if (m_Locked) {
                    return 0;
                }
                if (m_AvailableIndices.size() > 0) {
                    size_t index;
                    {
                        std::lock_guard lock{ m_Mutex };
                        index = pop_index();
                        ASSERT(index >= 0 && index < m_Resources.size(), "Engine::priv::ResourceVector::push_back(): index was out of bounds!");
                        m_Resources[index] = Entry{ std::forward<ARGS>(args)... };
                    }
                    return index;
                }
                {
                    std::lock_guard lock{ m_Mutex };
                    m_Resources.emplace_back(std::forward<ARGS>(args)...);
                }
                return m_Resources.size() - 1;
            }
            
            size_t push_back(TResource&& inResource) {
                ASSERT(!m_Locked, "Engine::priv::ResourceVector::push_back(): is locked!");
                if (m_Locked) {
                    return 0;
                }
                if (m_AvailableIndices.size() > 0) {
                    size_t index;
                    {
                        std::lock_guard lock{ m_Mutex };
                        index = pop_index();
                        ASSERT(index >= 0 && index < m_Resources.size(), "Engine::priv::ResourceVector::push_back(): index was out of bounds!");
                        m_Resources[index] = Entry{ std::move(inResource) };
                    }
                    return index;
                }
                {
                    std::lock_guard lock{ m_Mutex };
                    m_Resources.push_back(std::move(inResource));
                }
                return m_Resources.size() - 1;
            }

            inline void erase(const Handle inHandle) noexcept { 
                ASSERT(!m_Locked, "Engine::priv::ResourceVector::erase(): is locked!");
                if (m_Locked) {
                    return;
                }
                ASSERT(inHandle.index() >= 0 && inHandle.index() < m_Resources.size(), "Engine::priv::ResourceVector::erase(): handle index was out of bounds!");
                m_Resources[inHandle.index()].m_Version++;
            }

            inline void reserve(const size_t inSize) override { 
                ASSERT(!m_Locked, "Engine::priv::ResourceVector::reserve(): is locked!");
                if (m_Locked) {
                    return;
                }
                m_Resources.reserve(inSize); 
            }
            inline constexpr size_t size() const noexcept override {  return m_Resources.size();  }

            std::list<Engine::view_ptr<TResource>> getAsList() {
                std::list<Engine::view_ptr<TResource>> returnedList;
                for (const auto& entry : m_Resources) {
                    returnedList.push_back((TResource*)entry.m_Resource.get());
                }
                return returnedList;
            }

    };
};

#endif
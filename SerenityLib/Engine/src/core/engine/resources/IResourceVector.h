#pragma once
#ifndef ENGINE_RESOURCES_I_RESOURCE_VECTOR_H
#define ENGINE_RESOURCES_I_RESOURCE_VECTOR_H

#include <core/engine/resources/Handle.h>

namespace Engine::priv {
    class IResourceVector {
        public:
            virtual ~IResourceVector() {}

            virtual void reserve(const size_t inSize) = 0;

            virtual void lock() = 0;
            virtual void unlock() = 0;

            virtual void shrink_to_fit() = 0;

            virtual Engine::view_ptr<std::shared_mutex> getMutex() noexcept = 0;

            virtual size_t size() const noexcept = 0;

            virtual void get(void*& out, const Handle inHandle) const noexcept = 0;

            virtual void erase(const Handle inHandle) noexcept = 0;
    };
};

#endif
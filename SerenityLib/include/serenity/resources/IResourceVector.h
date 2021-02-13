#pragma once
#ifndef ENGINE_RESOURCES_I_RESOURCE_VECTOR_H
#define ENGINE_RESOURCES_I_RESOURCE_VECTOR_H

#include <serenity/resources/Handle.h>
#include <mutex>

namespace Engine::priv {
    class IResourceVector {
        public:
            virtual ~IResourceVector() {}

            virtual void reserve(const size_t inSize) = 0;

            virtual void lock() = 0;
            virtual void unlock() = 0;

            virtual void shrink_to_fit() = 0;

            [[nodiscard]] virtual std::mutex& getMutex() noexcept = 0;

            [[nodiscard]] virtual size_t size() const noexcept = 0;

            [[nodiscard]] virtual void get(void*& out, const Handle inHandle) const noexcept = 0;

            virtual void erase(const Handle inHandle) noexcept = 0;
    };
};

#endif
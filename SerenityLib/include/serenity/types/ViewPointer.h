#pragma once
#ifndef ENGINE_TYPES_VIEW_POINTER_H
#define ENGINE_TYPES_VIEW_POINTER_H

namespace Engine {
    //a simple wrapper around a raw pointer that prevents explicit deletion by the user.
    template<typename T>
    class view_ptr final {
        private:
            T* m_Ptr = nullptr;
        public:
            view_ptr() = default;

            inline T* operator->() const noexcept { return m_Ptr; }
            inline T& operator* () noexcept { return *m_Ptr; }
            inline operator bool() const noexcept { return static_cast<bool>(m_Ptr); }
            inline operator T*() const noexcept { return m_Ptr; }

            //returns the static_cast of the underlying pointer to the templated type if the typeid's match. Otherwise returns nullptr.
            //this is faster than using dynamic_cast
            template<class U> inline U* cast_type() noexcept { return (m_Ptr && typeid(U) == typeid(*m_Ptr)) ? static_cast<U*>(m_Ptr) : nullptr; }

            //returns the dynamic_cast of the underlying pointer to the templated type
            template<class U> inline U* cast_dynamic() noexcept { return dynamic_cast<U*>(m_Ptr); }

            //returns the static_cast of the underlying pointer to the templated type
            template<class U> inline U* cast() noexcept { return static_cast<U*>(m_Ptr); }

            view_ptr(T* ptr)
                : m_Ptr{ ptr }
            {}


    };
}

#endif
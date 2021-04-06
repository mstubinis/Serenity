#pragma once
#ifndef ENGINE_LUA_H
#define ENGINE_LUA_H

#include <serenity/lua/LuaModule.h>
#include <serenity/lua/LuaScript.h>
#include <serenity/system/Macros.h>
#include <memory>

template <typename OWNER, typename... ARGS> 
class LuaCallableFunction final {
    using cpp_type = void(*)(const OWNER*, ARGS... args);
    private:
        std::unique_ptr<cpp_type>             m_CPPFunctor = std::make_unique<cpp_type>([](const OWNER*, ARGS... args) {});
        std::unique_ptr<luabridge::LuaRef>    m_LUAFunctor;

        void internal_delete() noexcept {
            m_CPPFunctor.reset();
            m_LUAFunctor.reset();
        }
        void internal_clone_from(const LuaCallableFunction& other) {
            if (other.m_LUAFunctor) {
                m_LUAFunctor = std::make_unique<luabridge::LuaRef>(*other.m_LUAFunctor);
            }
            if (other.m_CPPFunctor) {
                m_CPPFunctor = std::make_unique<cpp_type>(*other.m_CPPFunctor);
            }
        }
    public:
        LuaCallableFunction() = default;
        LuaCallableFunction(const LuaCallableFunction& other) {
            internal_clone_from(other);
        }
        LuaCallableFunction& operator=(const LuaCallableFunction& other) {
            if (&other != this) {
                internal_clone_from(other);
            }
            return *this;
        }
        LuaCallableFunction(LuaCallableFunction&& other) noexcept = default;
        LuaCallableFunction& operator=(LuaCallableFunction&& other) noexcept = default;

        void setFunctor(const cpp_type& functor) noexcept {
            internal_delete();
            m_CPPFunctor = std::make_unique<cpp_type>(functor);
        }
        void setFunctor(cpp_type&& functor) noexcept {
            internal_delete();
            m_CPPFunctor = std::make_unique<cpp_type>(std::move(functor));
        }
        void setFunctor(luabridge::LuaRef& luaFunction) noexcept {
            if (!luaFunction.isNil() && luaFunction.isFunction()) {
                internal_delete();
                m_LUAFunctor = std::make_unique<luabridge::LuaRef>(luaFunction);
            }
        }

        #ifdef ENGINE_PRODUCTION
            void operator()(const OWNER* owner, ARGS... args) const noexcept {
                if (m_CPPFunctor)
                    (*m_CPPFunctor)(owner, (args)...);
                else
                    (*m_LUAFunctor)(owner, (args)...);
            }
        #else
            void operator()(const OWNER* owner, ARGS... args) const {
                if (m_CPPFunctor) {
                    (*m_CPPFunctor)(owner, (args)...);
                }else{
                    try {
                        (*m_LUAFunctor)(owner, (args)...);
                    }catch (const luabridge::LuaException& luaException) {
                        ENGINE_PRODUCTION_LOG("LuaCallableFunction::operator() LuaException: " << luaException.what())
                    }
                }
            }
        #endif

};
#endif
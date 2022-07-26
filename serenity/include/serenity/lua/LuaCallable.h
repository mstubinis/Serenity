#pragma once
#ifndef ENGINE_LUA_LUA_CALLABLE_H
#define ENGINE_LUA_LUA_CALLABLE_H

#include <memory>
#include <serenity/system/Macros.h>
#include <LuaBridge/LuaBridge.h>

template <class OWNER, class ... ARGS>
class LuaCallableFunction final {
    using CPPFunctor = void(*)(const OWNER*, ARGS... args);
    private:
        std::unique_ptr<CPPFunctor>           m_CPPFunctor;
        std::unique_ptr<luabridge::LuaRef>    m_LUAFunctor;

        void internal_delete() noexcept {
            m_CPPFunctor.reset(nullptr);
            m_LUAFunctor.reset(nullptr);
        }
        void internal_clone_from(const LuaCallableFunction& other) {
            if (other.m_LUAFunctor) {
                m_LUAFunctor = std::make_unique<luabridge::LuaRef>(*other.m_LUAFunctor);
            }
            if (other.m_CPPFunctor) {
                m_CPPFunctor = std::make_unique<CPPFunctor>(*other.m_CPPFunctor);
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
        LuaCallableFunction(LuaCallableFunction&& other) noexcept
            : m_CPPFunctor{ std::exchange(other.m_CPPFunctor, nullptr) }
            , m_LUAFunctor{ std::exchange(other.m_LUAFunctor, nullptr) }
        {}
        LuaCallableFunction& operator=(LuaCallableFunction&& other) noexcept {
            m_CPPFunctor = std::exchange(other.m_CPPFunctor, nullptr);
            m_LUAFunctor = std::exchange(other.m_LUAFunctor, nullptr);
            return *this;
        }

        void setFunctor(const CPPFunctor& functor) noexcept {
            internal_delete();
            m_CPPFunctor = std::make_unique<CPPFunctor>(functor);
        }
        void setFunctor(CPPFunctor&& functor) noexcept {
            internal_delete();
            m_CPPFunctor = std::make_unique<CPPFunctor>(std::move(functor));
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
            else if (m_LUAFunctor)
                (*m_LUAFunctor)(owner, (args)...);
        }
    #else
        void operator()(const OWNER* owner, ARGS... args) const noexcept {
            if (m_CPPFunctor) {
                (*m_CPPFunctor)(owner, (args)...);
            }
            else if (m_LUAFunctor) {
                try {
                    (*m_LUAFunctor)(owner, (args)...);
                } catch (const luabridge::LuaException& luaException) {
                    ENGINE_PRODUCTION_LOG("LuaCallableFunction::operator() LuaException: " << luaException.what())
                } catch (const std::exception& e) {
                    ENGINE_PRODUCTION_LOG("LuaCallableFunction::operator() C++ Exception: " << e.what())
                } catch (...) {
                    ENGINE_PRODUCTION_LOG("LuaCallableFunction::operator() Unknown C++ Exception")
                }
            }
        }
    #endif

};

#endif
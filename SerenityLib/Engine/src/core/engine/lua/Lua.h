#pragma once
#ifndef ENGINE_LUA_H
#define ENGINE_LUA_H

#include <core/engine/lua/LuaModule.h>
#include <core/engine/lua/LuaScript.h>

template <typename OWNER, typename... ARGS> class LuaCallableFunction {
    using cpp_type = std::function<void(const OWNER*, ARGS... args)>;
    private:
        cpp_type*             m_CPPFunctor  = nullptr;
        luabridge::LuaRef*    m_LUAFunctor  = nullptr;

        void internal_delete() {
            SAFE_DELETE(m_CPPFunctor);
            SAFE_DELETE(m_LUAFunctor);
        }
    public:
        LuaCallableFunction(OWNER* owner = nullptr) {
            m_CPPFunctor = NEW cpp_type([](const OWNER*, ARGS... args) {});
        }
        virtual ~LuaCallableFunction() {
            internal_delete();
        }
        LuaCallableFunction(const LuaCallableFunction& other) {
            if (other.m_LUAFunctor) {
                m_LUAFunctor = NEW luabridge::LuaRef(*other.m_LUAFunctor);
            }
            if (other.m_CPPFunctor) {
                m_CPPFunctor = NEW cpp_type(*other.m_CPPFunctor);
            }
        }
        LuaCallableFunction& operator=(const LuaCallableFunction& other) {
            if (&other != this) {
                if (other.m_LUAFunctor) {
                    m_LUAFunctor = NEW luabridge::LuaRef(*other.m_LUAFunctor);
                }
                if (other.m_CPPFunctor) {
                    m_CPPFunctor = NEW cpp_type(*other.m_CPPFunctor);
                }
            }
            return *this;
        }
        LuaCallableFunction(LuaCallableFunction&& other) noexcept {
            m_LUAFunctor  = std::exchange(other.m_LUAFunctor, nullptr);
            m_CPPFunctor  = std::exchange(other.m_CPPFunctor, nullptr);
        }
        LuaCallableFunction& operator=(LuaCallableFunction&& other) noexcept {
            if (&other != this) {
                m_LUAFunctor  = std::exchange(other.m_LUAFunctor, nullptr);
                m_CPPFunctor  = std::exchange(other.m_CPPFunctor, nullptr);
            }
            return *this;
        }
        void setFunctor(cpp_type functor) {
            internal_delete();
            m_CPPFunctor = NEW cpp_type(functor);
        }
        void setFunctor(luabridge::LuaRef& luaFunction) {
            if (!luaFunction.isNil() && luaFunction.isFunction()) {
                internal_delete();
                m_LUAFunctor = NEW luabridge::LuaRef(luaFunction);
            }
        }
        void operator()(const OWNER* owner, ARGS... args) const {
            if (m_CPPFunctor) {
                (*m_CPPFunctor)(owner, (args)...);
            }else{
                //try {
                    (*m_LUAFunctor)(owner, (args)...);
                //}catch (luabridge::LuaException const& e) {
                //    std::cout << "LuaException: " << e.what() << std::endl;
                //}
            }
        }

};
#endif
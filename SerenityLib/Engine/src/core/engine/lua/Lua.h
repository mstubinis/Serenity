#pragma once
#ifndef ENGINE_LUA_H
#define ENGINE_LUA_H

#include <core/engine/lua/LuaModule.h>
#include <core/engine/lua/LuaScript.h>

template <typename T>
class LuaCallableUpdateFunction {
    private:
        std::function<void(const T*, const float)>* m_CPPFunctor  = nullptr;
        luabridge::LuaRef*                          m_LUAFunctor  = nullptr;
    public:
        LuaCallableUpdateFunction(T* owner = nullptr) {
            m_CPPFunctor = NEW std::function([](const T*, const float) {});
        }
        virtual ~LuaCallableUpdateFunction() {
            reset();
        }
        LuaCallableUpdateFunction(const LuaCallableUpdateFunction& other) = delete;
        LuaCallableUpdateFunction& operator=(const LuaCallableUpdateFunction& other) = delete;
        LuaCallableUpdateFunction(LuaCallableUpdateFunction&& other) noexcept {
            m_LUAFunctor  = std::exchange(other.m_LUAFunctor, nullptr);
            m_CPPFunctor  = std::exchange(other.m_CPPFunctor, nullptr);
        }
        LuaCallableUpdateFunction& operator=(LuaCallableUpdateFunction&& other) noexcept {
            if (&other != this) {
                m_LUAFunctor  = std::exchange(other.m_LUAFunctor, nullptr);
                m_CPPFunctor  = std::exchange(other.m_CPPFunctor, nullptr);
            }
            return *this;
        }
        void reset() {
            SAFE_DELETE(m_CPPFunctor);
            SAFE_DELETE(m_LUAFunctor);
        }
        void setFunctor(std::function<void(const T*, const float)> functor) {
            reset();
            m_CPPFunctor = NEW std::function<void(const T*, const float)>(functor);
        }
        void setFunctor(luabridge::LuaRef luaFunction) {
            if (!luaFunction.isNil() && luaFunction.isFunction()) {
                reset();
                m_LUAFunctor = NEW luabridge::LuaRef(luaFunction);
            }
        }
        void call(const T* owner, const float dt) const {
            if (m_CPPFunctor) {
                (*m_CPPFunctor)(owner, dt);
            }else{
                //try {
                (*m_LUAFunctor)(owner, dt);
                //}
                //catch (luabridge::LuaException const& e) {
                //    std::cout << "LuaException: " << e.what() << std::endl;
                //}
            }
        }

};

#endif
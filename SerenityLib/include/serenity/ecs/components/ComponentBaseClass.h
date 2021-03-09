#pragma once
#ifndef ENGINE_ECS_COMPONENT_BASE_CLASS_H
#define ENGINE_ECS_COMPONENT_BASE_CLASS_H

#include <cstdint>
//#include <atomic>

template<class T>
class ComponentBaseClass {
    private:

    public:
        //static inline std::atomic<uint32_t> TYPE_ID = 0;
        static inline uint32_t TYPE_ID = 0;
};

#endif
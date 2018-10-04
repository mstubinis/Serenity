#pragma once
#ifndef ENGINE_ECS_COMPONENT_SYSTEM_H
#define ENGINE_ECS_COMPONENT_SYSTEM_H

#include "ECSComponentPool.h"

namespace Engine {
    namespace epriv {
        class IECSComponentSystem {
            public:
                virtual void update(const float& dt) = 0;
        };
        template<typename T> class ECSComponentSystem : public IECSComponentSystem {
            private:
                ECSComponentPool<T> pool;
            public:
                ECSComponentSystem() {}
                ~ECSComponentSystem() {}
                virtual void update(const float& dt) {}
        };
    };
};
#endif
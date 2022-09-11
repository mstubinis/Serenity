#pragma once
#ifndef ENGINE_ECS_SYSTEM_POOL_H
#define ENGINE_ECS_SYSTEM_POOL_H

class  Scene;
class  SystemBaseClass;

#include <vector>
#include <memory>
#include <cstdint>
#include <serenity/ecs/systems/SystemBaseClass.h>
#include <serenity/ecs/components/ComponentBaseClass.h>
#include <serenity/system/Macros.h>
#include <serenity/utils/Utils.h>

namespace Engine::priv::detail {
    template<class SYSTEM>
    void construct_system(auto& container, auto& ecs, auto&& tuple) {
        auto f = [&ecs, &container](auto&&... args) {
            container[SYSTEM::TYPE_ID] = NEW SYSTEM(ecs, std::forward<decltype(args)>(args)...);
        };
        std::apply(f, std::forward<decltype(tuple)>(tuple));
    }
}
namespace Engine::priv {
    class ECSSystemPool final {
        using SystemComponentHashContainer = std::vector<std::vector<SystemBaseClass*>>;
        using SystemMainContainer          = std::vector<SystemBaseClass*>;
        private:
            struct OrderEntry final {
                int32_t  order;
                uint32_t typeID;
                OrderEntry() noexcept = default;
                OrderEntry(int32_t order_, uint32_t typeID_) noexcept
                    : order { order_ }
                    , typeID { typeID_ }
                {}
                inline bool operator<(const OrderEntry& other) { return order < other.order; }
                inline bool operator>(const OrderEntry& other) { return order > other.order; }
                inline bool operator<=(const OrderEntry& other) { return order <= other.order; }
                inline bool operator>=(const OrderEntry& other) { return order >= other.order; }
                inline bool operator==(const OrderEntry& other) { return order == other.order; }
            };
        using SystemOrderContainer = std::vector<OrderEntry>;
        private:
            static inline uint32_t         m_RegisteredSystems     = 0;
            SystemComponentHashContainer   m_ComponentIDToAssociatedSystems;  //component type_id => systems associated with that component
            SystemMainContainer            m_Systems;                         //main system container, system type_id => system
            SystemOrderContainer           m_Order;                           //Order => TYPE_ID

            template<class COMPONENT>
            void hashSystemImpl(auto* inSystem) {
                ASSERT(COMPONENT::TYPE_ID != std::numeric_limits<uint32_t>().max(), __FUNCTION__ << "(): COMPONENT::TYPE_ID was null, please register this component type! (component class: " << typeid(COMPONENT).name());
                if (std::size(m_ComponentIDToAssociatedSystems) <= COMPONENT::TYPE_ID) {
                    m_ComponentIDToAssociatedSystems.resize(COMPONENT::TYPE_ID + 1);
                }
                m_ComponentIDToAssociatedSystems[COMPONENT::TYPE_ID].push_back(inSystem);
                inSystem->associateComponent<COMPONENT>();
            }
            template<class ... COMPONENTS>
            inline void hashSystem(auto* inSystem) {
                (hashSystemImpl<COMPONENTS>(inSystem), ...);
            }
        public:
            ECSSystemPool() = default;
            ECSSystemPool(const ECSSystemPool&)                = delete;
            ECSSystemPool& operator=(const ECSSystemPool&)     = delete;
            ECSSystemPool(ECSSystemPool&&) noexcept            = delete;
            ECSSystemPool& operator=(ECSSystemPool&&) noexcept = delete;
            ~ECSSystemPool();

            [[nodiscard]] inline size_t getSize() const noexcept { return m_Systems.size(); }

            template<class SYSTEM, class ARG_TUPLE, class ... COMPONENTS>
            SYSTEM* registerSystem(Engine::priv::ECS& ecs, ARG_TUPLE&& argTuple) {
                SYSTEM* createdSystem = nullptr;
                if (SYSTEM::TYPE_ID == std::numeric_limits<uint32_t>().max()) {
                    SYSTEM::TYPE_ID = m_RegisteredSystems++;
                }
                auto threshold = std::max(SYSTEM::TYPE_ID, m_RegisteredSystems);
                if (std::size(m_Systems) <= threshold) {
                    m_Systems.resize(threshold, nullptr);
                }
                SAFE_DELETE(m_Systems[SYSTEM::TYPE_ID]);
                //constructs the system in place by using a tuple as arguments
                Engine::priv::detail::construct_system<SYSTEM>(m_Systems, ecs, std::forward<decltype(argTuple)>(argTuple));
                m_Order.emplace_back(((SYSTEM::TYPE_ID) * 1000) + 1000, SYSTEM::TYPE_ID);
                createdSystem = static_cast<SYSTEM*>(m_Systems[SYSTEM::TYPE_ID]);
                hashSystem<COMPONENTS...>(createdSystem);
                return createdSystem;
            }
            template<class SYSTEM, class ARG_TUPLE, class ... COMPONENTS>
            SYSTEM* registerSystemOrdered(int32_t order, Engine::priv::ECS& ecs, ARG_TUPLE&& argTuple) {
                SYSTEM* createdSystem = nullptr;
                if (SYSTEM::TYPE_ID == std::numeric_limits<uint32_t>().max()) {
                    SYSTEM::TYPE_ID = m_RegisteredSystems++;
                }
                auto threshold = std::max(SYSTEM::TYPE_ID, m_RegisteredSystems);
                if (std::size(m_Systems) <= threshold) {
                    m_Systems.resize(threshold, nullptr);
                }
                SAFE_DELETE(m_Systems[SYSTEM::TYPE_ID]);
                //constructs the system in place by using a tuple as arguments
                Engine::priv::detail::construct_system<SYSTEM>(m_Systems, ecs, std::forward<decltype(argTuple)>(argTuple));
                m_Order.emplace_back(order, SYSTEM::TYPE_ID );

                //auto comp = [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; };
                //std::sort(std::begin(m_Order), std::end(m_Order), comp);
                Engine::insertion_sort(m_Order);

                createdSystem = static_cast<SYSTEM*>(m_Systems[SYSTEM::TYPE_ID]);
                hashSystem<COMPONENTS...>(createdSystem);
                return createdSystem;
            }





            void sort();
            void update(const float dt, Scene&);
            void onComponentAddedToEntity(uint32_t componentTypeID, void* component, Entity);
            void onComponentRemovedFromEntity(uint32_t componentTypeID, Entity);
            void onComponentRemovedFromEntity(Entity);
            void onSceneEntered(Scene&) noexcept;
            void onSceneLeft(Scene&) noexcept;

            inline SystemBaseClass& operator[](const uint32_t idx) noexcept { return *m_Systems[idx]; }
            inline const SystemBaseClass& operator[](const uint32_t idx) const noexcept { return *m_Systems[idx]; }

            //(SystemBaseClass*, int32_t&) should be the function arguments
            template<class FUNC>
            void forEachOrdered(FUNC&& func) {
                for (auto& order : m_Order) {
                    func(m_Systems[order.typeID], order.order);
                }
            }

            BUILD_BEGIN_END_ITR_CLASS_MEMBERS(SystemMainContainer, m_Systems)
    };
}

#endif
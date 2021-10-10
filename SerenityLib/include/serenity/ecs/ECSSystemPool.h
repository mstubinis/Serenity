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

namespace Engine::priv {
    class ECSSystemPool final {
        using SystemComponentHashContainer = std::vector<std::vector<SystemBaseClass*>>;
        using SystemMainContainer          = std::vector<SystemBaseClass*>;
        private:
            struct OrderEntry final {
                uint32_t order;
                uint32_t typeID;
                OrderEntry() = default;
                OrderEntry(uint32_t order_, uint32_t typeID_)
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

            template<class SYSTEM, class COMPONENT>
            void hashSystemImpl(SYSTEM* inSystem) {
                ASSERT(COMPONENT::TYPE_ID != std::numeric_limits<uint32_t>().max(), __FUNCTION__ << "(): COMPONENT::TYPE_ID was null, please register this component type! (component class: " << typeid(COMPONENT).name());
                if (m_ComponentIDToAssociatedSystems.size() <= COMPONENT::TYPE_ID) {
                    m_ComponentIDToAssociatedSystems.resize(COMPONENT::TYPE_ID + 1);
                }
                m_ComponentIDToAssociatedSystems[COMPONENT::TYPE_ID].push_back(inSystem);
                inSystem->associateComponent<COMPONENT>();
            }
            template<class SYSTEM, class ... COMPONENTS>
            inline void hashSystem(SYSTEM* inSystem) {
                (hashSystemImpl<SYSTEM, COMPONENTS>(inSystem), ...);
            }
        public:
            ECSSystemPool() = default;
            ECSSystemPool(const ECSSystemPool&)                = delete;
            ECSSystemPool& operator=(const ECSSystemPool&)     = delete;
            ECSSystemPool(ECSSystemPool&&) noexcept            = delete;
            ECSSystemPool& operator=(ECSSystemPool&&) noexcept = delete;
            ~ECSSystemPool();

            template<class SYSTEM, class ARG_TUPLE, class ... COMPONENTS>
            SYSTEM* registerSystem(Engine::priv::ECS& ecs, ARG_TUPLE&& argTuple) {
                SYSTEM* createdSystem = nullptr;
                if (SYSTEM::TYPE_ID == std::numeric_limits<uint32_t>().max()) {
                    SYSTEM::TYPE_ID = m_RegisteredSystems++;
                }
                auto threshold = std::max(SYSTEM::TYPE_ID, m_RegisteredSystems);
                if (m_Systems.size() <= threshold) {
                    m_Systems.resize(threshold, nullptr);
                }
                auto tupleCat = std::tuple_cat(std::tie(ecs), std::forward<ARG_TUPLE>(argTuple));
                auto sys = std::make_from_tuple<SYSTEM>(tupleCat);
                SAFE_DELETE(m_Systems[SYSTEM::TYPE_ID]);
                m_Systems[SYSTEM::TYPE_ID] = NEW SYSTEM(sys);
                m_Order.emplace_back(((SYSTEM::TYPE_ID) * 1000) + 1000, SYSTEM::TYPE_ID);

                createdSystem = static_cast<SYSTEM*>(m_Systems[SYSTEM::TYPE_ID]);
                hashSystem<SYSTEM, COMPONENTS...>(createdSystem);
                return createdSystem;
            }
            template<class SYSTEM, class ARG_TUPLE, class ... COMPONENTS>
            SYSTEM* registerSystemOrdered(uint32_t order, Engine::priv::ECS& ecs, ARG_TUPLE&& argTuple) {
                SYSTEM* createdSystem = nullptr;
                if (SYSTEM::TYPE_ID == std::numeric_limits<uint32_t>().max()) {
                    SYSTEM::TYPE_ID = m_RegisteredSystems++;
                }
                auto threshold = std::max(SYSTEM::TYPE_ID, m_RegisteredSystems);
                if (m_Systems.size() <= threshold) {
                    m_Systems.resize(threshold, nullptr);
                }
                auto tupleCat = std::tuple_cat(std::tie(ecs), std::forward<ARG_TUPLE>(argTuple));
                auto sys = std::make_from_tuple<SYSTEM>(tupleCat);
                SAFE_DELETE(m_Systems[SYSTEM::TYPE_ID]);
                m_Systems[SYSTEM::TYPE_ID] = NEW SYSTEM(sys);
                m_Order.emplace_back(order, SYSTEM::TYPE_ID );

                //auto comp = [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; };
                //std::sort(std::begin(m_Order), std::end(m_Order), comp);
                Engine::insertion_sort(m_Order);

                createdSystem = static_cast<SYSTEM*>(m_Systems[SYSTEM::TYPE_ID]);
                hashSystem<SYSTEM, COMPONENTS...>(createdSystem);
                return createdSystem;
            }
            void update(const float dt, Scene&);
            void onComponentAddedToEntity(uint32_t componentTypeID, void* component, Entity);
            void onComponentRemovedFromEntity(uint32_t componentTypeID, Entity);
            void onComponentRemovedFromEntity(Entity);
            void onSceneEntered(Scene&) noexcept;
            void onSceneLeft(Scene&) noexcept;

            inline SystemBaseClass& operator[](const uint32_t idx) noexcept { return *m_Systems[idx]; }
            inline const SystemBaseClass& operator[](const uint32_t idx) const noexcept { return *m_Systems[idx]; }

            BUILD_BEGIN_END_ITR_CLASS_MEMBERS(SystemMainContainer, m_Systems)
    };
}

#endif
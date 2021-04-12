#pragma once
#ifndef ENGINE_ECS_SYSTEM_POOL_H
#define ENGINE_ECS_SYSTEM_POOL_H

class  Scene;
class  SystemBaseClass;

#include <vector>
#include <memory>
#include <cstdint>
#include <serenity/ecs/systems/SystemBaseClass.h>
#include <serenity/system/Macros.h>
#include <serenity/utils/Utils.h>

#include <iostream>

namespace Engine::priv {
    class ECSSystemPool final {
        using SystemComponentHashContainer = std::vector<std::vector<SystemBaseClass*>>;
        using SystemMainContainer          = std::vector<std::unique_ptr<SystemBaseClass>>;
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
            SystemComponentHashContainer   m_ComponentIDToSystems;  //component type_id => systems associated with that component
            SystemMainContainer            m_Systems;               //main system container, system type_id => system
            SystemOrderContainer           m_Order;                 //Order => TYPE_ID

            template<class SYSTEM, class COMPONENT>
            void hashSystemImpl(SYSTEM* inSystem) {
                ASSERT(COMPONENT::TYPE_ID != 0, __FUNCTION__ << "(): COMPONENT::TYPE_ID was 0, please register this component type! (component class: " << typeid(COMPONENT).name());
                if (m_ComponentIDToSystems.size() < COMPONENT::TYPE_ID) {
                    m_ComponentIDToSystems.resize(COMPONENT::TYPE_ID);
                }
                //TODO: assert if system is not already in the container ?
                m_ComponentIDToSystems[COMPONENT::TYPE_ID - 1].push_back(inSystem);
                inSystem->associateComponent<COMPONENT>();
            }

            template<class SYSTEM, class ... COMPONENTS>
            inline void hashSystem(SYSTEM* inSystem) {
                (hashSystemImpl<SYSTEM, COMPONENTS>(inSystem) , ...);
            }
        public:
            ECSSystemPool() = default;
            ECSSystemPool(const ECSSystemPool&)                = delete;
            ECSSystemPool& operator=(const ECSSystemPool&)     = delete;
            ECSSystemPool(ECSSystemPool&&) noexcept            = delete;
            ECSSystemPool& operator=(ECSSystemPool&&) noexcept = delete;

            template<class SYSTEM, class ... COMPONENTS, class ... ARGS>
            SYSTEM* registerSystem(ARGS&&... args) {
                SYSTEM* createdSystem = nullptr;
                if (SYSTEM::TYPE_ID == 0) {
                    SYSTEM::TYPE_ID = ++m_RegisteredSystems;
                }
                auto threshold = std::max(SYSTEM::TYPE_ID, m_RegisteredSystems);
                if (m_Systems.size() < threshold) {
                    m_Systems.resize(threshold);
                    m_Order.resize(threshold, OrderEntry{ std::numeric_limits<uint32_t>().max(), std::numeric_limits<uint32_t>().max() });
                }
                m_Systems[SYSTEM::TYPE_ID - 1].reset( NEW SYSTEM(std::forward<ARGS>(args)...) );
                m_Order[SYSTEM::TYPE_ID - 1] = OrderEntry{ ((SYSTEM::TYPE_ID - 1) * 1000) + 1000, SYSTEM::TYPE_ID };

                createdSystem = static_cast<SYSTEM*>(m_Systems[SYSTEM::TYPE_ID - 1].get());
                hashSystem<SYSTEM, COMPONENTS...>(createdSystem);
                return createdSystem;
            }
            template<class SYSTEM, class ... COMPONENTS, class ... ARGS>
            SYSTEM* registerSystemOrdered(uint32_t order, ARGS&&... args) {
                SYSTEM* createdSystem = nullptr;
                if (SYSTEM::TYPE_ID == 0) {
                    SYSTEM::TYPE_ID = ++m_RegisteredSystems;
                }
                auto threshold = std::max(SYSTEM::TYPE_ID, m_RegisteredSystems);
                if (m_Systems.size() < threshold) {
                    m_Systems.resize(threshold);
                    m_Order.resize(threshold, OrderEntry{ std::numeric_limits<uint32_t>().max(), std::numeric_limits<uint32_t>().max() });
                }
                m_Systems[SYSTEM::TYPE_ID - 1].reset(NEW SYSTEM(std::forward<ARGS>(args)...));
                m_Order[SYSTEM::TYPE_ID - 1] = OrderEntry{ order, SYSTEM::TYPE_ID };

                //auto comp = [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; };
                //std::sort(std::begin(m_Order), std::end(m_Order), comp);
                Engine::insertion_sort(m_Order);

                createdSystem = static_cast<SYSTEM*>(m_Systems[SYSTEM::TYPE_ID - 1].get());
                hashSystem<SYSTEM, COMPONENTS...>(createdSystem);
                return createdSystem;
            }

            void update(const float dt, Scene& scene) {
                for (int i = 0; i < m_Order.size(); ++i) {
                    m_Systems[m_Order[i].typeID - 1]->onUpdate(dt, scene);
                }
            }
            void onComponentAddedToEntity(uint32_t componentTypeID, void* component, Entity entity) {
                if (m_ComponentIDToSystems.size() < componentTypeID) {
                    return;
                }
                for (int i = 0; i < m_ComponentIDToSystems[componentTypeID - 1].size(); ++i) {
                    m_ComponentIDToSystems[componentTypeID - 1][i]->onComponentAddedToEntity(component, entity);
                }
                for (int i = 0; i < m_ComponentIDToSystems[componentTypeID - 1].size(); ++i) {
                    m_ComponentIDToSystems[componentTypeID - 1][i]->addEntity(entity);
                }
            }
            void onComponentRemovedFromEntity(uint32_t componentTypeID, Entity entity) {
                ASSERT(m_ComponentIDToSystems.size() >= componentTypeID, __FUNCTION__ << "(): m_ComponentIDToSystems did not have componentTypeID");
                //if (m_ComponentIDToSystems.size() < componentTypeID) {
                //    return;
                //}
                for (int i = 0; i < m_ComponentIDToSystems[componentTypeID - 1].size(); ++i) {
                    m_ComponentIDToSystems[componentTypeID - 1][i]->removeEntity(entity);
                }
                for (int i = 0; i < m_ComponentIDToSystems[componentTypeID - 1].size(); ++i) {
                    m_ComponentIDToSystems[componentTypeID - 1][i]->onComponentRemovedFromEntity(entity);
                }
            }
            void onComponentRemovedFromEntity(Entity entity) {
                for (int i = 0; i < m_Order.size(); ++i) {
                    m_Systems[m_Order[i].typeID - 1]->removeEntity(entity);
                }
                for (int i = 0; i < m_Order.size(); ++i) {
                    m_Systems[m_Order[i].typeID - 1]->onComponentRemovedFromEntity(entity);
                }
            }

            void onSceneEntered(Scene& scene) noexcept {
                for (int i = 0; i < m_Order.size(); ++i) {
                    m_Systems[m_Order[i].typeID - 1]->onSceneEntered(scene);
                }
            }
            void onSceneLeft(Scene& scene) noexcept {
                for (int i = 0; i < m_Order.size(); ++i) {
                    m_Systems[m_Order[i].typeID - 1]->onSceneLeft(scene);
                }
            }

            inline SystemBaseClass& operator[](const uint32_t idx) noexcept { return *m_Systems[idx].get(); }
            inline const SystemBaseClass& operator[](const uint32_t idx) const noexcept { return *m_Systems[idx].get(); }

            inline SystemMainContainer::iterator begin() noexcept { return m_Systems.begin(); }
            inline SystemMainContainer::iterator end() noexcept { return m_Systems.end(); }
            inline const SystemMainContainer::const_iterator cbegin() const noexcept { return m_Systems.cbegin(); }
            inline const SystemMainContainer::const_iterator cend() const noexcept { return m_Systems.cend(); }
    };
}

#endif
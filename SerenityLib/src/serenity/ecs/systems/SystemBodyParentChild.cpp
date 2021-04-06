#include <serenity/ecs/systems/SystemBodyParentChild.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/ecs/components/ComponentModel.h>

#include <serenity/utils/Utils.h>

SystemBodyParentChild::SystemBodyParentChild(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system = (SystemBodyParentChild&)inSystem;

        // compute local matrices and apply starting world matrices as locals
        system.forEach<SystemBodyParentChild*>([](SystemBodyParentChild* system, Entity entity, ComponentTransform* transform) {
            auto entityIndex  = entity.id() - 1;
            auto& localMatrix = system->m_LocalTransforms[entityIndex];
            auto& worldMatrix = system->m_WorldTransforms[entityIndex];
#if defined(ENGINE_HIGH_PRECISION)
            localMatrix       = glm::translate(transform->m_Position) * glm_mat4(glm::mat4_cast(transform->m_Rotation)) * glm_mat4(glm::scale(transform->m_Scale));
#else
            localMatrix       = glm::translate(transform->m_Position) * glm::mat4_cast(transform->m_Rotation) * glm::scale(transform->m_Scale);
#endif
            worldMatrix       = localMatrix;
        }, &system, SystemExecutionPolicy::ParallelWait);

        // traverse parent child relationships and build the proper world matrices
        system.computeAllParentChildWorldTransforms();

        //finalize bullet rigid body positions by giving them their true world locations and rotations. TODO: move to separate system?
        system.forEach<SystemBodyParentChild*>([](SystemBodyParentChild* system, Entity entity, ComponentTransform* transform) {
            auto rigidBody = entity.getComponent<ComponentRigidBody>();
            if (rigidBody) {
                const auto entityIndex  = entity.id() - 1;
                auto& worldMatrix       = system->m_WorldTransforms[entityIndex];
                const uint32_t parentID = system->m_Parents[entityIndex];
                rigidBody->internal_setPosition(worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2]);
                if (parentID == 0) {
                    rigidBody->internal_setRotation(transform->m_Rotation.x, transform->m_Rotation.y, transform->m_Rotation.z, transform->m_Rotation.w);
                }else{
#if defined(ENGINE_HIGH_PRECISION)
                    auto worldRotation  = glm::quat_cast( system->m_WorldTransforms[parentID - 1] * glm_mat4(glm::mat4_cast(transform->m_Rotation)) );
#else
                    auto worldRotation = glm::quat_cast(system->m_WorldTransforms[parentID - 1] * glm::mat4_cast(transform->m_Rotation));
#endif
                    rigidBody->internal_setRotation(worldRotation.x, worldRotation.y, worldRotation.z, worldRotation.w);
                }
            }
        }, &system, SystemExecutionPolicy::ParallelWait);
    });
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        auto& system  = (SystemBodyParentChild&)inSystem;
        const auto id = entity.id();
        if (system.m_Parents.capacity() < id) {
            system.reserve(id + 50);
        }
        if (system.m_Parents.size() < id) {
            system.resize(id);
        }
        auto model = entity.getComponent<ComponentModel>();
        if (model) {
            Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
        }
    });
    setComponentRemovedFromEntityFunction([](SystemBaseClass& inSystem, Entity entity) {
        auto& system         = (SystemBodyParentChild&)inSystem;
        const auto id        = entity.id();
        const auto thisIndex = id - 1;
        if (system.m_Parents[thisIndex] > 0) {
            system.removeChild(system.m_Parents[thisIndex], id);
        }
    });
}
Entity SystemBodyParentChild::getParentEntity(Entity entity) const {
    auto parent = m_Parents[entity.id() - 1];
    if (parent > 0) {
        return getEntity(parent);
    }
    return Entity{};
}
void SystemBodyParentChild::computeAllParentChildWorldTransforms() {
    for (size_t i = 0; i < m_Order.size(); ++i) {
        const uint32_t entityID = m_Order[i];
        if (entityID > 0) {
            const uint32_t entityIndex = entityID - 1U;
            const uint32_t parentID    = m_Parents[entityIndex];
            if (parentID == 0) {
                m_WorldTransforms[entityIndex] = m_LocalTransforms[entityIndex];
            }else{
                const uint32_t parentIndex = parentID - 1U;
                m_WorldTransforms[entityIndex] = m_WorldTransforms[parentIndex] * m_LocalTransforms[entityIndex];
            }
        }else{
            break;
        }
    }
}


#pragma region ParentChildVector

void SystemBodyParentChild::resize(size_t size) {
    m_Parents.resize(size, 0U);
    m_WorldTransforms.resize(size, glm_mat4{ 1.0 });
    m_LocalTransforms.resize(size, glm_mat4{ 1.0 });
}
void SystemBodyParentChild::reserve(size_t size) {
    m_Parents.reserve(size);
    m_WorldTransforms.reserve(size);
    m_LocalTransforms.reserve(size);
}
void SystemBodyParentChild::internal_reserve_from_insert(uint32_t parentID, uint32_t childID) {
    if (m_Parents.capacity() < parentID || m_Parents.capacity() < childID) {
        reserve(std::max(parentID, childID) + 50);
    }
    if (m_Parents.size() < parentID || m_Parents.size() < childID) {
        resize(std::max(parentID, childID));
    }
}
void SystemBodyParentChild::addChild(uint32_t parentID, uint32_t childID) {
    internal_reserve_from_insert(parentID, childID);
    if (getParent(childID) == parentID) {
        return;
    }
    auto parentBlock = getBlockIndices(parentID); //O(order.size())
    auto childBlock  = getBlockIndices(childID);  //O(order.size())

    if (parentBlock.first == NULL_INDEX && childBlock.first == NULL_INDEX) {
        m_Order.push_back(parentID);
        m_Order.push_back(childID);
    }else if (parentBlock.first != NULL_INDEX && childBlock.first == NULL_INDEX) {
        //parent found, not child
        m_Order.insert(std::begin(m_Order) + (parentBlock.first + 1), childID); //TODO: test edge case of parentBlock.first being at [size - 1]
    }else if (parentBlock.first == NULL_INDEX && childBlock.first != NULL_INDEX) {
        //child found, not parent
        m_Order.insert(std::begin(m_Order) + childBlock.first, parentID);
    }else{
        //both found
        uint32_t childBlockSize = (childBlock.second - childBlock.first) + 1;
        if (childBlockSize <= 1) {
            //child is by itself, add child right after parent. is this case ever hit?
            m_Order.erase(std::begin(m_Order) + childBlock.first);
            m_Order.insert(std::begin(m_Order) + (parentBlock.first + 1), childID); //TODO: test edge case of parentBlock.first being at [size - 1]
        }else{
            //child has multiple children itself. insert it at the end of parent's block
            auto temp = Engine::create_and_reserve<std::vector<uint32_t>>(childBlockSize);
            for (uint32_t i = childBlock.first; i <= childBlock.second; ++i) {
                temp.push_back(m_Order[i]);
            }
            m_Order.erase(std::cbegin(m_Order) + childBlock.first, std::cbegin(m_Order) + childBlock.second + 1);
            m_Order.insert(std::cbegin(m_Order) + (parentBlock.second + 1), std::begin(temp), std::end(temp));  //TODO: test edge case of parentBlock.first being at [size - 1]
        }
    }

    getParent(childID) = parentID;
    getWorld(childID)  = getWorld(parentID) * getLocal(childID);
}
void SystemBodyParentChild::removeChild(uint32_t parentID, uint32_t childID) {
    if (getParent(childID) != parentID) {
        return;
    }
    auto parentBlock = getBlockIndices(parentID); //O(order.size())
    if (parentBlock.first == NULL_INDEX) {
        return;
    }
    auto childBlock         = getBlockIndices(childID);  //O(order.size())
    uint32_t childBlockSize = (childBlock.second - childBlock.first) + 1;
    if (childBlockSize == 1) {
        m_Order.erase(std::cbegin(m_Order) + childBlock.first);
    }else{
        auto temp = Engine::create_and_reserve<std::vector<uint32_t>>(childBlockSize);
        for (uint32_t i = childBlock.first; i <= childBlock.second; ++i) {
            temp.push_back(m_Order[i]);
        }
        m_Order.erase(std::cbegin(m_Order) + childBlock.first, std::cbegin(m_Order) + childBlock.second + 1);
        m_Order.insert(std::cend(m_Order), std::begin(temp), std::end(temp));
    }
    if (parentBlock.first == m_Order.size() - 1 || getParent(m_Order[parentBlock.first + 1]) == 0) { //TODO: check for out of bounds possibility
        m_Order.erase(std::cbegin(m_Order) + parentBlock.first);
    }
    getParent(childID) = 0;
}
std::pair<uint32_t, uint32_t> SystemBodyParentChild::getBlockIndices(uint32_t ID) {
    std::pair<uint32_t, uint32_t> ret = std::make_pair(NULL_INDEX, NULL_INDEX);
    //find first index
    for (uint32_t i = 0; i < m_Order.size(); ++i) {
        if (m_Order[i] == ID) {
            ret.first = i;
            break;
        }
    }
    //find second index
    ret.second = m_Order.size() - 1;
    if (m_Order.size() > 0) {
        for (uint32_t i = ret.first; i < m_Order.size() - 1; ++i) {
            const auto nextID     = m_Order[i + 1];
            const auto rootParent = getRootParent(nextID);
            if (rootParent != ID) {
                ret.second = i;
                break;
            }
        }
    }
    return ret;
}
void SystemBodyParentChild::clear_all() {
    m_Parents.clear();
    m_Order.clear();
    m_OrderHead = 0;
}
void SystemBodyParentChild::clear_and_shrink_all() {
    clear_all();
    m_Parents.shrink_to_fit();
    m_Order.shrink_to_fit();
}
#pragma endregion

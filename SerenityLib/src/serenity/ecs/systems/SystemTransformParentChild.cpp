#include <serenity/ecs/systems/SystemTransformParentChild.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/components/ComponentCollisionShape.h>
#include <serenity/ecs/components/ComponentName.h>
#include <serenity/math/Engine_Math.h>

#include <serenity/utils/Utils.h>

SystemTransformParentChild::SystemTransformParentChild(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system = static_cast<SystemTransformParentChild&>(inSystem);
        system.computeAllParentChildWorldTransforms();
        system.forEach<SystemTransformParentChild*>([](SystemTransformParentChild* pcsArg, Entity entity, ComponentTransform* transform) {
            auto rigidBody      = entity.getComponent<ComponentRigidBody>();
            auto collisionShape = entity.getComponent<ComponentCollisionShape>();
            pcsArg->syncRigidToTransform(rigidBody, collisionShape, entity);
        }, &system, SystemExecutionPolicy::ParallelWait);
    });
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        auto& system  = static_cast<SystemTransformParentChild&>(inSystem);
        system.acquireMoreMemory(entity.id());
        auto model = entity.getComponent<ComponentModel>();
        if (model) {
            Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
        }
    });
    setComponentRemovedFromEntityFunction([](SystemBaseClass& inSystem, Entity entity) {
        auto& system         = static_cast<SystemTransformParentChild&>(inSystem);
        const auto id        = entity.id();
        if (system.m_Parents[id] != NULL_IDX) {
            system.removeChild(system.m_Parents[id], id);
        }
    });
}
Entity SystemTransformParentChild::getParentEntity(Entity entity) const {
    auto parent = m_Parents[entity.id()];
    if (parent != NULL_IDX) {
        return getEntity(parent);
    }
    return Entity{};
}
bool SystemTransformParentChild::computeEntityParentChild(uint32_t entityID) {
    if (entityID != NULL_IDX) {
        const uint32_t parentID = m_Parents[entityID];
        if (parentID == NULL_IDX) {
            m_WorldTransforms[entityID] = m_LocalTransforms[entityID];
        } else {
            m_WorldTransforms[entityID] = m_WorldTransforms[parentID] * m_LocalTransforms[entityID];
        }
    } else {
        return false;
    }
    return true;
}
void SystemTransformParentChild::computeParentChildWorldTransforms(uint32_t parent) {
    auto root  = getRootParent(parent);
    auto block = getBlockIndices(root);
    ASSERT(block.first >= 0 && block.second >= 0, __FUNCTION__ << "(): block contains negative indices");
    ASSERT(block.first < m_Order.size() && block.second < m_Order.size(), __FUNCTION__ << "(): block does not fit in m_Order");
    for (size_t i = block.first; i <= block.second; ++i) {
        if (!computeEntityParentChild(m_Order[i])) {
            break;
        }
    }
}
void SystemTransformParentChild::computeAllParentChildWorldTransforms() {
    for (size_t i = 0; i < m_Order.size(); ++i) {
        if (!computeEntityParentChild(m_Order[i])) {
            break;
        }
    }
}
void SystemTransformParentChild::syncRigidToTransform(ComponentRigidBody* rigidBody, ComponentCollisionShape* collisionShape, Entity entity) {
    if (rigidBody) {
        const auto& thisWorldMatrix        = m_WorldTransforms[entity.id()];
        if (collisionShape && collisionShape->isChildShape()) {
            const auto& parentWorldMatrix  = m_WorldTransforms[collisionShape->getParent().id()];
            auto localMatrix               = glm::inverse(parentWorldMatrix) * thisWorldMatrix;
            const auto localScale          = Engine::Math::removeMatrixScale<glm_mat4, glm_vec3>(localMatrix);
            collisionShape->updateChildShapeTransform(localMatrix);
        } else {
            rigidBody->internal_set_matrix(thisWorldMatrix);
        }
    }
}

void SystemTransformParentChild::resize(size_t size) {
    m_Parents.resize(size, NULL_IDX);
    m_WorldTransforms.resize(size, glm_mat4{ 1.0 });
    m_LocalTransforms.resize(size, glm_mat4{ 1.0 });
}
void SystemTransformParentChild::reserve(size_t size) {
    m_Parents.reserve(size);
    m_WorldTransforms.reserve(size);
    m_LocalTransforms.reserve(size);
}
void SystemTransformParentChild::acquireMoreMemory(uint32_t entityID) {
    if (m_Parents.capacity() <= entityID) {
        reserve(entityID + 50);
    }
    if (m_Parents.size() <= entityID) {
        resize(entityID + 1);
    }
}
void SystemTransformParentChild::internal_reserve_from_insert(uint32_t parentID, uint32_t childID) {
    if (m_Parents.capacity() <= parentID || m_Parents.capacity() <= childID) {
        reserve(std::max(parentID, childID) + 50);
    }
    if (m_Parents.size() <= parentID || m_Parents.size() <= childID) {
        resize(std::max(parentID, childID) + 1);
    }
}
uint32_t SystemTransformParentChild::getRootParent(uint32_t childID) noexcept {
    uint32_t parent = m_Parents[childID];
    while (parent != NULL_IDX && getParent(parent) != NULL_IDX) {
        parent = getParent(parent);
    }
    return parent;
}
void SystemTransformParentChild::addChild(uint32_t parentID, uint32_t childID) {
    internal_reserve_from_insert(parentID, childID);
    if (getParent(childID) == parentID) {
        return;
    }
    auto parentBlock = getBlockIndices(parentID); //O(order.size())
    auto childBlock  = getBlockIndices(childID);  //O(order.size())

    if (parentBlock.first == NULL_IDX && childBlock.first == NULL_IDX) {
        m_Order.push_back(parentID);
        m_Order.push_back(childID);
    } else if (parentBlock.first != NULL_IDX && childBlock.first == NULL_IDX) {
        //parent found, not child
        m_Order.insert(std::begin(m_Order) + (parentBlock.first + 1), childID);
    } else if (parentBlock.first == NULL_IDX && childBlock.first != NULL_IDX) {
        //child found, not parent
        m_Order.insert(std::begin(m_Order) + childBlock.first, parentID);
    } else {
        //both found
        uint32_t childBlockSize = (childBlock.second - childBlock.first) + 1;
        if (childBlockSize <= 1) {
            //child is by itself, add child right after parent. is this case ever hit?
            m_Order.erase(std::begin(m_Order) + childBlock.first);
            m_Order.insert(std::begin(m_Order) + (parentBlock.first + 1), childID);
        } else {
            //child has multiple children itself. insert it at the end of parent's block
            auto temp = Engine::create_and_reserve<std::vector<uint32_t>>(childBlockSize);
            for (uint32_t i = childBlock.first; i <= childBlock.second; ++i) {
                temp.push_back(m_Order[i]);
            }
            m_Order.erase(std::cbegin(m_Order) + childBlock.first, std::cbegin(m_Order) + childBlock.second + 1);
            m_Order.insert(std::cbegin(m_Order) + (parentBlock.second + 1), std::begin(temp), std::end(temp));
        }
    }
    getParent(childID) = parentID;
    getWorld(childID)  = getWorld(parentID) * getLocal(childID);
}
void SystemTransformParentChild::removeChild(uint32_t parentID, uint32_t childID) {
    if (getParent(childID) != parentID) {
        return;
    }
    auto parentBlock = getBlockIndices(parentID); //O(order.size())
    if (parentBlock.first == NULL_IDX) {
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
    if (parentBlock.first == m_Order.size() - 1 || getParent(m_Order[parentBlock.first + 1]) == NULL_IDX) { //TODO: check for out of bounds possibility
        m_Order.erase(std::cbegin(m_Order) + parentBlock.first);
    }
    getParent(childID) = NULL_IDX;
}
std::pair<uint32_t, uint32_t> SystemTransformParentChild::getBlockIndices(uint32_t ID) {
    std::pair<uint32_t, uint32_t> ret{ NULL_IDX, uint32_t(m_Order.size() - 1) };
    //find first index
    for (uint32_t i = 0; i < m_Order.size(); ++i) {
        if (m_Order[i] == ID) {
            ret.first = i;
            break;
        }
    }
    //find second index
    if (m_Order.size() > 0) {
        for (uint32_t i = ret.first; i < m_Order.size() - 1; ++i) {
            const auto nextID         = m_Order[i + 1];
            const auto nextRootParent = getRootParent(nextID);
            if (nextRootParent != ID) {
                ret.second = i;
                break;
            }
        }
    }
    return ret;
}
void SystemTransformParentChild::clear_all() {
    m_Parents.clear();
    m_Order.clear();
}
void SystemTransformParentChild::clear_and_shrink_all() {
    clear_all();
    m_Parents.shrink_to_fit();
    m_Order.shrink_to_fit();
}
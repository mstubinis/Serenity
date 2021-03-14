#include <serenity/ecs/systems/SystemBodyParentChild.h>
#include <serenity/ecs/components/ComponentBody.h>
#include <serenity/ecs/components/ComponentBodyRigid.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/ECS.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/ecs/ECSComponentPool.h>
#include <serenity/physics/Collision.h>
#include <serenity/resources/Engine_Resources.h>

SystemBodyParentChild::SystemBodyParentChild(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system = (SystemBodyParentChild&)inSystem;
        system.forEach<SystemBodyParentChild*>([](SystemBodyParentChild* system, Entity entity, ComponentBody* body, ComponentBodyRigid* rigid) {
            const auto entityIndex = entity.id() - 1;
            auto& localMatrix = system->LocalTransforms[entityIndex];
            auto& worldMatrix = system->WorldTransforms[entityIndex];
            if (body) {
                localMatrix = glm::translate(body->m_Position) * glm::mat4_cast(body->m_Rotation) * glm::scale(body->m_Scale);
                worldMatrix = localMatrix;
            }else if(rigid){
                localMatrix = rigid->modelMatrix();
                worldMatrix = localMatrix;
            }
        }, &system, SystemExecutionPolicy::ParallelWait);
        system.computeAllMatrices();
    });
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        auto& system  = (SystemBodyParentChild&)inSystem;
        const auto id = entity.id();
        if (system.Parents.capacity() < id) {
            system.reserve(id + 50);
        }
        if (system.Parents.size() < id) {
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
        if (system.Parents[thisIndex] > 0) {
            system.remove(system.Parents[thisIndex], id);
        }
    });
}

void SystemBodyParentChild::computeAllMatrices() {
    for (size_t i = 0; i < Order.size(); ++i) {
        const uint32_t entityID = Order[i];
        if (entityID > 0) {
            const uint32_t entityIndex = entityID - 1U;
            const uint32_t parentID    = Parents[entityIndex];
            if (parentID == 0) {
                WorldTransforms[entityIndex] = LocalTransforms[entityIndex];
            }else{
                const uint32_t parentIndex = parentID - 1U;
                WorldTransforms[entityIndex] = WorldTransforms[parentIndex] * LocalTransforms[entityIndex];
            }
        }else{
            break;
        }
    }
}


#pragma region ParentChildVector

void SystemBodyParentChild::resize(size_t size) {
    Parents.resize(size, 0U);
    Order.resize(size, 0U);
    WorldTransforms.resize(size, glm_mat4{ 1.0 });
    LocalTransforms.resize(size, glm_mat4{ 1.0 });
}
void SystemBodyParentChild::reserve(size_t size) {
    Parents.reserve(size);
    Order.reserve(size);
    WorldTransforms.reserve(size);
    LocalTransforms.reserve(size);
}
void SystemBodyParentChild::internal_reserve_from_insert(uint32_t parentID, uint32_t childID) {
    if (Parents.capacity() < parentID || Parents.capacity() < childID) {
        reserve(std::max(parentID, childID) + 50);
    }
    if (Parents.size() < parentID || Parents.size() < childID) {
        resize(std::max(parentID, childID));
    }
}
void SystemBodyParentChild::insert(uint32_t parentID, uint32_t childID) {
    internal_reserve_from_insert(parentID, childID);
    if (getParent(childID) == parentID) {
        //ENGINE_PRODUCTION_LOG(parentID << ", " << childID << " - added: already added")
        return;
    }
    //ENGINE_PRODUCTION_LOG(parentID << ", " << childID << " - adding")
    bool added = false;
    for (size_t i = 0; i < Order.size(); ++i) {
        auto entityID = Order[i];
        if (entityID == parentID) {
            //insert after the parent node where the next available spot is
            //the next available spot is either the next zero or the next spot where that spot's parent is zero
            for (size_t j = i + 1; j < Order.size(); ++j) {
                auto entityIDCaseOne = Order[j];
                if (entityIDCaseOne == 0 || Parents[entityIDCaseOne - 1] == 0) {
                    Order.insert(Order.begin() + j, childID);
                    Order.pop_back();
                    ++OrderHead;
                    added = true;
                    goto END_LOOP;
                }
            }
        }else if (entityID == childID) {
            //insert right before the child node
            Order.insert(Order.begin() + i, parentID);
            Order.pop_back();
            ++OrderHead;
            added = true;
            break;
        }else if (entityID == 0) {
            break;
        }
    }
END_LOOP:

    if (!added) {
        /* add both at order head */
        Order[OrderHead] = parentID;
        Order[OrderHead + 1] = childID;
        OrderHead += 2;
    }
    getParent(childID) = parentID;

    getWorld(childID) = getWorld(parentID) * getLocal(childID);
}
void SystemBodyParentChild::remove(uint32_t parentID, uint32_t childID) {
    if (getParent(childID) == 0) {
        //std::cout << parentID << ", " << childID << " - remove: already removed\n";
        return;
    }
    size_t parentIndex = 0;
    size_t erasedIndex = 0;
    bool foundParent = false;

    for (size_t i = 0; i < Order.size(); ++i) {
        auto entityID = Order[i];
        if (entityID == 0) {
            break;
        }
        else if (entityID == parentID) {
            parentIndex = i;
            foundParent = true;
            break;
        }
    }
    if (!foundParent) {
        //ENGINE_PRODUCTION_LOG(parentID << ", " << childID << " - remove: not found")
        return;
    }
    getParent(childID) = 0;
    erasedIndex = parentIndex;
    //ENGINE_PRODUCTION_LOG(parentID << ", " << childID << " - removing")
    for (size_t i = parentIndex; i < Order.size(); ++i) {
        auto entityID = Order[i];
        if (entityID == childID) {
            erasedIndex = i;
            Order[i] = 0;

            //now move all children of parent to be next to parent
            for (size_t j = i + 1; j < Order.size(); ++j) {
                auto entityIDCaseOne = Order[j];
                if (Order[j] == 0) {
                    break;
                }
                if (getParent(entityIDCaseOne) == parentID) {
                    std::swap(Order[j - 1], Order[j]);
                    ++erasedIndex;
                }
                else if (getParent(entityIDCaseOne) == childID && Order[j - 1] == 0) {
                    Order[j - 1U] = childID;
                }
                else if (getParent(entityIDCaseOne) == 0) {
                    break;
                }
            }
            //cleanup / edge cases
            //if (Order[erasedIndex + 1] == 0) {
            if (Order[erasedIndex] == 0) {
                Order.erase(Order.begin() + erasedIndex);
                Order.emplace_back(0);
                --OrderHead;
            }
            if (parentIndex > 0 && Order[parentIndex + 1] == 0) {
                Order[parentIndex] = 0;
                Order.erase(Order.begin() + parentIndex);
                Order.emplace_back(0);
                --OrderHead;
            }
            break;
        }
    }
}

#pragma endregion

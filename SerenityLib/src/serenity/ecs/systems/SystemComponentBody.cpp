#include <serenity/ecs/systems/SystemComponentBody.h>
#include <serenity/ecs/components/ComponentBody.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/ECS.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/ecs/ECSComponentPool.h>
#include <serenity/physics/Collision.h>
#include <serenity/resources/Engine_Resources.h>

SystemComponentBody::SystemComponentBody(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        auto& system     = (SystemComponentBody&)inSystem;
        auto& components = inSystem.getComponentPool<ComponentBody>(0).data();
        auto lamda_update_component = [dt, &system](ComponentBody& b, size_t i, size_t k) {
            const auto entityIndex = b.m_Owner.id() - 1U;
            auto& localMatrix = system.ParentChildSystem.LocalTransforms[entityIndex];
            auto& worldMatrix = system.ParentChildSystem.WorldTransforms[entityIndex];
            if (b.m_Physics) {
                auto& BtRigidBody = *b.p->bullet_rigidBody;
                Engine::Math::recalculateForwardRightUp(BtRigidBody, b.m_Forward, b.m_Right, b.m_Up);
                localMatrix = b.modelMatrix();
                worldMatrix = localMatrix;
            }else{
                auto& n = *b.n;
                n.position += (n.linearVelocity * (decimal)dt);
                localMatrix = glm::translate(n.position) * glm::mat4_cast(n.rotation) * glm::scale(n.scale);
                worldMatrix = localMatrix;
            }
        };
        if (components.size() < 200) {
            for (size_t i = 0; i < components.size(); ++i) {
                lamda_update_component(components[i], i, 0);
            }
        }else{
            Engine::priv::threading::addJobSplitVectored(lamda_update_component, components, true, 0);
        }

        ComponentBody::internal_recalculateAllParentChildMatrices(system);
        /*
      #if defined(_DEBUG) || defined(ENGINE_FORCE_PHYSICS_DEBUG_DRAW)
          for (auto& componentBody : components) {
              Entity entity      = componentBody.getOwner();
              auto bodyRenderPos = componentBody.getPosition();
              auto model         = entity.getComponent<ComponentModel>();
              if (model) {
                  auto world_pos = glm::vec3(componentBody.getPosition());
                  auto world_rot = glm::quat(componentBody.getRotation());
                  auto world_scl = glm::vec3(componentBody.getScale());
                  for (size_t i = 0; i < model->getNumModels(); ++i) {
                      auto& modelInstance = (*model)[i];

                      auto rotation = world_rot * modelInstance.orientation();
                      auto fwd      = glm::normalize(Math::getForward(rotation)) * 0.3f;
                      auto right    = glm::normalize(Math::getRight(rotation)) * 0.3f;
                      auto up       = glm::normalize(Math::getUp(rotation)) * 0.3f;

                      auto& physics = Engine::priv::Core::m_Engine->m_PhysicsModule;
                      physics.debug_draw_line(world_pos, (world_pos+fwd) , 1, 0, 0, 1);
                      physics.debug_draw_line(world_pos, (world_pos+right) , 0, 1, 0, 1);
                      physics.debug_draw_line(world_pos, (world_pos+up) , 0, 0, 1, 1);
                  }
              }
              const auto screenPos = Math::getScreenCoordinates(bodyRenderPos, *scene.getActiveCamera(), false);
              if (screenPos.z > 0) {
                  const std::string text = "ID: " + std::to_string(entity.id());
                  Font::renderTextStatic(text, glm::vec2(screenPos.x, screenPos.y), glm::vec4(1.0f), 0.0f, glm::vec2(0.5f), 0.1f, TextAlignment::Left);
              }
          }
      #endif
          */
    });
    setComponentAddedToEntityFunction([](SystemBaseClass& inSystem, void* component, Entity entity) {
        auto& system  = (SystemComponentBody&)inSystem;
        auto& pcs     = system.ParentChildSystem;
        const auto id = entity.id();
        if (pcs.Parents.capacity() < id) {
            pcs.reserve(id + 50U);
        }
        if (pcs.Parents.size() < id) {
            pcs.resize(id);
        }
        auto model = entity.getComponent<ComponentModel>();
        if (model) {
            Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
        }
    });
    setComponentRemovedFromEntityFunction([](SystemBaseClass& inSystem, Entity entity) {
        auto& system  = (SystemComponentBody&)inSystem;
        const auto id = entity.id();
        auto& pcs     = system.ParentChildSystem;
        const auto thisIndex = id - 1U;
        if (pcs.Parents[thisIndex] > 0) {
            pcs.remove(pcs.Parents[thisIndex], id);
        }
    });
    setEntityAddedToSceneFunction([](SystemBaseClass& inSystem, Scene& scene, Entity entity) {
        auto& pool          = inSystem.getComponentPool<ComponentBody>(0);
        auto* component_ptr = pool.getComponent(entity);
        if (component_ptr) {
            if (component_ptr->hasPhysics()) {
                auto& physicsData = *component_ptr->p;
                component_ptr->setCollision(physicsData.collision->getType(), physicsData.mass);
                auto currentScene = Engine::Resources::getCurrentScene();
                if (currentScene && currentScene == &scene) {
                    component_ptr->addPhysicsToWorld(true);
                }
            }
        }
    });
    setSceneEnteredFunction([](SystemBaseClass& inSystem, Scene& scene) {
        auto& components = inSystem.getComponentPool<ComponentBody>(0).data();
        for (auto& component : components) {
            if (component.hasPhysics()) {
                component.addPhysicsToWorld(true);
            }
        }
    });
    setSceneLeftFunction([](SystemBaseClass& inSystem, Scene& scene) {
        auto& components = inSystem.getComponentPool<ComponentBody>(0).data();
        for (auto& component : components) {
            if (component.hasPhysics()) {
                component.removePhysicsFromWorld(true);
            }
        }
    });
}



#pragma region ParentChildVector

void SystemComponentBody::ParentChildVector::resize(size_t size) {
    Parents.resize(size, 0U);
    Order.resize(size, 0U);
    WorldTransforms.resize(size, glm_mat4{ 1.0 });
    LocalTransforms.resize(size, glm_mat4{ 1.0 });
}
void SystemComponentBody::ParentChildVector::reserve(size_t size) {
    Parents.reserve(size);
    Order.reserve(size);
    WorldTransforms.reserve(size);
    LocalTransforms.reserve(size);
}
void SystemComponentBody::ParentChildVector::internal_reserve_from_insert(uint32_t parentID, uint32_t childID) {
    if (Parents.capacity() < parentID || Parents.capacity() < childID) {
        reserve(std::max(parentID, childID) + 50U);
    }
    if (Parents.size() < parentID || Parents.size() < childID) {
        resize(std::max(parentID, childID));
    }
}
void SystemComponentBody::ParentChildVector::insert(uint32_t parentID, uint32_t childID) {
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
                if (entityIDCaseOne == 0 || Parents[entityIDCaseOne - 1U] == 0) {
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
        Order[OrderHead + 1U] = childID;
        OrderHead += 2;
    }
    getParent(childID) = parentID;

    getWorld(childID) = getWorld(parentID) * getLocal(childID);
}
void SystemComponentBody::ParentChildVector::remove(uint32_t parentID, uint32_t childID) {
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
        }else if (entityID == parentID) {
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
            for (size_t j = i + 1U; j < Order.size(); ++j) {
                auto entityIDCaseOne = Order[j];
                if (Order[j] == 0) {
                    break;
                }
                if (getParent(entityIDCaseOne) == parentID) {
                    std::swap(Order[j - 1U], Order[j]);
                    ++erasedIndex;
                }else if (getParent(entityIDCaseOne) == childID && Order[j - 1U] == 0) {
                    Order[j - 1U] = childID;
                }else if (getParent(entityIDCaseOne) == 0) {
                    break;
                }
            }
            //cleanup / edge cases
            //if (Order[erasedIndex + 1U] == 0) {
            if (Order[erasedIndex] == 0) {
                Order.erase(Order.begin() + erasedIndex);
                Order.emplace_back(0);
                --OrderHead;
            }
            if (parentIndex > 0 && Order[parentIndex + 1U] == 0) {
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

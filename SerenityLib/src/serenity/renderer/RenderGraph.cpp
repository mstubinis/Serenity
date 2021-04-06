
#include <serenity/renderer/RenderGraph.h>
#include <serenity/renderer/pipelines/IRenderingPipeline.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/material/Material.h>
#include <serenity/model/ModelInstance.h>
#include <serenity/scene/Camera.h>
#include <serenity/scene/Viewport.h>
#include <serenity/system/Engine.h>

using namespace Engine;
using namespace Engine::priv;

void RenderGraph::internal_addModelInstanceToPipeline(ModelInstance& inModelInstance) {
    // material node check
    priv::MaterialNode* materialNode  = nullptr;
    priv::MeshNode*     meshNode      = nullptr;
    ModelInstance*      modelInstance = nullptr;
    for (auto& itr : m_MaterialNodes) {
        if (itr.material == inModelInstance.material()) {
            materialNode = &itr;
            // mesh node check
            for (auto& itr1 : materialNode->meshNodes) {
                if (itr1.mesh == inModelInstance.mesh()) {
                    meshNode = &itr1;
                    // instance check
                    for (auto& itr2 : meshNode->instanceNodes) {
                        if (itr2 == &inModelInstance) {
                            modelInstance = itr2;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (!materialNode) {
        materialNode = &m_MaterialNodes.emplace_back(inModelInstance.material());
    }
    if (!meshNode) {
        meshNode = &materialNode->meshNodes.emplace_back(inModelInstance.mesh());
    }
    if (!modelInstance) {
        meshNode->instanceNodes.emplace_back(&inModelInstance);
        m_InstancesTotal.emplace_back(&inModelInstance);
    }
}
void RenderGraph::internal_removeModelInstanceFromPipeline(ModelInstance& inModelInstance) {
    // material node check
    priv::MaterialNode*  materialNode  = nullptr;
    priv::MeshNode*      meshNode      = nullptr;
    ModelInstance*       modelInstance = nullptr;
    for (auto& itr : m_MaterialNodes) {
        if (itr.material == inModelInstance.material()) {
            materialNode = &itr;
            // mesh node check
            for (auto& itr1 : materialNode->meshNodes) {
                if (itr1.mesh == inModelInstance.mesh()) {
                    meshNode = &itr1;
                    // instance check
                    for (auto& itr2 : meshNode->instanceNodes) {
                        if (itr2 == &inModelInstance) {
                            modelInstance = itr2;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (meshNode) {
        if (modelInstance) {
            remove_instance_node(*meshNode, *modelInstance);
        }
        if (meshNode->instanceNodes.size() == 0) {
            remove_mesh_node(*materialNode, *meshNode);
            if (materialNode->meshNodes.size() == 0) {
                remove_material_node(*materialNode);
            }
        }
    }
}
bool RenderGraph::remove_material_node(MaterialNode& materialNode) {
    for (size_t i = 0; i < m_MaterialNodes.size(); ++i) {
        if (&m_MaterialNodes[i] == &materialNode) {
            m_MaterialNodes.erase(m_MaterialNodes.begin() + i);
            return true;
        }
    }
    return false;
}
bool RenderGraph::remove_mesh_node(MaterialNode& materialNode, MeshNode& meshNode) {
    for (size_t i = 0; i < materialNode.meshNodes.size(); ++i) {
        if (&materialNode.meshNodes[i] == &meshNode) {
            materialNode.meshNodes.erase(materialNode.meshNodes.begin() + i);
            return true;
        }
    }
    return false;
}
bool RenderGraph::remove_instance_node(MeshNode& meshNode, ModelInstance& instanceNode) {
    for (size_t i = 0; i < meshNode.instanceNodes.size(); ++i) {
        if (meshNode.instanceNodes[i] == &instanceNode) {
            meshNode.instanceNodes.erase(meshNode.instanceNodes.begin() + i);
        }
    }
    for (size_t i = 0; i < m_InstancesTotal.size(); ++i) {
        if (m_InstancesTotal[i] == &instanceNode) {
            m_InstancesTotal.erase(m_InstancesTotal.begin() + i);
            return true;
        }
    }
    return false;
}

// TODO: correct this
void RenderGraph::sort_bruteforce(Camera& camera, SortingMode sortingMode) {
#ifndef _DEBUG
    glm_vec3 camPos = (camera.getPosition());
    auto lambda_sorter = [sortingMode, &camPos](ModelInstance* lhs, ModelInstance* rhs) {
        auto [lhsBody, lhsModel] = lhs->parent().getComponents<ComponentTransform, ComponentModel>();
        auto [rhsBody, rhsModel] = rhs->parent().getComponents<ComponentTransform, ComponentModel>();

        auto lhsPos    = lhsBody->getPosition();
        auto rhsPos    = rhsBody->getPosition();

        auto leftDir   = glm::normalize(lhsPos - camPos);
        auto rightDir  = glm::normalize(rhsPos - camPos);

        auto leftPos   = lhsPos - (leftDir * (decimal)lhsModel->radius());
        auto rightPos  = rhsPos - (rightDir * (decimal)rhsModel->radius());

        if (sortingMode == SortingMode::FrontToBack)
            return glm::distance2(leftPos, camPos) < glm::distance2(rightPos, camPos);
        else if (sortingMode == SortingMode::BackToFront)
            return glm::distance2(leftPos, camPos) > glm::distance2(rightPos, camPos);
        return false;
    };
    Engine::sort(/*std::execution::par_unseq, */ m_InstancesTotal, lambda_sorter);
#endif
}
void RenderGraph::sort_cheap_bruteforce(Camera& camera, SortingMode sortingMode) {
#ifndef _DEBUG
    glm::vec3 camPos = glm::vec3(camera.getPosition());
    auto lambda_sorter = [sortingMode, &camPos](ModelInstance* lhs, ModelInstance* rhs) {
        auto lhsBody = lhs->parent().getComponent<ComponentTransform>();
        auto rhsBody = rhs->parent().getComponent<ComponentTransform>();

        glm::vec3 lhsPos = glm::vec3(lhsBody->getPosition()) + lhs->position();
        glm::vec3 rhsPos = glm::vec3(rhsBody->getPosition()) + rhs->position();

        if (sortingMode == SortingMode::FrontToBack)
            return glm::distance2(lhsPos, camPos) < glm::distance2(rhsPos, camPos);
        else if (sortingMode == SortingMode::BackToFront)
            return glm::distance2(lhsPos, camPos) > glm::distance2(rhsPos, camPos);
        return false;
    };
    Engine::sort(/*std::execution::par_unseq, */ m_InstancesTotal, lambda_sorter);
#endif
}
void RenderGraph::sort_cheap(Camera& camera, SortingMode sortingMode) {
#ifndef _DEBUG
    glm::vec3 camPos = glm::vec3(camera.getPosition());
    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            auto lambda_sorter = [sortingMode, &camPos](ModelInstance* lhs, ModelInstance* rhs) {
                auto  lhsParent        = lhs->parent();
                auto  rhsParent        = rhs->parent();

                auto lhsBody = lhsParent.getComponent<ComponentTransform>();
                auto rhsBody = rhsParent.getComponent<ComponentTransform>();

                glm::vec3 lhsPos       = glm::vec3(lhsBody->getPosition()) + lhs->position();
                glm::vec3 rhsPos       = glm::vec3(rhsBody->getPosition()) + rhs->position();
                if (sortingMode == SortingMode::FrontToBack)
                    return glm::distance2(lhsPos, camPos) < glm::distance2(rhsPos, camPos);
                else if (sortingMode == SortingMode::BackToFront)
                    return glm::distance2(lhsPos, camPos) > glm::distance2(rhsPos, camPos);
                return false;
            };
            Engine::sort(/*std::execution::par_unseq, */ meshNode.instanceNodes, lambda_sorter);
        }
    }
#endif
}
// TODO: correct this
void RenderGraph::sort(Camera& camera, SortingMode sortingMode) {
#ifndef _DEBUG
    glm_vec3 camPos = (camera.getPosition());
    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            auto& vect = meshNode.instanceNodes;
            auto lambda_sorter = [sortingMode, &camPos](ModelInstance* lhs, ModelInstance* rhs) {
                auto [lhsBody, lhsModel] = lhs->parent().getComponents<ComponentTransform, ComponentModel>();
                auto [rhsBody, rhsModel] = rhs->parent().getComponents<ComponentTransform, ComponentModel>();

                auto lhsPos     = lhsBody->getPosition();
                auto rhsPos     = rhsBody->getPosition();

                auto leftDir    = glm::normalize(lhsPos - camPos);
                auto rightDir   = glm::normalize(rhsPos - camPos);

                auto leftPos    = lhsPos - (leftDir * (decimal)lhsModel->radius());
                auto rightPos   = rhsPos - (rightDir * (decimal)rhsModel->radius());

                if (sortingMode == SortingMode::FrontToBack)
                    return glm::distance2(leftPos, camPos) < glm::distance2(rightPos, camPos);
                else if (sortingMode == SortingMode::BackToFront)
                    return glm::distance2(leftPos, camPos) > glm::distance2(rightPos, camPos);
                return false;
            };
            Engine::sort(/*std::execution::par_unseq, */ vect, lambda_sorter);
        }
    }
#endif
}
void RenderGraph::clean(Entity inEntity) {
    if (inEntity.null()) {
        return;
    }
    auto kept_nodes_total = Engine::create_and_reserve<std::vector<ModelInstance*>>((uint32_t)m_InstancesTotal.size());
    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            auto kept_nodes = Engine::create_and_reserve<std::vector<ModelInstance*>>((uint32_t)meshNode.instanceNodes.size());
            for (auto& modelInstance : meshNode.instanceNodes) {
                auto entity = modelInstance->parent();
                if (entity != inEntity) {
                    kept_nodes.push_back(modelInstance);
                    kept_nodes_total.push_back(modelInstance);
                }
            }
            meshNode.instanceNodes.clear();
            std::move(std::begin(kept_nodes), std::end(kept_nodes), std::back_inserter(meshNode.instanceNodes));
        }
    }
    m_InstancesTotal.clear();
    std::move(std::begin(kept_nodes_total), std::end(kept_nodes_total), std::back_inserter(m_InstancesTotal));
}
void RenderGraph::validate_model_instances_for_rendering(Viewport& viewport, Camera& camera) {
    decimal global_distance_factor = ModelInstance::getGlobalDistanceFactor();
    auto lambda = [&](std::vector<ModelInstance*>& inInstanceNodes, const glm_vec3& camPos) {
        for (auto& modelInstancePtr : inInstanceNodes) {
            auto& modelInstance    = *modelInstancePtr;
            auto body              = modelInstance.parent().getComponent<ComponentTransform>();
            auto model             = modelInstance.parent().getComponent<ComponentModel>();
            bool is_valid_viewport = PublicModelInstance::IsViewportValid(modelInstance, viewport);
            if (is_valid_viewport) {
                if (body) {
                    if (modelInstance.isForceRendered()) {
                        modelInstance.setPassedRenderCheck(modelInstance.visible());
                    }else{
                        float radius            = model->radius();
                        glm_vec3 worldPosition  = body->getWorldPosition() + glm_vec3(modelInstance.position());
                        uint32_t sphereTest     = camera.sphereIntersectTest(worldPosition, radius); //per mesh instance radius instead?
                        decimal comparison      = (decimal)radius * global_distance_factor;
                        auto comparison2        = comparison * comparison;

                        bool failedVisibleTest  = !modelInstance.visible();
                        bool failedSphereTest   = (sphereTest == 0);
                        bool failedDistanceTest = glm::distance2(worldPosition, camPos) > comparison2;
                        bool result             = !(failedVisibleTest || failedSphereTest || failedDistanceTest);
                        modelInstance.setPassedRenderCheck(result);
                    }
                }else{
                    modelInstance.setPassedRenderCheck(false);
                }
            }else{
                modelInstance.setPassedRenderCheck(false);
            }
        }
    };
    lambda(m_InstancesTotal, camera.getPosition());
}
void RenderGraph::render(Engine::priv::RenderModule& renderer, Viewport& viewport, Camera& camera, bool useDefaultShaders, SortingMode sortingMode) {
    auto shaderProgram = m_ShaderProgram.get<ShaderProgram>();
    if (useDefaultShaders) {
        renderer.bind(shaderProgram);
    }
    for (auto& materialNode : m_MaterialNodes) {
        if (materialNode.meshNodes.size() > 0) {
            auto material = materialNode.material.get<Material>();
            renderer.bind(material);
            for (auto& meshNode : materialNode.meshNodes) {
                if (meshNode.instanceNodes.size() > 0) {
                    auto mesh = meshNode.mesh.get<Mesh>();
                    renderer.bind(mesh);
                    for (auto& modelInstance : meshNode.instanceNodes) {
                        auto transform       = modelInstance->parent().getComponent<ComponentTransform>();
                        auto renderingMatrix = transform->getWorldMatrixRendering();
                        if (modelInstance->passedRenderCheck()) {
                            if (sortingMode != SortingMode::None) {
                                mesh->sortTriangles(camera, *modelInstance, renderingMatrix, sortingMode);
                            }
                            renderer.bind(modelInstance);
                            renderer.m_Pipeline->renderMesh(*mesh, (uint32_t)modelInstance->getDrawingMode());
                            renderer.unbind(modelInstance);
                        }
                    }
                    // protect against any custom changes by restoring to the regular shader and material
                    if (useDefaultShaders) {
                        if (renderer.m_Pipeline->getCurrentBoundShaderProgram() != shaderProgram) {
                            renderer.bind(shaderProgram);
                            renderer.bind(material);
                        }
                    }
                    renderer.unbind(mesh);
                }
            }
            renderer.unbind(material);
        }
    }
}
void RenderGraph::render_bruteforce(Engine::priv::RenderModule& renderer, Viewport& viewport, Camera& camera, bool useDefaultShaders, SortingMode sortingMode) {
    auto shaderProgram = m_ShaderProgram.get<ShaderProgram>();
    if (useDefaultShaders) {
        renderer.bind(shaderProgram);
    }
    for (auto& modelInstance : m_InstancesTotal) {
        auto mesh            = modelInstance->mesh().get<Mesh>();
        auto material        = modelInstance->material().get<Material>();
        auto transform       = modelInstance->parent().getComponent<ComponentTransform>();
        auto renderingMatrix = transform->getWorldMatrixRendering();
        if (modelInstance->passedRenderCheck()) {
            if (sortingMode != SortingMode::None) {
                mesh->sortTriangles(camera, *modelInstance, renderingMatrix, sortingMode);
            }
            renderer.bind(material);
            renderer.bind(mesh);
            renderer.bind(modelInstance);

            renderer.m_Pipeline->renderMesh(*mesh, (uint32_t)modelInstance->getDrawingMode());

            renderer.unbind(modelInstance);
            renderer.unbind(mesh);
            renderer.unbind(material);
        }
        // protect against any custom changes by restoring to the regular shader and material
        if (useDefaultShaders) {
            if (renderer.m_Pipeline->getCurrentBoundShaderProgram() != shaderProgram) {
                renderer.bind(shaderProgram);
                renderer.bind(material);
            }
        }
    }
}

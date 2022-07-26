
#include <serenity/renderer/RenderGraph.h>
#include <serenity/renderer/pipelines/IRenderingPipeline.h>
#include <serenity/model/ModelInstance.h>
#include <serenity/scene/Camera.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/material/Material.h>
#include <serenity/renderer/culling/Culling.h>

namespace {
    std::vector<ModelInstance*> KEPT_NODES_TOTAL_BUFFER(10, nullptr);
    std::vector<ModelInstance*> KEPT_NODES_PER_INSTANCE_BUFFER(4, nullptr);

    template<class CONTAINER, class ... ARGS>
    inline auto internal_insert_into_container(CONTAINER& container, ARGS&&... args) {
        return &container.emplace_back(std::forward<ARGS>(args)...);
    }
}


void Engine::priv::RenderGraph::internal_scan_nodes(const ModelInstance& inInstance, Engine::priv::MaterialNode*& materialNode, Engine::priv::MeshNode*& meshNode, ModelInstance*& modelInstance) {
    for (auto& itrMaterial : m_MaterialNodes) {
        if (itrMaterial.materialHandle == inInstance.getMaterial()) {
            materialNode = &itrMaterial;
            for (auto& itrMesh : materialNode->meshNodes) {
                if (itrMesh.meshHandle == inInstance.getMesh()) {
                    meshNode = &itrMesh;
                    for (ModelInstance* itrInstance : meshNode->instanceNodes) {
                        if (itrInstance == &inInstance) {
                            modelInstance = itrInstance;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
}
void Engine::priv::RenderGraph::internal_addModelInstanceToPipeline(ModelInstance& inModelInstance) {
    Engine::priv::MaterialNode* materialNode  = nullptr;
    Engine::priv::MeshNode*     meshNode      = nullptr;
    ModelInstance*              modelInstance = nullptr;

    internal_scan_nodes(inModelInstance, materialNode, meshNode, modelInstance);
    if (!materialNode) {
        materialNode = internal_insert_into_container(m_MaterialNodes, inModelInstance.getMaterial());
    }
    if (!meshNode) {
        meshNode = internal_insert_into_container(materialNode->meshNodes, inModelInstance.getMesh());
    }
    if (!modelInstance) {
        internal_insert_into_container(meshNode->instanceNodes, &inModelInstance);
        internal_insert_into_container(m_InstancesTotal, &inModelInstance);
    }
}
void Engine::priv::RenderGraph::internal_removeModelInstanceFromPipeline(ModelInstance& inModelInstance) {
    Engine::priv::MaterialNode*  materialNode  = nullptr;
    Engine::priv::MeshNode*      meshNode      = nullptr;
    ModelInstance*               modelInstance = nullptr;

    internal_scan_nodes(inModelInstance, materialNode, meshNode, modelInstance);

    remove_instance_node(meshNode, modelInstance);
    remove_mesh_node(materialNode, meshNode);
    remove_material_node(materialNode);
}
bool Engine::priv::RenderGraph::remove_material_node(const MaterialNode* materialNode) {
    if (materialNode && materialNode->meshNodes.size() == 0) { //only remove if we have no more mesh nodes
        size_t removedMaterialNodes = Engine::swap_and_pop_single(m_MaterialNodes, [](auto& item, const MaterialNode* inMaterialNode) {
            return &item == inMaterialNode;
        }, materialNode);
        return removedMaterialNodes > 0;
    }
    return false;
}
bool Engine::priv::RenderGraph::remove_mesh_node(MaterialNode* materialNode, const MeshNode* meshNode) {
    if (materialNode && meshNode && meshNode->instanceNodes.size() == 0) { //only remove if we have no more instance nodes
        size_t removedMeshNodes = Engine::swap_and_pop_single(materialNode->meshNodes, [](auto& item, const MeshNode* inMeshNode) {
            return &item == inMeshNode;
        }, meshNode);
        return removedMeshNodes > 0;
    }
    return false;
}
bool Engine::priv::RenderGraph::remove_instance_node(MeshNode* meshNode, const ModelInstance* instanceNode) {
    if (meshNode && instanceNode) {
        size_t removedInstances = Engine::swap_and_pop_single(meshNode->instanceNodes, [](auto& item, const ModelInstance* inInstanceNode) {
            return item == inInstanceNode;
        }, instanceNode);
        size_t removedInstancesTotal = Engine::swap_and_pop_single(m_InstancesTotal, [](auto& item, const ModelInstance* inInstanceNode) {
            return item == inInstanceNode;
        }, instanceNode);
        return removedInstances > 0 && removedInstancesTotal > 0;
    }
    return false;
}
// TODO: correct this
void Engine::priv::RenderGraph::internal_sort_impl(Camera* camera, SortingMode sortingMode, std::vector<ModelInstance*>& instances) {
    if (camera) {
#ifndef _DEBUG
        glm_vec3 camPos = camera->getPosition();
        auto lambda_sorter = [sortingMode, &camPos](ModelInstance* lhs, ModelInstance* rhs) {
            auto [lhsTransform, lhsModel] = lhs->getOwner().getComponents<ComponentTransform, ComponentModel>();
            auto [rhsTransform, rhsModel] = rhs->getOwner().getComponents<ComponentTransform, ComponentModel>();
            auto lhsPos                   = lhsTransform->getPosition();
            auto rhsPos                   = rhsTransform->getPosition();
            auto leftDir                  = glm::normalize(lhsPos - camPos);
            auto rightDir                 = glm::normalize(rhsPos - camPos);
            auto leftPos                  = lhsPos - (leftDir * (decimal)lhsModel->getRadius());
            auto rightPos                 = rhsPos - (rightDir * (decimal)rhsModel->getRadius());
            if (sortingMode == SortingMode::FrontToBack)
                return glm::distance2(leftPos, camPos) < glm::distance2(rightPos, camPos);
            else if (sortingMode == SortingMode::BackToFront)
                return glm::distance2(leftPos, camPos) > glm::distance2(rightPos, camPos);
            return false;
        };
        Engine::sort(std::execution::par_unseq, instances, lambda_sorter);
#endif
    }
}
void Engine::priv::RenderGraph::internal_sort_cheap_impl(Camera* camera, SortingMode sortingMode, std::vector<ModelInstance*>& instances) {
    if (camera) {
#ifndef _DEBUG
        glm::vec3 camPos = glm::vec3{ camera->getPosition() };
        auto lambda_sorter = [sortingMode, &camPos](ModelInstance* lhs, ModelInstance* rhs) {
            auto lhsTransform = lhs->getOwner().getComponent<ComponentTransform>();
            auto rhsTransform = rhs->getOwner().getComponent<ComponentTransform>();
            auto lhsPos       = glm::vec3(lhsTransform->getPosition()) + lhs->getPosition();
            auto rhsPos       = glm::vec3(rhsTransform->getPosition()) + rhs->getPosition();
            if (sortingMode == SortingMode::FrontToBack)
                return glm::distance2(lhsPos, camPos) < glm::distance2(rhsPos, camPos);
            else if (sortingMode == SortingMode::BackToFront)
                return glm::distance2(lhsPos, camPos) > glm::distance2(rhsPos, camPos);
            return false;
        };
        Engine::sort(std::execution::par_unseq, instances, lambda_sorter);
#endif
    }
}
void Engine::priv::RenderGraph::sort_cheap(Camera* camera, SortingMode sortingMode) {
    if (camera) {
#ifndef _DEBUG
        for (auto& materialNode : m_MaterialNodes) {
            for (auto& meshNode : materialNode.meshNodes) {
                internal_sort_cheap_impl(camera, sortingMode, meshNode.instanceNodes);
            }
        }
#endif
    }
}
void Engine::priv::RenderGraph::sort_cheap_bruteforce(Camera* camera, SortingMode sortingMode) {
    internal_sort_cheap_impl(camera ,sortingMode, m_InstancesTotal);
}
// TODO: make sure the math here is correct
void Engine::priv::RenderGraph::sort(Camera* camera, SortingMode sortingMode) {
    if (camera) {
#ifndef _DEBUG
        for (auto& materialNode : m_MaterialNodes) {
            for (auto& meshNode : materialNode.meshNodes) {
                internal_sort_impl(camera, sortingMode, meshNode.instanceNodes);
            }
        }
#endif
    }
}
void Engine::priv::RenderGraph::sort_bruteforce(Camera* camera, SortingMode sortingMode) {
    internal_sort_impl(camera, sortingMode, m_InstancesTotal);
}
void Engine::priv::RenderGraph::clean(Entity inEntity) {
    if (!inEntity) {
        return;
    }
    KEPT_NODES_TOTAL_BUFFER.clear();
    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            KEPT_NODES_PER_INSTANCE_BUFFER.clear();
            for (auto& modelInstance : meshNode.instanceNodes) {
                auto entity = modelInstance->getOwner();
                if (entity != inEntity) {
                    KEPT_NODES_PER_INSTANCE_BUFFER.push_back(modelInstance);
                    KEPT_NODES_TOTAL_BUFFER.push_back(modelInstance);
                }
            }
            meshNode.instanceNodes.clear();
            meshNode.instanceNodes.insert(std::end(meshNode.instanceNodes),
                std::make_move_iterator(std::begin(KEPT_NODES_PER_INSTANCE_BUFFER)),
                std::make_move_iterator(std::end(KEPT_NODES_PER_INSTANCE_BUFFER))
            );
        }
    }
    m_InstancesTotal.clear();
    m_InstancesTotal.insert(std::end(m_InstancesTotal),
        std::make_move_iterator(std::begin(KEPT_NODES_TOTAL_BUFFER)),
        std::make_move_iterator(std::end(KEPT_NODES_TOTAL_BUFFER))
    );
}
void Engine::priv::RenderGraph::validate_model_instances_for_rendering(Camera* camera, Viewport* viewport) {
    Engine::priv::Culling::cull(camera, viewport, m_InstancesTotal);
}
void Engine::priv::RenderGraph::validate_model_instances_for_rendering(const glm::mat4& viewProj, Viewport* viewport) {
    Engine::priv::Culling::cull(viewProj, viewport, m_InstancesTotal);
}
void Engine::priv::RenderGraph::render(Engine::priv::RenderModule& renderer, Camera* camera, bool useDefaultShaders, SortingMode sortingMode) {
    auto shaderProgram = m_ShaderProgram.get<ShaderProgram>();
    if (useDefaultShaders) {
        renderer.bind(shaderProgram);
    }
    for (auto& materialNode : m_MaterialNodes) {
        if (materialNode.meshNodes.size() > 0) {
            auto material = materialNode.materialHandle.get<Material>();
            renderer.bind(material);
            for (auto& meshNode : materialNode.meshNodes) {
                if (meshNode.instanceNodes.size() > 0) {
                    auto mesh = meshNode.meshHandle.get<Mesh>();
                    renderer.bind(mesh);
                    for (auto& modelInstance : meshNode.instanceNodes) {
                        auto transform       = modelInstance->getOwner().getComponent<ComponentTransform>();
                        auto renderingMatrix = transform->getWorldMatrixRendering();
                        if (modelInstance->hasPassedRenderCheck()) {
                            if (sortingMode != SortingMode::None) {
                                if (camera) {
                                    mesh->sortTriangles(*camera, *modelInstance, renderingMatrix, sortingMode);
                                }
                            }
                            renderer.bind(modelInstance);
                            renderer.m_Pipeline->renderMesh(*mesh, modelInstance->getDrawingMode());
                            renderer.unbind(modelInstance);
                        } else {

                        }
                    }
                    // protect against any custom changes by restoring to the regular shader and material
                    if (useDefaultShaders && renderer.m_Pipeline->getCurrentBoundShaderProgram() != shaderProgram) {
                        renderer.bind(shaderProgram);
                        renderer.bind(material);
                    }
                    renderer.unbind(mesh);
                }
            }
            renderer.unbind(material);
        }
    }
}
void Engine::priv::RenderGraph::render_bruteforce(Engine::priv::RenderModule& renderer, Camera* camera, bool useDefaultShaders, SortingMode sortingMode) {
    auto shaderProgram = m_ShaderProgram.get<ShaderProgram>();
    if (useDefaultShaders) {
        renderer.bind(shaderProgram);
    }
    for (auto& modelInstance : m_InstancesTotal) {
        auto mesh            = modelInstance->getMesh().get<Mesh>();
        auto material        = modelInstance->getMaterial().get<Material>();
        auto transform       = modelInstance->getOwner().getComponent<ComponentTransform>();
        auto renderingMatrix = transform->getWorldMatrixRendering();
        if (modelInstance->hasPassedRenderCheck()) {
            if (sortingMode != SortingMode::None) {
                if (camera) {
                    mesh->sortTriangles(*camera, *modelInstance, renderingMatrix, sortingMode);
                }
            }
            renderer.bind(material);
            renderer.bind(mesh);
            renderer.bind(modelInstance);

            renderer.m_Pipeline->renderMesh(*mesh, modelInstance->getDrawingMode());

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
void Engine::priv::RenderGraph::render_shadow_map(Engine::priv::RenderModule& renderer, Camera* camera) {
    for (auto& materialNode : m_MaterialNodes) {
        if (materialNode.meshNodes.size() > 0) {
            for (auto& meshNode : materialNode.meshNodes) {
                if (meshNode.instanceNodes.size() > 0) {
                    auto mesh = meshNode.meshHandle.get<Mesh>();
                    renderer.bind(mesh);
                    for (auto& modelInstance : meshNode.instanceNodes) {
                        if (modelInstance->hasPassedRenderCheck() && modelInstance->isShadowCaster()) {
                            renderer.bind(modelInstance);
                            renderer.m_Pipeline->renderMesh(*mesh, modelInstance->getDrawingMode());
                            renderer.unbind(modelInstance);
                        }
                    }
                    renderer.unbind(mesh);
                }
            }
        }
    }
}
void Engine::priv::RenderGraph::render_bruteforce_shadow_map(Engine::priv::RenderModule& renderer, Camera* camera) {
    for (auto& modelInstance : m_InstancesTotal) {
        auto mesh = modelInstance->getMesh().get<Mesh>();
        if (modelInstance->hasPassedRenderCheck() && modelInstance->isShadowCaster()) {
            renderer.bind(mesh);
            renderer.bind(modelInstance);

            renderer.m_Pipeline->renderMesh(*mesh, modelInstance->getDrawingMode());

            renderer.unbind(modelInstance);
            renderer.unbind(mesh);
        }
    }
}
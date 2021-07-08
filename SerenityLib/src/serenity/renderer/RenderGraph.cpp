
#include <serenity/renderer/RenderGraph.h>
#include <serenity/renderer/pipelines/IRenderingPipeline.h>
#include <serenity/model/ModelInstance.h>
#include <serenity/scene/Camera.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/renderer/culling/Culling.h>

void Engine::priv::RenderGraph::internal_scan_nodes(const ModelInstance& inInstance, Engine::priv::MaterialNode*& materialNode, Engine::priv::MeshNode*& meshNode, ModelInstance*& modelInstance) {
    for (auto& itr : m_MaterialNodes) {
        if (itr.material == inInstance.getMaterial()) {
            materialNode = &itr;
            for (auto& itr1 : materialNode->meshNodes) {
                if (itr1.mesh == inInstance.getMesh()) {
                    meshNode = &itr1;
                    for (auto& itr2 : meshNode->instanceNodes) {
                        if (itr2 == &inInstance) {
                            modelInstance = itr2;
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
        materialNode = &m_MaterialNodes.emplace_back(inModelInstance.getMaterial());
    }
    if (!meshNode) {
        meshNode = &materialNode->meshNodes.emplace_back(inModelInstance.getMesh());
    }
    if (!modelInstance) {
        meshNode->instanceNodes.emplace_back(&inModelInstance);
        m_InstancesTotal.emplace_back(&inModelInstance);
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
    if (!materialNode) {
        return false;
    }
    for (size_t i = 0; i < m_MaterialNodes.size(); ++i) {
        if (&m_MaterialNodes[i] == materialNode) {
            m_MaterialNodes.erase(m_MaterialNodes.begin() + i);
            return true;
        }
    }
    return false;
}
bool Engine::priv::RenderGraph::remove_mesh_node(MaterialNode* materialNode, const MeshNode* meshNode) {
    if (!materialNode || !meshNode) {
        return false;
    }
    for (size_t i = 0; i < materialNode->meshNodes.size(); ++i) {
        if (&materialNode->meshNodes[i] == meshNode) {
            materialNode->meshNodes.erase(materialNode->meshNodes.begin() + i);
            return true;
        }
    }
    return false;
}
bool Engine::priv::RenderGraph::remove_instance_node(MeshNode* meshNode, const ModelInstance* instanceNode) {
    if (!meshNode || !instanceNode) {
        return false;
    }
    for (size_t i = 0; i < meshNode->instanceNodes.size(); ++i) {
        if (meshNode->instanceNodes[i] == instanceNode) {
            meshNode->instanceNodes.erase(meshNode->instanceNodes.begin() + i);
        }
    }
    for (size_t i = 0; i < m_InstancesTotal.size(); ++i) {
        if (m_InstancesTotal[i] == instanceNode) {
            m_InstancesTotal.erase(m_InstancesTotal.begin() + i);
            return true;
        }
    }
    return false;
}
// TODO: correct this
void Engine::priv::RenderGraph::internal_sort_impl(Camera* camera, SortingMode sortingMode, std::vector<ModelInstance*>& instances) {
    if (!camera) {
        return;
    }
#ifndef _DEBUG
    glm_vec3 camPos = camera->getPosition();
    auto lambda_sorter = [sortingMode, &camPos](ModelInstance* lhs, ModelInstance* rhs) {
        auto [lhsTransform, lhsModel] = lhs->getParent().getComponents<ComponentTransform, ComponentModel>();
        auto [rhsTransform, rhsModel] = rhs->getParent().getComponents<ComponentTransform, ComponentModel>();
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
void Engine::priv::RenderGraph::internal_sort_cheap_impl(Camera* camera, SortingMode sortingMode, std::vector<ModelInstance*>& instances) {
    if (!camera) {
        return;
    }
#ifndef _DEBUG
    glm::vec3 camPos = glm::vec3{ camera->getPosition() };
    auto lambda_sorter = [sortingMode, &camPos](ModelInstance* lhs, ModelInstance* rhs) {
        auto lhsTransform = lhs->getParent().getComponent<ComponentTransform>();
        auto rhsTransform = rhs->getParent().getComponent<ComponentTransform>();
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
void Engine::priv::RenderGraph::sort_cheap(Camera* camera, SortingMode sortingMode) {
    if (!camera) {
        return;
    }
#ifndef _DEBUG
    glm::vec3 camPos = glm::vec3(camera->getPosition());
    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            internal_sort_cheap_impl(camera, sortingMode, meshNode.instanceNodes);
        }
    }
#endif
}
void Engine::priv::RenderGraph::sort_cheap_bruteforce(Camera* camera, SortingMode sortingMode) {
    internal_sort_cheap_impl(camera ,sortingMode, m_InstancesTotal);
}
// TODO: make sure the math here is correct
void Engine::priv::RenderGraph::sort(Camera* camera, SortingMode sortingMode) {
    if (!camera) {
        return;
    }
#ifndef _DEBUG
    glm_vec3 camPos = camera->getPosition();
    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            internal_sort_impl(camera, sortingMode, meshNode.instanceNodes);
        }
    }
#endif
}
void Engine::priv::RenderGraph::sort_bruteforce(Camera* camera, SortingMode sortingMode) {
    internal_sort_impl(camera, sortingMode, m_InstancesTotal);
}
void Engine::priv::RenderGraph::clean(Entity inEntity) {
    if (inEntity.null()) {
        return;
    }
    auto kept_nodes_total = Engine::create_and_reserve<std::vector<ModelInstance*>>((uint32_t)m_InstancesTotal.size());
    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            auto kept_nodes = Engine::create_and_reserve<std::vector<ModelInstance*>>((uint32_t)meshNode.instanceNodes.size());
            for (auto& modelInstance : meshNode.instanceNodes) {
                auto entity = modelInstance->getParent();
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
            auto material = materialNode.material.get<Material>();
            renderer.bind(material);
            for (auto& meshNode : materialNode.meshNodes) {
                if (meshNode.instanceNodes.size() > 0) {
                    auto mesh = meshNode.mesh.get<Mesh>();
                    renderer.bind(mesh);
                    for (auto& modelInstance : meshNode.instanceNodes) {
                        auto transform       = modelInstance->getParent().getComponent<ComponentTransform>();
                        auto renderingMatrix = transform->getWorldMatrixRendering();
                        if (modelInstance->hasPassedRenderCheck()) {
                            if (sortingMode != SortingMode::None) {
                                if (camera) {
                                    mesh->sortTriangles(*camera, *modelInstance, renderingMatrix, sortingMode);
                                }
                            }
                            renderer.bind(modelInstance);
                            renderer.m_Pipeline->renderMesh(*mesh, (uint32_t)modelInstance->getDrawingMode());
                            renderer.unbind(modelInstance);
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
        auto transform       = modelInstance->getParent().getComponent<ComponentTransform>();
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
void Engine::priv::RenderGraph::render_shadow_map(Engine::priv::RenderModule& renderer, Camera* camera) {
    for (auto& materialNode : m_MaterialNodes) {
        if (materialNode.meshNodes.size() > 0) {
            for (auto& meshNode : materialNode.meshNodes) {
                if (meshNode.instanceNodes.size() > 0) {
                    auto mesh = meshNode.mesh.get<Mesh>();
                    renderer.bind(mesh);
                    for (auto& modelInstance : meshNode.instanceNodes) {
                        if (modelInstance->hasPassedRenderCheck() && modelInstance->isShadowCaster()) {
                            renderer.bind(modelInstance);
                            renderer.m_Pipeline->renderMesh(*mesh, (uint32_t)modelInstance->getDrawingMode());
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

            renderer.m_Pipeline->renderMesh(*mesh, (uint32_t)modelInstance->getDrawingMode());

            renderer.unbind(modelInstance);
            renderer.unbind(mesh);
        }
    }
}
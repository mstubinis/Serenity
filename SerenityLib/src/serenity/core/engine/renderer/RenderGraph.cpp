
#include <serenity/core/engine/renderer/RenderGraph.h>
#include <serenity/core/engine/renderer/pipelines/IRenderingPipeline.h>
#include <serenity/core/engine/shaders/ShaderProgram.h>
#include <serenity/core/engine/mesh/Mesh.h>
#include <serenity/core/engine/materials/Material.h>
#include <serenity/core/engine/model/ModelInstance.h>
#include <serenity/core/engine/scene/Camera.h>
#include <serenity/core/engine/scene/Viewport.h>
#include <serenity/core/engine/system/Engine.h>

using namespace Engine;
using namespace Engine::priv;

void RenderGraph::addModelInstanceToPipeline(ModelInstance& inModelInstance, ComponentModel& componentModel) {
    //material node check
    priv::MaterialNode* materialNode  = nullptr;
    priv::MeshNode*     meshNode      = nullptr;
    ModelInstance*      modelInstance = nullptr;
    for (auto& itr : m_MaterialNodes) {
        if (itr.material == inModelInstance.material()) {
            materialNode = &itr;
            //mesh node check
            for (auto& itr1 : materialNode->meshNodes) {
                if (itr1.mesh == inModelInstance.mesh()) {
                    meshNode = &itr1;
                    //instance check
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
void RenderGraph::removeModelInstanceFromPipeline(ModelInstance& inModelInstance) {
    //material node check
    priv::MaterialNode*  materialNode  = nullptr;
    priv::MeshNode*      meshNode      = nullptr;
    ModelInstance*       modelInstance = nullptr;
    for (auto& itr : m_MaterialNodes) {
        if (itr.material == inModelInstance.material()) {
            materialNode = &itr;
            //mesh node check
            for (auto& itr1 : materialNode->meshNodes) {
                if (itr1.mesh == inModelInstance.mesh()) {
                    meshNode = &itr1;
                    //instance check
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


//TODO: correct this
void RenderGraph::sort_bruteforce(Camera& camera, SortingMode sortingMode) {
#ifndef _DEBUG
    glm_vec3 camPos = (camera.getPosition());
    auto lambda_sorter = [&](ModelInstance* lhs, ModelInstance* rhs) {
        auto lhsParent = lhs->parent();
        auto rhsParent = rhs->parent();

        auto [lhsBody, lhsModel] = lhsParent.getComponents<ComponentBody, ComponentModel>();
        auto [rhsBody, rhsModel] = rhsParent.getComponents<ComponentBody, ComponentModel>();

        auto lhsPos    = lhsBody->getPosition();
        auto rhsPos    = rhsBody->getPosition();
        auto lhsRad    = lhsModel->radius();
        auto rhsRad    = rhsModel->radius();

        auto leftDir   = glm::normalize(lhsPos - camPos);
        auto rightDir  = glm::normalize(rhsPos - camPos);

        auto leftPos   = lhsPos - (leftDir * (decimal)lhsRad);
        auto rightPos  = rhsPos - (rightDir * (decimal)rhsRad);

        if (sortingMode == SortingMode::FrontToBack)
            return glm::distance2(leftPos, camPos) < glm::distance2(rightPos, camPos);
        else if (sortingMode == SortingMode::BackToFront)
            return glm::distance2(leftPos, camPos) > glm::distance2(rightPos, camPos);
        else
            return false;
        return false;
    };
    std::sort(/*std::execution::par_unseq, */ m_InstancesTotal.begin(), m_InstancesTotal.end(), lambda_sorter);
#endif
}
void RenderGraph::sort_cheap_bruteforce(Camera& camera, SortingMode sortingMode) {
#ifndef _DEBUG
    glm::vec3 camPos = glm::vec3(camera.getPosition());
    auto lambda_sorter = [&](ModelInstance* lhs, ModelInstance* rhs) {
        auto& lhsInstance         = *lhs;
        auto& rhsInstance         = *rhs;
        auto lhsParent            = lhsInstance.parent();
        auto rhsParent            = rhsInstance.parent();
        ComponentBody& lhsBody    = *lhsParent.getComponent<ComponentBody>();
        ComponentBody& rhsBody    = *rhsParent.getComponent<ComponentBody>();
        glm::vec3 lhsPos          = glm::vec3(lhsBody.getPosition()) + lhsInstance.position();
        glm::vec3 rhsPos          = glm::vec3(rhsBody.getPosition()) + rhsInstance.position();

        if (sortingMode == SortingMode::FrontToBack)
            return glm::distance2(lhsPos, camPos) < glm::distance2(rhsPos, camPos);
        else if (sortingMode == SortingMode::BackToFront)
            return glm::distance2(lhsPos, camPos) > glm::distance2(rhsPos, camPos);
        else
            return false;
        return false;
    };
    std::sort(/*std::execution::par_unseq, */ m_InstancesTotal.begin(), m_InstancesTotal.end(), lambda_sorter);
#endif
}

void RenderGraph::sort_cheap(Camera& camera, SortingMode sortingMode) {
#ifndef _DEBUG
    glm::vec3 camPos = glm::vec3(camera.getPosition());
    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            auto& vect = meshNode.instanceNodes;

            auto lambda_sorter = [&](ModelInstance* lhs, ModelInstance* rhs) {
                auto& lhsInstance      = *lhs;
                auto& rhsInstance      = *rhs;
                auto  lhsParent        = lhsInstance.parent();
                auto  rhsParent        = rhsInstance.parent();
                ComponentBody& lhsBody = *lhsParent.getComponent<ComponentBody>();
                ComponentBody& rhsBody = *rhsParent.getComponent<ComponentBody>();
                glm::vec3 lhsPos       = glm::vec3(lhsBody.getPosition()) + lhsInstance.position();
                glm::vec3 rhsPos       = glm::vec3(rhsBody.getPosition()) + rhsInstance.position();

                if (sortingMode == SortingMode::FrontToBack)
                    return glm::distance2(lhsPos, camPos) < glm::distance2(rhsPos, camPos);
                else if (sortingMode == SortingMode::BackToFront)
                    return glm::distance2(lhsPos, camPos) > glm::distance2(rhsPos, camPos);
                else
                    return false;
                return false;
            };

            std::sort(/*std::execution::par_unseq, */ vect.begin(), vect.end(), lambda_sorter);
        }
    }
#endif
}
//TODO: correct this
void RenderGraph::sort(Camera& camera, SortingMode sortingMode) {
#ifndef _DEBUG
    glm_vec3 camPos = (camera.getPosition());
    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            auto& vect = meshNode.instanceNodes;

            auto lambda_sorter = [&](ModelInstance* lhs, ModelInstance* rhs) {
                auto  lhsParent = lhs->parent();
                auto  rhsParent = rhs->parent();

                auto [lhsBody, lhsModel] = lhsParent.getComponents<ComponentBody, ComponentModel>();
                auto [rhsBody, rhsModel] = rhsParent.getComponents<ComponentBody, ComponentModel>();

                auto lhsPos     = lhsBody->getPosition();
                auto rhsPos     = rhsBody->getPosition();
                auto lhsRad     = lhsModel->radius();
                auto rhsRad     = rhsModel->radius();

                auto leftDir    = glm::normalize(lhsPos - camPos);
                auto rightDir   = glm::normalize(rhsPos - camPos);

                auto leftPos    = lhsPos - (leftDir * (decimal)lhsRad);
                auto rightPos   = rhsPos - (rightDir * (decimal)rhsRad);

                if (sortingMode == SortingMode::FrontToBack)
                    return glm::distance2(leftPos, camPos) < glm::distance2(rightPos, camPos);
                else if (sortingMode == SortingMode::BackToFront)
                    return glm::distance2(leftPos, camPos) > glm::distance2(rightPos, camPos);
                else
                    return false;
                return false;
            };
            std::sort(/*std::execution::par_unseq, */ vect.begin(), vect.end(), lambda_sorter);
        }
    }
#endif
}
void RenderGraph::clean(Entity inEntity) {
    if (inEntity.null()) {
        return;
    }
    std::vector<ModelInstance*> kept_nodes_total;
    kept_nodes_total.reserve(m_InstancesTotal.size());

    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            std::vector<ModelInstance*> kept_nodes;
            kept_nodes.reserve(meshNode.instanceNodes.size());
            for (auto& modelInstance : meshNode.instanceNodes) {
                auto entity = modelInstance->parent();
                if (entity != inEntity) {
                    kept_nodes.push_back(modelInstance);
                    kept_nodes_total.push_back(modelInstance);
                }
            }
            meshNode.instanceNodes.clear();
            std::move(kept_nodes.begin(), kept_nodes.end(), std::back_inserter(meshNode.instanceNodes));
        }
    }
    m_InstancesTotal.clear();
    std::move(kept_nodes_total.begin(), kept_nodes_total.end(), std::back_inserter(m_InstancesTotal));
}
void RenderGraph::validate_model_instances_for_rendering(Viewport& viewport, Camera& camera) {
    decimal global_distance_factor = ModelInstance::getGlobalDistanceFactor();
    auto lambda = [&](std::vector<ModelInstance*>& inInstanceNodes, const glm_vec3& camPos) {
        for (auto& modelInstancePtr : inInstanceNodes) {
            auto& modelInstance = *modelInstancePtr;
            auto body           = modelInstance.parent().getComponent<ComponentBody>();
            auto model          = modelInstance.parent().getComponent<ComponentModel>();
            bool is_valid_viewport = InternalModelInstancePublicInterface::IsViewportValid(modelInstance, viewport);
            if (is_valid_viewport) {
                if (body) {
                    if (modelInstance.isForceRendered()) {
                        modelInstance.setPassedRenderCheck(modelInstance.visible());
                    }else{
                        float radius            = model->radius();
                        glm_vec3 pos            = body->getPosition() + glm_vec3(modelInstance.position());
                        unsigned int sphereTest = camera.sphereIntersectTest(pos, radius); //per mesh instance radius instead?
                        decimal comparison      = (decimal)radius * global_distance_factor;

                        bool failedVisibleTest  = !modelInstance.visible();
                        bool failedSphereTest   = (sphereTest == 0);
                        bool failedDistanceTest = (glm::distance2(pos, camPos) > comparison * comparison);

                        bool result = !((failedVisibleTest || failedSphereTest || failedDistanceTest));
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
    auto* shaderProgram = m_ShaderProgram.get<ShaderProgram>();
    if (useDefaultShaders) {
        renderer.bind(shaderProgram);
    }
    for (auto& materialNode : m_MaterialNodes) {
        if (materialNode.meshNodes.size() > 0) {
            auto& material = *materialNode.material.get<Material>();
            renderer.bind(&material);
            for (auto& meshNode : materialNode.meshNodes) {
                if (meshNode.instanceNodes.size() > 0) {
                    auto& mesh = *meshNode.mesh.get<Mesh>();
                    renderer.bind(&mesh);
                    for (auto& modelInstancePtr : meshNode.instanceNodes) {
                        auto& modelInstance   = *modelInstancePtr;
                        auto body             = modelInstance.parent().getComponent<ComponentBody>();
                        glm::mat4 modelMatrix = body->modelMatrixRendering();
                        if (modelInstance.passedRenderCheck()) {
                            if (sortingMode != SortingMode::None) {
                                mesh.sortTriangles(camera, modelInstance, modelMatrix, sortingMode);
                            }
                            renderer.bind(&modelInstance);
                            renderer.m_Pipeline->renderMesh(mesh, (unsigned int)modelInstance.getDrawingMode());
                            renderer.unbind(&modelInstance);
                        }
                    }
                    //protect against any custom changes by restoring to the regular shader and material
                    if (useDefaultShaders) {
                        if (renderer.m_Pipeline->getCurrentBoundShaderProgram() != shaderProgram) {
                            renderer.bind(shaderProgram);
                            renderer.bind(&material);
                        }
                    }
                    renderer.unbind(&mesh);
                }
            }
            renderer.unbind(&material);
        }
    }
}
void RenderGraph::render_bruteforce(Engine::priv::RenderModule& renderer, Viewport& viewport, Camera& camera, bool useDefaultShaders, SortingMode sortingMode) {
    auto* shaderProgram = m_ShaderProgram.get<ShaderProgram>();
    if (useDefaultShaders) {
        renderer.bind(shaderProgram);
    }
    for (auto& modelInstancePtr : m_InstancesTotal) {
        auto& modelInstance   = *modelInstancePtr;
        auto* mesh            = modelInstance.mesh().get<Mesh>();
        auto* material        = modelInstance.material().get<Material>();
        auto  body            = modelInstance.parent().getComponent<ComponentBody>();
        glm::mat4 modelMatrix = body->modelMatrixRendering();
        if (modelInstance.passedRenderCheck()) {
            if (sortingMode != SortingMode::None) {
                mesh->sortTriangles(camera, modelInstance, modelMatrix, sortingMode);
            }
            renderer.bind(material);
            renderer.bind(mesh);
            renderer.bind(&modelInstance);

            renderer.m_Pipeline->renderMesh(*mesh, (unsigned int)modelInstance.getDrawingMode());

            renderer.unbind(&modelInstance);
            renderer.unbind(mesh);
            renderer.unbind(material);
        }
        //protect against any custom changes by restoring to the regular shader and material
        if (useDefaultShaders) {
            if (renderer.m_Pipeline->getCurrentBoundShaderProgram() != shaderProgram) {
                renderer.bind(shaderProgram);
                renderer.bind(material);
            }
        }
    }
}

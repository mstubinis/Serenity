#include <core/engine/renderer/RenderGraph.h>
#include <core/engine/renderer/pipelines/IRenderingPipeline.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/system/Engine.h>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <execution>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

//TODO: profile std::execution::par_unseq compared to regular execution, the overhead to set up par_unseq might be more trouble than it's worth for these, unlike in the mesh triangle sorter


#pragma region MaterialNode
MaterialNode::MaterialNode(const Material& material_) {
    material = &const_cast<Material&>(material_);
}
MaterialNode::~MaterialNode() {
}
MaterialNode::MaterialNode(MaterialNode&& other) noexcept {
    material  = std::exchange(other.material, nullptr);
    meshNodes = std::move(other.meshNodes);
}
MaterialNode& MaterialNode::operator=(MaterialNode&& other) noexcept {
    if (&other != this) {
        material  = std::exchange(other.material, nullptr);
        meshNodes = std::move(other.meshNodes);
    }
    return *this;
}

#pragma endregion

#pragma region MeshNode
MeshNode::MeshNode(const Mesh& mesh_) {
    mesh = &const_cast<Mesh&>(mesh_);
}
MeshNode::~MeshNode() {
    SAFE_DELETE_VECTOR(instanceNodes);
}
MeshNode::MeshNode(MeshNode&& other) noexcept {
    mesh          = std::exchange(other.mesh, nullptr);
    instanceNodes = std::move(other.instanceNodes);
}
MeshNode& MeshNode::operator=(MeshNode&& other) noexcept {
    if (&other != this) {
        mesh          = std::exchange(other.mesh, nullptr);
        instanceNodes = std::move(other.instanceNodes);
    }
    return *this;
}

#pragma endregion

#pragma region InstanceNode
InstanceNode::InstanceNode(const ModelInstance& modelInstance_){
    instance = &const_cast<ModelInstance&>(modelInstance_);
}
InstanceNode::~InstanceNode() {
}
InstanceNode::InstanceNode(InstanceNode&& other) noexcept {
    instance = std::exchange(other.instance, nullptr);
}
InstanceNode& InstanceNode::operator=(InstanceNode&& other) noexcept {
    if (&other != this) {
        instance = std::exchange(other.instance, nullptr);
    }
    return *this;
}

#pragma endregion

RenderGraph::RenderGraph(ShaderProgram& shaderProgram){
    m_ShaderProgram = &shaderProgram;
}

RenderGraph::RenderGraph(RenderGraph&& other) noexcept {
    m_ShaderProgram  = std::exchange(other.m_ShaderProgram, nullptr);
    m_MaterialNodes  = std::move(other.m_MaterialNodes);
    m_InstancesTotal = std::move(other.m_InstancesTotal);
}
RenderGraph& RenderGraph::operator=(RenderGraph&& other) noexcept {
    if (&other != this) {
        m_ShaderProgram  = std::exchange(other.m_ShaderProgram, nullptr);
        m_MaterialNodes  = std::move(other.m_MaterialNodes);
        m_InstancesTotal = std::move(other.m_InstancesTotal);
    }
    return *this;
}


RenderGraph::~RenderGraph() {
}

void RenderGraph::addModelInstanceToPipeline(ModelInstance& modelInstance) {
    //material node check
    priv::MaterialNode* materialNode = nullptr;
    priv::MeshNode*     meshNode     = nullptr;
    priv::InstanceNode* instanceNode = nullptr;
    for (auto& itr : m_MaterialNodes) {
        if (itr.material == modelInstance.material()) {
            materialNode = &itr;
            //mesh node check
            for (auto& itr1 : materialNode->meshNodes) {
                if (itr1.mesh == modelInstance.mesh()) {
                    meshNode = &itr1;
                    //instance check
                    for (auto& itr2 : meshNode->instanceNodes) {
                        if (itr2->instance == &modelInstance) {
                            instanceNode = itr2;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (!materialNode) {
        materialNode = &(m_MaterialNodes.emplace_back(*modelInstance.material()));
    }
    if (!meshNode) {
        meshNode = &(materialNode->meshNodes.emplace_back(*modelInstance.mesh()));
    }
    if (!instanceNode) {
        instanceNode = NEW InstanceNode(modelInstance);
        meshNode->instanceNodes.push_back(instanceNode);
        m_InstancesTotal.push_back(instanceNode);
    }
}
void RenderGraph::removeModelInstanceFromPipeline(ModelInstance& modelInstance) {
    //material node check
    priv::MaterialNode*  materialNode  = nullptr;
    priv::MeshNode*      meshNode      = nullptr;
    priv::InstanceNode*  instanceNode  = nullptr;
    for (auto& itr : m_MaterialNodes) {
        if (itr.material == modelInstance.material()) {
            materialNode = &itr;
            //mesh node check
            for (auto& itr1 : materialNode->meshNodes) {
                if (itr1.mesh == modelInstance.mesh()) {
                    meshNode = &itr1;
                    //instance check
                    for (auto& itr2 : meshNode->instanceNodes) {
                        if (itr2->instance == &modelInstance) {
                            instanceNode = itr2;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (meshNode) {
        remove_instance_node(*meshNode, *instanceNode);
        if (meshNode->instanceNodes.size() == 0) {
            remove_mesh_node(*materialNode, *meshNode);
            if (materialNode->meshNodes.size() == 0) {
                remove_material_node(*materialNode);
            }
        }
    }
}
const bool RenderGraph::remove_material_node(const MaterialNode& materialNode) {
    for (size_t i = 0; i < m_MaterialNodes.size(); ++i) {
        if (&m_MaterialNodes[i] == &materialNode) {
            m_MaterialNodes.erase(m_MaterialNodes.begin() + i);
            return true;
        }
    }
    return false;
}
const bool RenderGraph::remove_mesh_node(MaterialNode& materialNode, const MeshNode& meshNode) {
    for (size_t i = 0; i < materialNode.meshNodes.size(); ++i) {
        if (&materialNode.meshNodes[i] == &meshNode) {
            materialNode.meshNodes.erase(materialNode.meshNodes.begin() + i);
            return true;
        }
    }
    return false;
}
const bool RenderGraph::remove_instance_node(MeshNode& meshNode, const InstanceNode& instanceNode) {
    for (size_t i = 0; i < meshNode.instanceNodes.size(); ++i) {
        if (meshNode.instanceNodes[i] == &instanceNode) {
            meshNode.instanceNodes.erase(meshNode.instanceNodes.begin() + i);
        }
    }
    for (size_t i = 0; i < m_InstancesTotal.size(); ++i) {
        if (m_InstancesTotal[i] == &instanceNode) {
            SAFE_DELETE(m_InstancesTotal[i]);
            m_InstancesTotal.erase(m_InstancesTotal.begin() + i);
            return true;
        }
    }
    return false;
}




//TODO: correct this
void RenderGraph::sort_bruteforce(const Camera& camera, SortingMode::Mode sortingMode) {
#ifndef _DEBUG
    const auto lambda_sorter = [&](InstanceNode* lhs, InstanceNode* rhs, const glm_vec3& camPos) {
        auto lhsParent       = lhs->instance->parent();
        auto rhsParent       = rhs->instance->parent();
        const auto& _dataReq1(lhsParent);
        const auto& _dataReq2(rhsParent);

        const auto& lhsBody  = *lhsParent.getComponent<ComponentBody>(_dataReq1);
        const auto& rhsBody  = *rhsParent.getComponent<ComponentBody>(_dataReq2);
        const auto& lhsModel = *lhsParent.getComponent<ComponentModel>(_dataReq1);
        const auto& rhsModel = *rhsParent.getComponent<ComponentModel>(_dataReq2);

        const auto& lhsPos   = lhsBody.getPosition();
        const auto& rhsPos   = rhsBody.getPosition();
        const auto& lhsRad   = lhsModel.radius();
        const auto& rhsRad   = rhsModel.radius();

        const auto leftDir   = glm::normalize(lhsPos - camPos);
        const auto rightDir  = glm::normalize(rhsPos - camPos);

        const auto leftPos   = lhsPos - (leftDir * static_cast<decimal>(lhsRad));
        const auto rightPos  = rhsPos - (rightDir * static_cast<decimal>(rhsRad));

        if (sortingMode == SortingMode::FrontToBack)
            return glm::distance2(leftPos, camPos) < glm::distance2(rightPos, camPos);
        else if (sortingMode == SortingMode::BackToFront)
            return glm::distance2(leftPos, camPos) > glm::distance2(rightPos, camPos);
        else
            return false;
        return false;
    };
    std::sort(/*std::execution::par_unseq, */ m_InstancesTotal.begin(), m_InstancesTotal.end(), 
        std::bind(lambda_sorter, std::placeholders::_1, std::placeholders::_2, camera.getPosition())
    );
#endif
}
void RenderGraph::sort_cheap_bruteforce(const Camera& camera, SortingMode::Mode sortingMode) {
#ifndef _DEBUG
    const auto lambda_sorter = [&](InstanceNode* lhs, InstanceNode* rhs, const glm::vec3& camPos) {
        auto& lhsInstance   = *lhs->instance;
        auto& rhsInstance   = *rhs->instance;
        auto lhsParent      = lhsInstance.parent();
        auto rhsParent      = rhsInstance.parent();
        const auto& lhsBody = *lhsParent.getComponent<ComponentBody>();
        const auto& rhsBody = *rhsParent.getComponent<ComponentBody>();
        const auto lhsPos   = glm::vec3(lhsBody.getPosition()) + lhsInstance.position();
        const auto rhsPos   = glm::vec3(rhsBody.getPosition()) + rhsInstance.position();

        if (sortingMode == SortingMode::FrontToBack)
            return glm::distance2(lhsPos, camPos) < glm::distance2(rhsPos, camPos);
        else if (sortingMode == SortingMode::BackToFront)
            return glm::distance2(lhsPos, camPos) > glm::distance2(rhsPos, camPos);
        else
            return false;
        return false;
    };
    std::sort(/*std::execution::par_unseq, */ m_InstancesTotal.begin(), m_InstancesTotal.end(), 
        std::bind(lambda_sorter, std::placeholders::_1, std::placeholders::_2, glm::vec3(camera.getPosition()))
    );
#endif
}

void RenderGraph::sort_cheap(const Camera& camera, SortingMode::Mode sortingMode) {
#ifndef _DEBUG
    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            auto& vect = meshNode.instanceNodes;

            const auto lambda_sorter = [&](InstanceNode* lhs, InstanceNode* rhs, const glm::vec3& camPos) {
                auto& lhsInstance    = *lhs->instance;
                auto& rhsInstance    = *rhs->instance;
                auto  lhsParent      = lhsInstance.parent();
                auto  rhsParent      = rhsInstance.parent();
                const auto& lhsBody  = *lhsParent.getComponent<ComponentBody>();
                const auto& rhsBody  = *rhsParent.getComponent<ComponentBody>();
                const auto lhsPos    = glm::vec3(lhsBody.getPosition()) + lhsInstance.position();
                const auto rhsPos    = glm::vec3(rhsBody.getPosition()) + rhsInstance.position();

                if (sortingMode == SortingMode::FrontToBack)
                    return glm::distance2(lhsPos, camPos) < glm::distance2(rhsPos, camPos);
                else if (sortingMode == SortingMode::BackToFront)
                    return glm::distance2(lhsPos, camPos) > glm::distance2(rhsPos, camPos);
                else
                    return false;
                return false;
            };

            std::sort(/*std::execution::par_unseq, */ vect.begin(), vect.end(), 
                std::bind(lambda_sorter, std::placeholders::_1, std::placeholders::_2, glm::vec3(camera.getPosition()))
            );
        }
    }
#endif
}
//TODO: correct this
void RenderGraph::sort(const Camera& camera, SortingMode::Mode sortingMode) {
#ifndef _DEBUG
    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            auto& vect = meshNode.instanceNodes;

            const auto lambda_sorter = [&](InstanceNode* lhs, InstanceNode* rhs, const glm_vec3& camPos) {
                auto  lhsParent      = lhs->instance->parent();
                auto  rhsParent      = rhs->instance->parent();
                const EntityDataRequest dataReq1(lhsParent);
                const EntityDataRequest dataReq2(rhsParent);

                const auto& lhsBody  = *lhsParent.getComponent<ComponentBody>(dataReq1);
                const auto& rhsBody  = *rhsParent.getComponent<ComponentBody>(dataReq2);
                const auto& lhsModel = *lhsParent.getComponent<ComponentModel>(dataReq1);
                const auto& rhsModel = *rhsParent.getComponent<ComponentModel>(dataReq2);

                const auto& lhsPos   = lhsBody.getPosition();
                const auto& rhsPos   = rhsBody.getPosition();
                const auto& lhsRad   = lhsModel.radius();
                const auto& rhsRad   = rhsModel.radius();

                const auto leftDir   = glm::normalize(lhsPos - camPos);
                const auto rightDir  = glm::normalize(rhsPos - camPos);

                const auto leftPos   = lhsPos - (leftDir * static_cast<decimal>(lhsRad));
                const auto rightPos  = rhsPos - (rightDir * static_cast<decimal>(rhsRad));

                if (sortingMode == SortingMode::FrontToBack)
                    return glm::distance2(leftPos, camPos) < glm::distance2(rightPos, camPos);
                else if (sortingMode == SortingMode::BackToFront)
                    return glm::distance2(leftPos, camPos) > glm::distance2(rightPos, camPos);
                else
                    return false;
                return false;
            };
            std::sort(/*std::execution::par_unseq, */ vect.begin(), vect.end(), 
                std::bind(lambda_sorter, std::placeholders::_1, std::placeholders::_2, camera.getPosition())
            );
        }
    }
#endif
}
void RenderGraph::clean(Entity inEntity) {
    vector<InstanceNode*> kept_nodes_total;
    for (auto& materialNode : m_MaterialNodes) {
        for (auto& meshNode : materialNode.meshNodes) {
            vector<InstanceNode*>& instances = meshNode.instanceNodes;
            vector<InstanceNode*> kept_nodes;
            vector<InstanceNode*> removed_nodes;
            for (auto& instanceNode : instances) {
                auto entity = instanceNode->instance->parent();
                if (entity != inEntity) {
                    kept_nodes.push_back(instanceNode);
                    kept_nodes_total.push_back(instanceNode);
                }else{
                    removed_nodes.push_back(instanceNode);
                }
            }
            SAFE_DELETE_VECTOR(removed_nodes);
            instances.clear();
            std::move(kept_nodes.begin(), kept_nodes.end(), std::back_inserter(instances));
        }
    }
    m_InstancesTotal.clear();
    std::move(kept_nodes_total.begin(), kept_nodes_total.end(), std::back_inserter(m_InstancesTotal));
}
void RenderGraph::validate_model_instances_for_rendering(const Viewport& viewport, const Camera& camera) {
    const auto& global_distance_factor = ModelInstance::getGlobalDistanceFactor();
    auto lambda = [&](vector<priv::InstanceNode*>& vector, const glm_vec3& camPos) {
        for (auto& instanceNode : vector) {
            auto& modelInstance          = *instanceNode->instance;
            auto* body                   = modelInstance.parent().getComponent<ComponentBody>();
            auto* model                  = modelInstance.parent().getComponent<ComponentModel>();
            const bool is_valid_viewport = InternalModelInstancePublicInterface::IsViewportValid(modelInstance, viewport);
            if (is_valid_viewport) {
                if (body) {
                    if (modelInstance.isForceRendered()) {
                        if (modelInstance.visible()) {
                            modelInstance.setPassedRenderCheck(true);
                        }else{
                            modelInstance.setPassedRenderCheck(false);
                        }
                    }else{
                        const float radius    = model->radius();
                        const glm_vec3 pos    = body->getPosition() + glm_vec3(modelInstance.position());
                        const uint sphereTest = camera.sphereIntersectTest(pos, radius); //per mesh instance radius instead?
                        decimal comparison    = static_cast<decimal>(radius) * global_distance_factor;

                        const bool failedVisibleTest  = !modelInstance.visible();
                        const bool failedSphereTest   = (sphereTest == 0);
                        const bool failedDistanceTest = (glm::distance2(pos, camPos) > comparison * comparison);

                        if (failedVisibleTest || failedSphereTest || failedDistanceTest) {
                            modelInstance.setPassedRenderCheck(false);
                        }else{
                            modelInstance.setPassedRenderCheck(true);
                        }
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
void RenderGraph::render(const Engine::priv::Renderer& renderer, const Viewport& viewport, const Camera& camera, bool useDefaultShaders, SortingMode::Mode sortingMode) {
    if (useDefaultShaders) {
        renderer.bind(m_ShaderProgram);
    }
    for (auto& materialNode : m_MaterialNodes) {
        if (materialNode.meshNodes.size() > 0) {
            auto& material = *materialNode.material;
            renderer.bind(&material);
            for (auto& meshNode : materialNode.meshNodes) {
                if (meshNode.instanceNodes.size() > 0) {
                    auto& mesh = *meshNode.mesh;

                    renderer.bind(&mesh);

                    for (auto& instanceNode : meshNode.instanceNodes) {
                        auto& modelInstance = *instanceNode->instance;
                        auto* body = modelInstance.parent().getComponent<ComponentBody>();
                        auto modelMatrix = body->modelMatrixRendering();
                        if (modelInstance.passedRenderCheck()) {
                            if (sortingMode != SortingMode::None) {
                                mesh.sortTriangles(camera, modelInstance, modelMatrix, sortingMode);
                            }
                            renderer.bind(&modelInstance);
                            renderer.m_Pipeline->renderMesh(mesh, modelInstance.getDrawingMode());
                            renderer.unbind(&modelInstance);
                        }
                    }
                    //protect against any custom changes by restoring to the regular shader and material
                    if (useDefaultShaders) {
                        if (renderer.m_Pipeline->getCurrentBoundShaderProgram() != m_ShaderProgram) {
                            renderer.bind(m_ShaderProgram);
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
void RenderGraph::render_bruteforce(const Engine::priv::Renderer& renderer, const Viewport& viewport, const Camera& camera, bool useDefaultShaders, SortingMode::Mode sortingMode) {
    if (useDefaultShaders) {
        renderer.bind(m_ShaderProgram);
    }
    for (auto& instance : m_InstancesTotal) {
        auto& modelInstance = *instance->instance;
        auto& mesh          = *modelInstance.mesh();
        auto& material      = *modelInstance.material();
        auto* body           = modelInstance.parent().getComponent<ComponentBody>();
        auto modelMatrix     = body->modelMatrixRendering();
        if (modelInstance.passedRenderCheck()) {
            if (sortingMode != SortingMode::None) {
                mesh.sortTriangles(camera, modelInstance, modelMatrix, sortingMode);
            }
            renderer.bind(&material);
            renderer.bind(&mesh);
            renderer.bind(&modelInstance);

            renderer.m_Pipeline->renderMesh(mesh, modelInstance.getDrawingMode());

            renderer.unbind(&modelInstance);
            renderer.unbind(&mesh);
            renderer.unbind(&material);
        }
        //protect against any custom changes by restoring to the regular shader and material
        if (useDefaultShaders) {
            if (renderer.m_Pipeline->getCurrentBoundShaderProgram() != m_ShaderProgram) {
                renderer.bind(m_ShaderProgram);
                renderer.bind(&material);
            }
        }
    }
}

#include <core/engine/renderer/RenderGraph.h>
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


RenderGraph::RenderGraph(ShaderProgram& _shaderProgram) : shaderProgram(_shaderProgram) {
}
RenderGraph::~RenderGraph() {
    SAFE_DELETE_VECTOR(materialNodes);
}
//TODO: correct this
void RenderGraph::sort_bruteforce(Camera& camera, const SortingMode::Mode sortingMode) {
#ifndef _DEBUG
    auto lambda_sorter = [&](InstanceNode* lhs, InstanceNode* rhs, const glm_vec3& camPos) {
        auto& lhsParent = lhs->instance->parent();
        auto& rhsParent = rhs->instance->parent();
        const auto& _dataReq1(lhsParent);
        const auto& _dataReq2(rhsParent);

        const auto& lhsBody = *lhsParent.getComponent<ComponentBody>(_dataReq1);
        const auto& rhsBody = *rhsParent.getComponent<ComponentBody>(_dataReq2);
        const auto& lhsModel = *lhsParent.getComponent<ComponentModel>(_dataReq1);
        const auto& rhsModel = *rhsParent.getComponent<ComponentModel>(_dataReq2);

        const auto& lhsPos = lhsBody.position();
        const auto& rhsPos = rhsBody.position();
        const auto& lhsRad = lhsModel.radius();
        const auto& rhsRad = rhsModel.radius();

        const auto leftDir = glm::normalize(lhsPos - camPos);
        const auto rightDir = glm::normalize(rhsPos - camPos);

        const auto leftPos = lhsPos - (leftDir * static_cast<decimal>(lhsRad));
        const auto rightPos = rhsPos - (rightDir * static_cast<decimal>(rhsRad));

        if (sortingMode == SortingMode::FrontToBack)
            return glm::distance2(leftPos, camPos) < glm::distance2(rightPos, camPos);
        else if (sortingMode == SortingMode::BackToFront)
            return glm::distance2(leftPos, camPos) > glm::distance2(rightPos, camPos);
        else
            return false;
        return false;
    };
    std::sort(/*std::execution::par_unseq, */ instancesTotal.begin(), instancesTotal.end(), std::bind(lambda_sorter, std::placeholders::_1, std::placeholders::_2, camera.getPosition()));
#endif
}
void RenderGraph::sort_cheap_bruteforce(Camera& camera, const SortingMode::Mode sortingMode) {
#ifndef _DEBUG
    const auto& lambda_sorter = [&](InstanceNode* lhs, InstanceNode* rhs, const glm::vec3& camPos) {
        auto& lhsInstance = *lhs->instance;
        auto& rhsInstance = *rhs->instance;
        auto& lhsParent = lhsInstance.parent();
        auto& rhsParent = rhsInstance.parent();
        const auto& lhsBody = *lhsParent.getComponent<ComponentBody>();
        const auto& rhsBody = *rhsParent.getComponent<ComponentBody>();
        const auto lhsPos = glm::vec3(lhsBody.position()) + lhsInstance.position();
        const auto rhsPos = glm::vec3(rhsBody.position()) + rhsInstance.position();

        if (sortingMode == SortingMode::FrontToBack)
            return glm::distance2(lhsPos, camPos) < glm::distance2(rhsPos, camPos);
        else if (sortingMode == SortingMode::BackToFront)
            return glm::distance2(lhsPos, camPos) > glm::distance2(rhsPos, camPos);
        else
            return false;
        return false;
    };
    std::sort(/*std::execution::par_unseq, */ instancesTotal.begin(), instancesTotal.end(), std::bind(lambda_sorter, std::placeholders::_1, std::placeholders::_2, glm::vec3(camera.getPosition())));
#endif
}

void RenderGraph::sort_cheap(Camera& camera, const SortingMode::Mode sortingMode) {
#ifndef _DEBUG
    for (auto& materialNode : materialNodes) {
        for (auto& meshNode : materialNode->meshNodes) {
            auto& vect = meshNode->instanceNodes;

            auto lambda_sorter = [&](InstanceNode* lhs, InstanceNode* rhs, const glm::vec3& camPos) {
                auto& lhsInstance = *lhs->instance;
                auto& rhsInstance = *rhs->instance;
                auto& lhsParent = lhsInstance.parent();
                auto& rhsParent = rhsInstance.parent();
                const auto& lhsBody = *lhsParent.getComponent<ComponentBody>();
                const auto& rhsBody = *rhsParent.getComponent<ComponentBody>();
                const auto lhsPos = glm::vec3(lhsBody.position()) + lhsInstance.position();
                const auto rhsPos = glm::vec3(rhsBody.position()) + rhsInstance.position();

                if (sortingMode == SortingMode::FrontToBack)
                    return glm::distance2(lhsPos, camPos) < glm::distance2(rhsPos, camPos);
                else if (sortingMode == SortingMode::BackToFront)
                    return glm::distance2(lhsPos, camPos) > glm::distance2(rhsPos, camPos);
                else
                    return false;
                return false;
            };

            std::sort(/*std::execution::par_unseq, */ vect.begin(), vect.end(), std::bind(lambda_sorter, std::placeholders::_1, std::placeholders::_2, glm::vec3(camera.getPosition())));
        }
    }
#endif
}
//TODO: correct this
void RenderGraph::sort(Camera& camera, const SortingMode::Mode sortingMode) {
#ifndef _DEBUG
    for (auto& materialNode : materialNodes) {
        for (auto& meshNode : materialNode->meshNodes) {
            auto& vect = meshNode->instanceNodes;

            auto lambda_sorter = [&](InstanceNode* lhs, InstanceNode* rhs, const glm_vec3& camPos) {
                auto& lhsParent = lhs->instance->parent();
                auto& rhsParent = rhs->instance->parent();
                const EntityDataRequest& _dataReq1(lhsParent);
                const EntityDataRequest& _dataReq2(rhsParent);

                const auto& lhsBody = *lhsParent.getComponent<ComponentBody>(_dataReq1);
                const auto& rhsBody = *rhsParent.getComponent<ComponentBody>(_dataReq2);
                const auto& lhsModel = *lhsParent.getComponent<ComponentModel>(_dataReq1);
                const auto& rhsModel = *rhsParent.getComponent<ComponentModel>(_dataReq2);

                const auto& lhsPos = lhsBody.position();
                const auto& rhsPos = rhsBody.position();
                const auto& lhsRad = lhsModel.radius();
                const auto& rhsRad = rhsModel.radius();

                const auto leftDir = glm::normalize(lhsPos - camPos);
                const auto rightDir = glm::normalize(rhsPos - camPos);

                const auto leftPos = lhsPos - (leftDir * static_cast<decimal>(lhsRad));
                const auto rightPos = rhsPos - (rightDir * static_cast<decimal>(rhsRad));

                if (sortingMode == SortingMode::FrontToBack)
                    return glm::distance2(leftPos, camPos) < glm::distance2(rightPos, camPos);
                else if (sortingMode == SortingMode::BackToFront)
                    return glm::distance2(leftPos, camPos) > glm::distance2(rightPos, camPos);
                else
                    return false;
                return false;
            };
            std::sort(/*std::execution::par_unseq, */ vect.begin(), vect.end(), std::bind(lambda_sorter, std::placeholders::_1, std::placeholders::_2, camera.getPosition()));
        }
    }
#endif
}
void RenderGraph::clean(const uint entityData) {
    vector<InstanceNode*> kept_nodes_total;
    for (auto& materialNode : materialNodes) {
        for (auto& meshNode : materialNode->meshNodes) {
            vector<InstanceNode*>& instances = meshNode->instanceNodes;
            vector<InstanceNode*> kept_nodes;
            vector<InstanceNode*> removed_nodes;
            for (auto& instanceNode : instances) {
                auto entity = instanceNode->instance->parent();
                if (entity.data != entityData) {
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
    instancesTotal.clear();
    std::move(kept_nodes_total.begin(), kept_nodes_total.end(), std::back_inserter(instancesTotal));
}
void RenderGraph::validate_model_instances_for_rendering(Viewport& viewport, Camera& camera) {
    //sf::Clock c;
    auto lambda = [&](vector<priv::InstanceNode*>& vector, const glm_vec3& camPos) {
        for (auto& instanceNode : vector) {
            auto& _modelInstance = *instanceNode->instance;
            auto* body = _modelInstance.parent().getComponent<ComponentBody>();
            auto& model = *_modelInstance.parent().getComponent<ComponentModel>();
            const bool is_valid_viewport = InternalModelInstancePublicInterface::IsViewportValid(_modelInstance, viewport);
            if (is_valid_viewport) {
                if (body) {
                    if (_modelInstance.isForceRendered()) {
                        if (_modelInstance.visible()) {
                            _modelInstance.setPassedRenderCheck(true);
                        }else{
                            _modelInstance.setPassedRenderCheck(false);
                        }
                    }else{
                        const float& radius = model.radius();
                        glm_vec3 pos = body->position() + glm_vec3(_modelInstance.position());
                        const uint sphereTest = camera.sphereIntersectTest(pos, radius); //per mesh instance radius instead?
                        decimal comparison = static_cast<decimal>(radius) * static_cast<decimal>(1100.0);
                        if (!_modelInstance.visible() || sphereTest == 0 || glm::distance2(pos, camPos) > comparison * comparison) {
                            _modelInstance.setPassedRenderCheck(false);
                        }else{
                            _modelInstance.setPassedRenderCheck(true);
                        }
                    }
                }else{
                    _modelInstance.setPassedRenderCheck(false);
                }
            }else{
                _modelInstance.setPassedRenderCheck(false);
            }
        }
    };
    auto camPos = camera.getPosition();
    lambda(instancesTotal, camPos);
    //this block is for multi-threading this section of code
    //auto vec = priv::threading::splitVector(instancesTotal);
    //for (auto& vector : vec) {
    //    priv::threading::addJobRef(lambda, vector, camPos);
    //}
    //epriv::threading::waitForAll();
    //std::cout << c.restart().asMicroseconds() << std::endl;
}
void RenderGraph::render(Viewport& viewport, Camera& camera, const bool useDefaultShaders, const SortingMode::Mode sortingMode) {
    if(useDefaultShaders)
        shaderProgram.bind();
    for (auto& materialNode : materialNodes) {
        if (materialNode->meshNodes.size() > 0) {
            auto& _material = *materialNode->material;
            _material.bind();
            for (auto& meshNode : materialNode->meshNodes) {
                if (meshNode->instanceNodes.size() > 0) {
                    auto& _mesh = *meshNode->mesh;
                    _mesh.bind();
                    for (auto& instanceNode : meshNode->instanceNodes) {
                        auto& _modelInstance = *instanceNode->instance;
                        auto* body = _modelInstance.parent().getComponent<ComponentBody>();
                        auto modelMatrix = body->modelMatrixRendering();
                        if (_modelInstance.passedRenderCheck()) {
                            if (sortingMode != SortingMode::None) {
                                _mesh.sortTriangles(camera, _modelInstance, modelMatrix, sortingMode);
                            }
                            _modelInstance.bind();
                            _mesh.render(false, _modelInstance.getDrawingMode());
                            _modelInstance.unbind();
                        }
                    }
                    //protect against any custom changes by restoring to the regular shader and material
                    if (useDefaultShaders) {
                        if (Core::m_Engine->m_RenderManager.RendererState.current_bound_shader_program != &shaderProgram) {
                            shaderProgram.bind();
                            _material.bind();
                        }
                    }
                    _mesh.unbind();
                }
            }
            _material.unbind();
        }
    }
}
void RenderGraph::render_bruteforce(Viewport& viewport, Camera& camera, const bool useDefaultShaders, const SortingMode::Mode sortingMode) {
    if (useDefaultShaders)
        shaderProgram.bind();
    for (auto& instance : instancesTotal) {
        auto& _modelInstance = *instance->instance;
        auto& _mesh = *_modelInstance.mesh();
        auto& _material = *_modelInstance.material();

        auto* body = _modelInstance.parent().getComponent<ComponentBody>();
        auto modelMatrix = body->modelMatrixRendering();
        if (_modelInstance.passedRenderCheck()) {
            if (sortingMode != SortingMode::None) {
                _mesh.sortTriangles(camera, _modelInstance, modelMatrix, sortingMode);
            }
            _material.bind();
            _mesh.bind();
            _modelInstance.bind();
            _mesh.render(false, _modelInstance.getDrawingMode());
            _modelInstance.unbind();
            _mesh.unbind();
            _material.unbind();
        }
        //protect against any custom changes by restoring to the regular shader and material
        if (useDefaultShaders) {
            if (Core::m_Engine->m_RenderManager.RendererState.current_bound_shader_program != &shaderProgram) {
                shaderProgram.bind();
                _material.bind();
            }
        }
    }
}

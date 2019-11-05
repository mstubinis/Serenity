#include <core/engine/renderer/RenderGraph.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>
#include <core/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/threading/Engine_ThreadManager.h>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <execution>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

//TODO: profile std::execution::par_unseq compared to regular execution, the overhead to set up par_unseq might be more trouble than it's worth for these, unlike in the mesh triangle sorter


RenderPipeline::RenderPipeline(ShaderProgram& _shaderProgram) :shaderProgram(_shaderProgram) {
}
RenderPipeline::~RenderPipeline() {
    SAFE_DELETE_VECTOR(materialNodes);
}
//TODO: correct this
void RenderPipeline::sort_bruteforce(Camera& camera, const SortingMode::Mode sortingMode) {
    const auto& lambda_sorter = [&camera, sortingMode](InstanceNode* lhs, InstanceNode* rhs) {
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

        const auto& camPos = camera.getPosition();
        const auto leftDir = glm::normalize(lhsPos - camPos);
        const auto rightDir = glm::normalize(rhsPos - camPos);

        const auto leftPos = lhsPos - (leftDir * static_cast<decimal>(lhsRad));
        const auto rightPos = rhsPos - (rightDir * static_cast<decimal>(rhsRad));

        if (sortingMode == SortingMode::FrontToBack)
            return camera.getDistanceSquared(leftPos) < camera.getDistanceSquared(rightPos);
        else if (sortingMode == SortingMode::BackToFront)
            return camera.getDistanceSquared(leftPos) > camera.getDistanceSquared(rightPos);
        else
            return false;
        return false;
    };
    std::sort( /*std::execution::par_unseq,*/ instancesTotal.begin(), instancesTotal.end(), lambda_sorter );
}
void RenderPipeline::sort_cheap_bruteforce(Camera& camera, const SortingMode::Mode sortingMode) {
    const auto& lambda_sorter = [&camera, sortingMode](InstanceNode* lhs, InstanceNode* rhs) {
        auto& lhsInstance = *lhs->instance;
        auto& rhsInstance = *rhs->instance;
        auto& lhsParent = lhsInstance.parent();
        auto& rhsParent = rhsInstance.parent();
        const auto& lhsBody = *lhsParent.getComponent<ComponentBody>();
        const auto& rhsBody = *rhsParent.getComponent<ComponentBody>();
        const auto lhsPos = glm::vec3(lhsBody.position()) + lhsInstance.position();
        const auto rhsPos = glm::vec3(rhsBody.position()) + rhsInstance.position();

        if (sortingMode == SortingMode::FrontToBack)
            return camera.getDistanceSquared(lhsPos) < camera.getDistanceSquared(rhsPos);
        else if (sortingMode == SortingMode::BackToFront)
            return camera.getDistanceSquared(lhsPos) > camera.getDistanceSquared(rhsPos);
        else
            return false;
        return false;
    };
    std::sort( /*std::execution::par_unseq,*/ instancesTotal.begin(), instancesTotal.end(), lambda_sorter );
}

void RenderPipeline::sort_cheap(Camera& camera, const SortingMode::Mode sortingMode) {
#ifndef _DEBUG
    for (auto& materialNode : materialNodes) {
        for (auto& meshNode : materialNode->meshNodes) {
            auto& vect = meshNode->instanceNodes;

            const auto& lambda_sorter = [&camera, sortingMode](InstanceNode* lhs, InstanceNode* rhs) {
                auto& lhsInstance = *lhs->instance;
                auto& rhsInstance = *rhs->instance;
                auto& lhsParent = lhsInstance.parent();
                auto& rhsParent = rhsInstance.parent();
                const auto& lhsBody = *lhsParent.getComponent<ComponentBody>();
                const auto& rhsBody = *rhsParent.getComponent<ComponentBody>();
                const auto lhsPos = glm::vec3(lhsBody.position()) + lhsInstance.position();
                const auto rhsPos = glm::vec3(rhsBody.position()) + rhsInstance.position();

                if (sortingMode == SortingMode::FrontToBack)
                    return camera.getDistanceSquared(lhsPos) < camera.getDistanceSquared(rhsPos);
                else if (sortingMode == SortingMode::BackToFront)
                    return camera.getDistanceSquared(lhsPos) > camera.getDistanceSquared(rhsPos);
                else
                    return false;
                return false;
            };

            std::sort( /*std::execution::par_unseq,*/ vect.begin(), vect.end(), lambda_sorter );
        }
    }
#endif
}
//TODO: correct this
void RenderPipeline::sort(Camera& camera, const SortingMode::Mode sortingMode) {
#ifndef _DEBUG
    for (auto& materialNode : materialNodes) {
        for (auto& meshNode : materialNode->meshNodes) {
            auto& vect = meshNode->instanceNodes;
            std::sort(
                vect.begin(), vect.end(),
                [&camera, sortingMode](InstanceNode* lhs, InstanceNode* rhs) {
                    auto& lhsParent = lhs->instance->parent();
                    auto& rhsParent = rhs->instance->parent();
                    const EntityDataRequest& _dataReq1(lhsParent);
                    const EntityDataRequest& _dataReq2(rhsParent);

                    const auto&  lhsBody  = *lhsParent.getComponent<ComponentBody>(_dataReq1);
                    const auto&  rhsBody  = *rhsParent.getComponent<ComponentBody>(_dataReq2);
                    const auto& lhsModel = *lhsParent.getComponent<ComponentModel>(_dataReq1);
                    const auto& rhsModel = *rhsParent.getComponent<ComponentModel>(_dataReq2);

                    const auto& lhsPos = lhsBody.position();
                    const auto& rhsPos = rhsBody.position();
                    const auto& lhsRad     = lhsModel.radius();
                    const auto& rhsRad     = rhsModel.radius();

                    const auto& camPos   = camera.getPosition();
                    const auto leftDir  = glm::normalize(lhsPos - camPos);
                    const auto rightDir = glm::normalize(rhsPos - camPos);

                    const auto leftPos  = lhsPos - (leftDir * static_cast<decimal>(lhsRad));
                    const auto rightPos = rhsPos - (rightDir * static_cast<decimal>(rhsRad));

                    if (sortingMode == SortingMode::FrontToBack)
                        return camera.getDistanceSquared(leftPos) < camera.getDistanceSquared(rightPos);
                    else if (sortingMode == SortingMode::BackToFront)
                        return camera.getDistanceSquared(leftPos) > camera.getDistanceSquared(rightPos);
                    else
                        return false;
                    return false;
                }
            );
        }
    }
#endif
}
void RenderPipeline::clean(const uint entityData) {
    vector<Engine::epriv::InstanceNode*> newNodesTotal;
    for (auto& materialNode : materialNodes) {
        for (auto& meshNode : materialNode->meshNodes) {
            auto& instances = meshNode->instanceNodes;
            vector<Engine::epriv::InstanceNode*> newNodes;
            for (auto& instanceNode : instances) {
                auto entity = instanceNode->instance->parent();
                if (entity.data != entityData) {
                    newNodes.push_back(instanceNode);
                    newNodesTotal.push_back(instanceNode);
                }
            }
            instances.clear();
            std::move(newNodes.begin(), newNodes.end(), std::back_inserter(instances));
        }
    }
    instancesTotal.clear();
    std::move(newNodesTotal.begin(), newNodesTotal.end(), std::back_inserter(instancesTotal));
}
void RenderPipeline::cpu_execute(Viewport& viewport, Camera& camera, const double& dt) {
    //sf::Clock c;
    for (auto& materialNode : materialNodes) {
        auto& _material = *materialNode->material;
        _material.update(dt);
    }  
    auto lambda = [&](vector<epriv::InstanceNode*>& vector) {
        for (auto& instanceNode : vector) {
            auto& _modelInstance = *instanceNode->instance;
            auto* body = _modelInstance.parent().getComponent<ComponentBody>();
            auto& model = *_modelInstance.parent().getComponent<ComponentModel>();
            if (InternalModelInstancePublicInterface::IsViewportValid(_modelInstance, viewport)) {
                if (body) {
                    if (_modelInstance.isForceRendered()) {
                        if (_modelInstance.visible()) {
                            _modelInstance.setPassedRenderCheck(true);
                        }else{
                            _modelInstance.setPassedRenderCheck(false);
                        }
                    }else{
                        const auto& radius = model.radius();
                        auto pos = glm::vec3(body->position()) + _modelInstance.position();
                        const auto sphereTest = camera.sphereIntersectTest(pos, radius); //per mesh instance radius instead?
                        auto comparison = radius * static_cast<decimal>(1100.0);
                        if (!_modelInstance.visible() || sphereTest == 0 || camera.getDistanceSquared(pos) > comparison * comparison) {
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


    lambda(instancesTotal);
    //this block is for multi-threading this section of code
    //auto vec = epriv::threading::splitVector(instancesTotal);
    //for (auto& vector : vec) {
    //    epriv::threading::addJobRef(lambda, vector);
    //}
    //epriv::threading::waitForAll();
    //std::cout << c.restart().asMicroseconds() << std::endl;
}
void RenderPipeline::render(Viewport& viewport, Camera& camera, const double& dt, const bool useDefaultShaders, const SortingMode::Mode sortingMode) {
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
void RenderPipeline::render_bruteforce(Viewport& viewport, Camera& camera, const double& dt, const bool useDefaultShaders, const SortingMode::Mode sortingMode) {
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
#include <core/engine/renderer/RenderGraph.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>
#include <core/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Viewport.h>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;


RenderPipeline::RenderPipeline(ShaderProgram& _shaderProgram) :shaderProgram(_shaderProgram) {
}
RenderPipeline::~RenderPipeline() {
    SAFE_DELETE_VECTOR(materialNodes);
}

void RenderPipeline::sort_cheap(Camera& camera, const SortingMode::Mode sortingMode) {
#ifndef _DEBUG
    for (auto& materialNode : materialNodes) {
        for (auto& meshNode : materialNode->meshNodes) {
            auto& vect = meshNode->instanceNodes;
            std::sort(
                vect.begin(), vect.end(),
                [&camera, &sortingMode](InstanceNode* lhs, InstanceNode* rhs) {
                    Entity& lhsParent = lhs->instance->parent();
                    Entity& rhsParent = rhs->instance->parent();
                    const ComponentBody& lhsBody = *lhsParent.getComponent<ComponentBody>();
                    const ComponentBody& rhsBody = *rhsParent.getComponent<ComponentBody>();
                    const glm::vec3& lhsPos = lhsBody.position();
                    const glm::vec3& rhsPos = rhsBody.position();

                    if (sortingMode == SortingMode::FrontToBack)
                        return camera.getDistanceSquared(lhsPos) < camera.getDistanceSquared(rhsPos);
                    else if (sortingMode == SortingMode::BackToFront)
                        return camera.getDistanceSquared(lhsPos) > camera.getDistanceSquared(rhsPos);
                    else
                        return false;
                }
            );
        }
    }
#endif
}
void RenderPipeline::sort(Camera& camera, const SortingMode::Mode sortingMode) {
#ifndef _DEBUG
    for (auto& materialNode : materialNodes) {
        for (auto& meshNode : materialNode->meshNodes) {
            auto& vect = meshNode->instanceNodes;
            std::sort(
                vect.begin(), vect.end(),
                [&camera, &sortingMode](InstanceNode* lhs, InstanceNode* rhs) {
                    Entity& lhsParent = lhs->instance->parent();
                    Entity& rhsParent = rhs->instance->parent();
                    const EntityDataRequest& _dataReq1(lhsParent);
                    const EntityDataRequest& _dataReq2(rhsParent);

                    const ComponentBody&  lhsBody  = *lhsParent.getComponent<ComponentBody>(_dataReq1);
                    const ComponentBody&  rhsBody  = *rhsParent.getComponent<ComponentBody>(_dataReq2);
                    const ComponentModel& lhsModel = *lhsParent.getComponent<ComponentModel>(_dataReq1);
                    const ComponentModel& rhsModel = *rhsParent.getComponent<ComponentModel>(_dataReq2);

                    const glm::vec3& lhsPos = lhsBody.position();
                    const glm::vec3& rhsPos = rhsBody.position();
                    const float& lhsRad     = lhsModel.radius();
                    const float& rhsRad     = rhsModel.radius();

                    const glm::vec3& camPos   = camera.getPosition();
                    const glm::vec3& leftDir  = glm::normalize(lhsPos - camPos);
                    const glm::vec3& rightDir = glm::normalize(rhsPos - camPos);

                    const glm::vec3& leftPos  = lhsPos - (leftDir * lhsRad);
                    const glm::vec3& rightPos = rhsPos - (rightDir * rhsRad);

                    if (sortingMode == SortingMode::FrontToBack)
                        return camera.getDistanceSquared(leftPos) < camera.getDistanceSquared(rightPos);
                    else if (sortingMode == SortingMode::BackToFront)
                        return camera.getDistanceSquared(leftPos) > camera.getDistanceSquared(rightPos);
                    else
                        return false;
                }
            );
        }
    }
#endif
}
void RenderPipeline::clean(const uint entityData) {
    for (auto& materialNode : materialNodes) {
        for (auto& meshNode : materialNode->meshNodes) {
            auto& instances = meshNode->instanceNodes;
            vector<Engine::epriv::InstanceNode*> newNodes;
            for (auto& instanceNode : instances) {
                auto entity = instanceNode->instance->parent();
                if (entity.data != entityData) {
                    newNodes.push_back(instanceNode);
                }
            }
            instances.clear();
            std::move(newNodes.begin(), newNodes.end(), std::back_inserter(instances));
        }
    }
}
void RenderPipeline::render(Viewport& viewport, Camera& camera, const double& dt, const bool useDefaultShaders, const SortingMode::Mode sortingMode) {
    if(useDefaultShaders) 
        shaderProgram.bind();
    for (auto& materialNode : materialNodes) {
        if (materialNode->meshNodes.size() > 0) {
            auto& _material = *materialNode->material;
            _material.update(dt);
            _material.bind();
            for (auto& meshNode : materialNode->meshNodes) {
                if (meshNode->instanceNodes.size() > 0) {
                    auto& _mesh = *meshNode->mesh;
                    _mesh.bind();
                    for (auto& instanceNode : meshNode->instanceNodes) {
                        auto& _modelInstance = *instanceNode->instance;
                        auto body = _modelInstance.parent().getComponent<ComponentBody>();
                        auto& model = *_modelInstance.parent().getComponent<ComponentModel>();

                        if (epriv::InternalModelInstancePublicInterface::IsViewportValid(_modelInstance, viewport)) {
                            if (body) {
                                const auto& radius = model.radius();
                                auto pos = body->position() + _modelInstance.position();
                                auto model = body->modelMatrix();
                                uint sphereTest = camera.sphereIntersectTest(pos, radius); //per mesh instance radius instead?
                                auto comparison = radius * 1100.0f;

                                if (!_modelInstance.visible() || sphereTest == 0 || camera.getDistanceSquared(pos) > comparison * comparison) { //optimization: using squared distance to remove the sqrt()
                                    _modelInstance.setPassedRenderCheck(false);
                                }else{
                                    _modelInstance.setPassedRenderCheck(true);
                                    if (sortingMode != SortingMode::None) {
                                        _mesh.sortTriangles(camera, _modelInstance, model, sortingMode);
                                    }
                                }
                            }else{
                                _modelInstance.setPassedRenderCheck(false);
                            }
                        }else{
                            _modelInstance.setPassedRenderCheck(false);
                        }
                        if (_modelInstance.passedRenderCheck()) {
                            _modelInstance.bind();
                            _mesh.render(false);
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
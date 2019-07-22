#include <core/engine/renderer/RenderGraph.h>
#include <core/ShaderProgram.h>
#include <core/engine/mesh/Mesh.h>
#include <core/Material.h>
#include <core/MeshInstance.h>
#include <core/engine/scene/Camera.h>

#include <glm/glm.hpp>

using namespace Engine;
using namespace std;

epriv::RenderPipeline::RenderPipeline(ShaderP& _shaderProgram) :shaderProgram(_shaderProgram) {
}
epriv::RenderPipeline::~RenderPipeline() {
    SAFE_DELETE_VECTOR(materialNodes);
}

float dist(const glm::vec3& lhs, const glm::vec3& rhs) {
    return glm::distance(lhs, rhs);
}
void epriv::RenderPipeline::sort_cheap(Camera& camera) {
    for (auto& materialNode : materialNodes) {
        for (auto& meshNode : materialNode->meshNodes) {
            auto& vect = meshNode->instanceNodes;
            std::sort(
                vect.begin(), vect.end(),
                [&camera](InstanceNode* lhs, InstanceNode* rhs) {
                    const glm::vec3& lhsPos = lhs->instance->parent().getComponent<ComponentBody>()->position();
                    const glm::vec3& rhsPos = rhs->instance->parent().getComponent<ComponentBody>()->position();
                    const glm::vec3& camPos = camera.getPosition();
                    return dist(camPos, lhsPos) < dist(camPos, rhsPos);
                }
            );
        }
    }
}
void epriv::RenderPipeline::sort(Camera& camera) {
    for (auto& materialNode : materialNodes) {
        for (auto& meshNode : materialNode->meshNodes) {
            auto& vect = meshNode->instanceNodes;
            std::sort(
                vect.begin(), vect.end(),
                [&camera](InstanceNode* lhs, InstanceNode* rhs) {
                    auto& lhsParent = lhs->instance->parent();
                    auto& rhsParent = rhs->instance->parent();
                    const EntityDataRequest& _dataReq1(lhsParent);
                    const EntityDataRequest& _dataReq2(rhsParent);

                    const glm::vec3& lhsPos = lhsParent.getComponent<ComponentBody>(_dataReq1)->position();
                    const glm::vec3& rhsPos = rhsParent.getComponent<ComponentBody>(_dataReq2)->position();
                    const float& lhsRad     = lhsParent.getComponent<ComponentModel>(_dataReq1)->radius();
                    const float& rhsRad     = rhsParent.getComponent<ComponentModel>(_dataReq2)->radius();

                    const glm::vec3& camPos   = camera.getPosition();
                    const glm::vec3& leftDir  = glm::normalize(lhsPos - camPos);
                    const glm::vec3& rightDir = glm::normalize(rhsPos - camPos);

                    const glm::vec3& leftPos  = lhsPos - (leftDir * lhsRad);
                    const glm::vec3& rightPos = rhsPos - (rightDir * rhsRad);

                    return dist(camPos, leftPos) < dist(camPos, rightPos);
                }
            );
        }
    }
}

void epriv::RenderPipeline::render(Camera& camera) {
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
                        auto& _meshInstance = *instanceNode->instance;
                        auto body = _meshInstance.parent().getComponent<ComponentBody>();
                        auto& model = *_meshInstance.parent().getComponent<ComponentModel>();
                        if (body) {
                            const auto& radius = model.radius();
                            auto pos = body->position() + _meshInstance.position();
                            uint sphereTest = camera.sphereIntersectTest(pos, radius); //per mesh instance radius instead?
                            auto comparison = radius * 1100.0f;
                            if (!_meshInstance.visible() || sphereTest == 0 || camera.getDistanceSquared(pos) > comparison * comparison) { //optimization: using squared distance to remove the sqrt()
                                _meshInstance.setPassedRenderCheck(false);
                            }else{
                                _meshInstance.setPassedRenderCheck(true);
                            }
                        }else{
                            _meshInstance.setPassedRenderCheck(false);
                        }
                        if (_meshInstance.passedRenderCheck()) {
                            _meshInstance.bind();
                            _mesh.render(false);
                            _meshInstance.unbind();
                        }
                    }
                    //protect against any custom changes by restoring to the regular shader and material
                    if (epriv::Core::m_Engine->m_RenderManager.glSM.current_bound_shader_program != &shaderProgram) {
                        shaderProgram.bind();
                        _material.bind();
                    }
                    _mesh.unbind();
                }
            }
        }
    }
}
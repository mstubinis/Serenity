#include <serenity/ecs/systems/SystemComponentTransformDebugDraw.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/system/Engine.h>
#include <serenity/resources/font/Font.h>
#include <serenity/scene/Scene.h>

SystemComponentTransformDebugDraw::SystemComponentTransformDebugDraw(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        #if defined(_DEBUG) || defined(ENGINE_FORCE_PHYSICS_DEBUG_DRAW)
        auto& system = static_cast<SystemComponentTransformDebugDraw&>(inSystem);
        system.forEach<Scene*>([](Scene* scene, Entity entity, ComponentTransform* transform, ComponentModel* model) {
            //ASSERT(scene && body && model, __FUNCTION__ << "(): parameter(s) was nullptr!");
            const auto worldMatrix = transform->getWorldMatrixRendering();
            const auto world_pos = Engine::Math::getMatrixPosition(worldMatrix);
            const auto world_rot = transform->getRotation();
            const auto world_scl = transform->getScale();
            for (size_t i = 0; i < model->getNumModels(); ++i) {
                auto& modelInstance = (*model)[i];

                auto rotation = world_rot * modelInstance.getRotation();
                auto fwd      = glm::normalize(Engine::Math::getForward(rotation)) * world_scl.x;
                auto right    = glm::normalize(Engine::Math::getRight(rotation)) * world_scl.y;
                auto up       = glm::normalize(Engine::Math::getUp(rotation)) * world_scl.z;

                Engine::Physics::drawDebugLine(world_pos, world_pos + fwd,   1.0f, 0.0f, 0.0f);
                Engine::Physics::drawDebugLine(world_pos, world_pos + right, 0.0f, 1.0f, 0.0f);
                Engine::Physics::drawDebugLine(world_pos, world_pos + up,    0.0f, 0.0f, 1.0f);

                auto& animationContainer = modelInstance.getRunningAnimations();

                for (size_t i = 0; i < animationContainer.getBoneVertexTransforms().size(); ++i) {
                    //if (animationContainer.getNodeData().m_Nodes[i].IsBone) {
                        auto bonePos = Engine::Math::getMatrixPosition(worldMatrix * animationContainer.getBoneLocalTransform(i));
                        Engine::Physics::drawDebugLine(bonePos, bonePos + (fwd * 0.01f), 1.0f, 1.0f, 0.0f);
                        Engine::Physics::drawDebugLine(bonePos, bonePos + (right * 0.01f), 0.0f, 1.0f, 1.0f);
                        Engine::Physics::drawDebugLine(bonePos, bonePos + (up * 0.01f), 1.0f, 0.0f, 1.0f);
                    //}
                }
            }
            /*
            const auto screenPos = transform->getScreenCoordinates(false);
            if (screenPos.z > 0) {
                const std::string text{ "ID: " + std::to_string(entity.id()) };
                Font::renderTextStatic(text, glm::vec2{ screenPos.x, screenPos.y }, glm::vec4{ 1.0f }, 0.0f, glm::vec2{ 0.5f }, 0.1f, TextAlignment::Left);
            }
            */
        }, &scene, SystemExecutionPolicy::Normal);
        #endif
    });  
}
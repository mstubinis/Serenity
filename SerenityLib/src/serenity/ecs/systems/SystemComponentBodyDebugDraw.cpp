#include <serenity/ecs/systems/SystemComponentBodyDebugDraw.h>
#include <serenity/ecs/components/ComponentBody.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/system/Engine.h>
#include <serenity/resources/font/Font.h>
#include <serenity/scene/Scene.h>

SystemComponentBodyDebugDraw::SystemComponentBodyDebugDraw(Engine::priv::ECS& ecs)
    : SystemCRTP{ ecs }
{
    setUpdateFunction([](SystemBaseClass& inSystem, const float dt, Scene& scene) {
        #if defined(_DEBUG) || defined(ENGINE_FORCE_PHYSICS_DEBUG_DRAW)
        auto& system = (SystemComponentBodyDebugDraw&)inSystem;
        system.forEach<Scene*>([](Scene* scene, Entity entity, ComponentBody* transform, ComponentModel* model) {
            //ASSERT(scene && body && model, __FUNCTION__ << "(): parameter(s) was nullptr!");
            const auto world_pos = glm::vec3{ transform->getPosition() };
            const auto world_rot = glm::quat{ transform->getRotation() };
            const auto world_scl = glm::vec3{ transform->getScale() };
            for (size_t i = 0; i < model->getNumModels(); ++i) {
                auto& modelInstance = (*model)[i];

                auto rotation = world_rot * modelInstance.orientation();
                auto fwd      = glm::normalize(Engine::Math::getForward(rotation)) * 0.3f;
                auto right    = glm::normalize(Engine::Math::getRight(rotation)) * 0.3f;
                auto up       = glm::normalize(Engine::Math::getUp(rotation)) * 0.3f;

                auto& physicsPipeline = Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline;
                physicsPipeline.drawLine(world_pos, (world_pos + fwd),   1.0f, 0.0f, 0.0f);
                physicsPipeline.drawLine(world_pos, (world_pos + right), 0.0f, 1.0f, 0.0f);
                physicsPipeline.drawLine(world_pos, (world_pos + up),    0.0f, 0.0f, 1.0f);
            }
            const auto screenPos = Engine::Math::getScreenCoordinates(world_pos, *scene->getActiveCamera(), false);
            //if (screenPos.z > 0) {
            //    const std::string text{ "ID: " + std::to_string(entity.id()) };
            //    Font::renderTextStatic(text, glm::vec2{ screenPos.x, screenPos.y }, glm::vec4{ 1.0f }, 0.0f, glm::vec2{ 0.5f }, 0.1f, TextAlignment::Left);
            //}
        }, &scene, SystemExecutionPolicy::Normal);
        #endif
    });  
}
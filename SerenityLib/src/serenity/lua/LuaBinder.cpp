#include <serenity/lua/LuaScript.h>
#include <LuaBridge/LuaBridge.h>
#include <serenity/lua/LuaModule.h>
#include <serenity/lua/LuaBinder.h>

#include <serenity/resources/Engine_Resources.h>
#include <serenity/resources/sound/SoundModule.h>

#include <serenity/scene/Scene.h>
#include <serenity/scene/SceneOptions.h>
#include <serenity/scene/Camera.h>
#include <serenity/scene/Viewport.h>
#include <serenity/scene/Skybox.h>

#include <serenity/ecs/entity/Entity.h>
#include <serenity/ecs/components/Components.h>

#include <serenity/lua/bindings/InputBindings.h>
#include <serenity/lua/bindings/MathBindings.h>
#include <serenity/lua/bindings/EventBindings.h>

using namespace Engine::priv;

namespace {
    void print(const std::string& s) { std::cout << s << '\n'; }
}
LUABinder::LUABinder()
    : m_LUA_STATE{ std::make_unique<LUAState>() }
{
    lua_State* L{ m_LUA_STATE->getState() };

    Engine::priv::lua::bindings::createBindingsEvents(L);
    Engine::priv::lua::bindings::createBindingsInput(L);
    Engine::priv::lua::bindings::createBindingsMath(L);

    luabridge::getGlobalNamespace(L)
        .addFunction("print", &print)

        .addFunction("registerScript", &Engine::lua::registerScript)

        .addFunction("getDeltaTime", &Engine::Resources::dt)
        .addFunction("dt", &Engine::Resources::dt)

        .addFunction("playSoundEffect", &Engine::Sound::playEffect)
        .addFunction("playSoundMusic", &Engine::Sound::playMusic)
        .addFunction("stopAllSoundEffects", &Engine::Sound::stop_all_effects)
        .addFunction("stopAllSoundMusic", &Engine::Sound::stop_all_music)

#pragma region LUAScriptInstance
        .beginClass<LUAScriptInstance>("LUAScriptInstance")
            .addFunction("addOnInitFunction", &LUAScriptInstance::addOnInitFunction)
            .addFunction("addOnUpdateFunction", &LUAScriptInstance::addOnUpdateFunction)
            .addFunction("addOnEventFunction", &LUAScriptInstance::addOnEventFunction)
            .addFunction("registerEvent", &LUAScriptInstance::registerEvent)
            .addFunction("unregisterEvent", &LUAScriptInstance::unregisterEvent)
        .endClass()
#pragma endregion

#pragma region Handle
        .beginClass<Handle>("Handle")
            .addProperty("index", &Handle::index)
            .addProperty("version", &Handle::version)
            .addProperty("type", &Handle::type)
        .endClass()
#pragma endregion

#pragma region Resources
        .addFunction("getResource", &Engine::priv::lua::resources::getResourceLUA)
#pragma endregion

#pragma region Event
        //event stuff
        .beginClass<Event>("Event")
            .addData("type", &Event::type, false)
            .addData("eventWindowResized", &Event::eventWindowResized, false)
            .addData("eventWindowFullscreenChanged", &Event::eventWindowFullscreenChanged, false)
            .addData("eventKeyboard", &Event::eventKeyboard, false)
            .addData("eventTextEntered", &Event::eventTextEntered, false)
            .addData("eventMouseButton", &Event::eventMouseButton, false)
            .addData("eventMouseMoved", &Event::eventMouseMoved, false)
            .addData("eventMouseWheel", &Event::eventMouseWheel, false)
            .addData("eventJoystickMoved", &Event::eventJoystickMoved, false)
            .addData("eventJoystickButton", &Event::eventJoystickButton, false)
            .addData("eventJoystickConnection", &Event::eventJoystickConnection, false)
            .addData("eventSoundStatusChanged", &Event::eventSoundStatusChanged, false)
            .addData("eventSceneChanged", &Event::eventSceneChanged, false)
            .addData("eventResource", &Event::eventResource, false)
            .addData("eventSocket", &Event::eventSocket, false)
            .addData("eventServer", &Event::eventServer, false)
            .addData("eventClient", &Event::eventClient, false)
        .endClass()
        .beginClass<Engine::priv::EventEnum>("EventEnum")
            .addData("enum", &Engine::priv::EventEnum::enumValue, false)
            //.addData("ptr", &Engine::priv::EventEnum::enumPtr, false)
        .endClass()
        .beginClass<Engine::priv::EventSocket>("EventSocket")
            .addData("localPort", &Engine::priv::EventSocket::localPort, false)
            .addData("remotePort", &Engine::priv::EventSocket::remotePort, false)
            .addData("type", &Engine::priv::EventSocket::type, false)
            //.addProperty("remoteIP", &Engine::priv::EventSocket::ipToString)
        .endClass()
        .beginClass<Engine::priv::EventServer>("EventServer")
        .endClass()
        .beginClass<Engine::priv::EventClient>("EventClient")
        .endClass()
        .beginClass<Engine::priv::EventResource>("EventResource")
        .endClass()
        .beginClass<Engine::priv::EventWindowResized>("EventWindowResized")
            .addData("width", &Engine::priv::EventWindowResized::width, false)
            .addData("height", &Engine::priv::EventWindowResized::height, false)
        .endClass()
        .beginClass<Engine::priv::EventWindowFullscreenChanged>("EventWindowFullscreenChanged")
            .addData("isFullscreen", &Engine::priv::EventWindowFullscreenChanged::isFullscreen, false)
        .endClass()
        .beginClass<Engine::priv::EventKeyboard>("EventKeyboard")
            .addData("key", &Engine::priv::EventKeyboard::key, false)
            .addData("alt", &Engine::priv::EventKeyboard::alt, false)
            .addData("shift", &Engine::priv::EventKeyboard::shift, false)
            .addData("system", &Engine::priv::EventKeyboard::system, false)
            .addData("control", &Engine::priv::EventKeyboard::control, false)
        .endClass()
        .beginClass<Engine::priv::EventTextEntered>("EventTextEntered")
            .addData("unicode", &Engine::priv::EventTextEntered::unicode, false)
            //.addProperty("text", &Engine::priv::EventTextEntered::convert, false)
        .endClass()
        .beginClass<Engine::priv::EventMouseButton>("EventMouseButton")
            .addData("button", &Engine::priv::EventMouseButton::button, false)
            .addData("x", &Engine::priv::EventMouseButton::x, false)
            .addData("y", &Engine::priv::EventMouseButton::y, false)
        .endClass()
        .beginClass<Engine::priv::EventMouseMove>("EventMouseMove")
            .addData("x", &Engine::priv::EventMouseMove::x, false)
            .addData("y", &Engine::priv::EventMouseMove::y, false)
        .endClass()
        .beginClass<Engine::priv::EventMouseWheel>("EventMouseWheel")
            .addData("delta", &Engine::priv::EventMouseWheel::delta, false)
            .addData("x", &Engine::priv::EventMouseWheel::x, false)
            .addData("y", &Engine::priv::EventMouseWheel::y, false)
        .endClass()
        .beginClass<Engine::priv::EventJoystickMoved>("EventJoystickMoved")
            .addData("id", &Engine::priv::EventJoystickMoved::joystickID, false)
            .addData("position", &Engine::priv::EventJoystickMoved::position, false)
            .addData("axis", &Engine::priv::EventJoystickMoved::axis, false)
        .endClass()
        .beginClass<Engine::priv::EventJoystickButton>("EventJoystickButton")
            .addData("id", &Engine::priv::EventJoystickButton::joystickID, false)
            .addData("button", &Engine::priv::EventJoystickButton::button, false)
        .endClass()
        .beginClass<Engine::priv::EventJoystickConnection>("EventJoystickConnection")
            .addData("id", &Engine::priv::EventJoystickConnection::joystickID, false)
        .endClass()
        .beginClass<Engine::priv::EventSoundStatusChanged>("EventSoundStatusChanged")
            .addData("status", &Engine::priv::EventSoundStatusChanged::status, false)
        .endClass()
        .beginClass<Engine::priv::EventSceneChanged>("EventSceneChanged")
        .endClass()
#pragma endregion

#pragma region Camera
//camera stuff
        .beginClass<Engine::priv::CameraLUABinder>("Camera")
            .addFunction("getAngle", &Engine::priv::CameraLUABinder::getAngle)
            .addFunction("setAngle", &Engine::priv::CameraLUABinder::setAngle)
            .addFunction("getAspectRatio", &Engine::priv::CameraLUABinder::getAspectRatio)
            .addFunction("setAspectRatio", &Engine::priv::CameraLUABinder::setAspectRatio)
            .addFunction("getNear", &Engine::priv::CameraLUABinder::getNear)
            .addFunction("setNear", &Engine::priv::CameraLUABinder::setNear)
            .addFunction("getFar", &Engine::priv::CameraLUABinder::getFar)
            .addFunction("setFar", &Engine::priv::CameraLUABinder::setFar)
            .addFunction("getPosition", &Engine::priv::CameraLUABinder::getPosition)
            .addFunction("getLocalPosition", &Engine::priv::CameraLUABinder::getLocalPosition)
            .addFunction("getRotation", &Engine::priv::CameraLUABinder::getRotation)
            .addFunction("getRight", &Engine::priv::CameraLUABinder::getRight)
            .addFunction("getUp", &Engine::priv::CameraLUABinder::getUp)
            .addFunction("getForward", &Engine::priv::CameraLUABinder::getForward)
            .addFunction("getView", &Engine::priv::CameraLUABinder::getView)
            .addFunction("setView", &Engine::priv::CameraLUABinder::setViewMatrix)
            .addFunction("getProjection", &Engine::priv::CameraLUABinder::getProjection)
            .addFunction("setProjection", &Engine::priv::CameraLUABinder::setProjectionMatrix)
        .endClass()
#pragma endregion

#pragma region Skybox
        //skybox stuff
        .beginClass<Skybox>("Skybox")

        .endClass()
#pragma endregion

#pragma region Viewport
        //viewport stuff
        .beginClass<Viewport>("Viewport")
            .addFunction("getId", &Viewport::getId)
            .addFunction("getScene", &Viewport::getScene)
            .addFunction("getCamera", &Viewport::getCamera)
            .addFunction("setCamera", &Viewport::setCamera)
            //.addFunction("setID", &Viewport::setID)
            .addFunction("activate", &Viewport::activate)
            .addFunction("activateDepthMask", &Viewport::activateDepthMask)
            .addFunction("setDepthMaskValue", &Viewport::setDepthMaskValue)
            .addFunction("setAspectRatioSynced", &Viewport::setAspectRatioSynced)
            .addFunction("isAspectRatioSynced", &Viewport::isAspectRatioSynced)
            .addFunction("isDepthMaskActive", &Viewport::isDepthMaskActive)
            .addFunction("getBackgroundColor", &Viewport::getBackgroundColor)
            .addFunction("setBackgroundColor", static_cast<void(Viewport::*)(float, float, float, float)>(&Viewport::setBackgroundColor))
            .addFunction("setViewportDimensions", &Viewport::setViewportDimensions)
            .addFunction("getViewportDimensions", &Viewport::getViewportDimensions)
        .endClass()
#pragma endregion

#pragma region Scene
        //scene stuff
        .beginClass<Engine::priv::SceneLUABinder>("Scene")
            .addFunction("setName", &Engine::priv::SceneLUABinder::setName)
            .addFunction("createEntity", &Engine::priv::SceneLUABinder::createEntity)
            .addFunction("createEntities", &Engine::priv::SceneLUABinder::createEntity)
            .addFunction("id", &Engine::priv::SceneLUABinder::id)
            .addFunction("setActiveCamera", &Engine::priv::SceneLUABinder::setActiveCamera)
            .addFunction("getActiveCamera", &Engine::priv::SceneLUABinder::getActiveCamera)
            //.addFunction("centerSceneToObject", &Engine::priv::SceneLUABinder::centerSceneToObject)
            //.addFunction("getBackgroundColor", &Engine::priv::SceneLUABinder::getBackgroundColor)
            //.addFunction("setBackgroundColor", &Engine::priv::SceneLUABinder::setBackgroundColor)
            //.addFunction("setGlobalIllumination", &Engine::priv::SceneLUABinder::setGlobalIllumination)
            //.addFunction("getGlobalIllumination", &Engine::priv::SceneLUABinder::getGlobalIllumination)
        .endClass()
        .addFunction("getCurrentScene", &Engine::priv::lua::resources::getCurrentSceneLUA)
        .addFunction("setCurrentScene", static_cast<void(*)(Engine::priv::SceneLUABinder)>(&Engine::priv::lua::resources::setCurrentSceneLUA))
         //TODO: same name?
        .addFunction("setCurrentSceneByName", static_cast<void(*)(const std::string&)>(&Engine::priv::lua::resources::setCurrentSceneByNameLUA))
#pragma endregion

#pragma region Entity
        //entity stuff
        .beginClass<Entity>("Entity")
            .addFunction("scene", &Entity::scene)
            .addFunction("sceneID", &Entity::sceneID)
            .addFunction("id", &Entity::id)
            .addFunction("isNull", &Entity::isNull)
            .addFunction("destroy", &Entity::destroy)
            .addFunction("hasParent", &Entity::hasParent)
            .addFunction("versionID", &Entity::versionID)
            .addFunction("addChild", &Entity::addChild)
            .addFunction("removeChild", &Entity::removeChild)
            .addFunction("removeComponent", static_cast<bool(Entity::*)(const std::string&)>(&Entity::removeComponent))
            .addFunction("getComponent", static_cast<luabridge::LuaRef(Entity::*)(luabridge::LuaRef)>(&Entity::getComponent))
            .addFunction("addComponent", static_cast<bool(Entity::*)(const std::string&, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef)>(&Entity::addComponent))
        .endClass()
#pragma endregion

#pragma region ComponentName
        .beginClass<Engine::priv::ComponentNameLUABinder>("ComponentName")
            .addFunction("name", &Engine::priv::ComponentNameLUABinder::name)
            .addFunction("size", &Engine::priv::ComponentNameLUABinder::size)
            .addFunction("empty", &Engine::priv::ComponentNameLUABinder::empty)
        .endClass()
#pragma endregion

#pragma region ComponentRigidBody
        .beginClass<Engine::priv::ComponentRigidBodyLUABinder>("ComponentRigidBody")

        .endClass()
#pragma endregion

#pragma region ComponentTransform
        .beginClass<Engine::priv::ComponentTransformLUABinder>("ComponentTransform")
            .addFunction("addChild", &Engine::priv::ComponentTransformLUABinder::addChild)
            .addFunction("removeChild", &Engine::priv::ComponentTransformLUABinder::removeChild)
            .addFunction("hasParent", &Engine::priv::ComponentTransformLUABinder::hasParent)

            .addFunction("setPosition", &Engine::priv::ComponentTransformLUABinder::setPosition)
            .addFunction("setScale",    &Engine::priv::ComponentTransformLUABinder::setScale)
            .addFunction("setRotation", &Engine::priv::ComponentTransformLUABinder::setRotation)

            .addFunction("setLocalPosition", &Engine::priv::ComponentTransformLUABinder::setLocalPosition)

            .addFunction("translate", &Engine::priv::ComponentTransformLUABinder::translate)
            .addFunction("scale", &Engine::priv::ComponentTransformLUABinder::scale)
            .addFunction("rotate", &Engine::priv::ComponentTransformLUABinder::rotate)
            .addFunction("getDistance", &Engine::priv::ComponentTransformLUABinder::getDistance)
            //.addFunction("getScreenCoordinates", &Engine::priv::ComponentTransformLUABinder::getScreenCoordinates)
            .addFunction("alignTo", &Engine::priv::ComponentTransformLUABinder::alignTo)

            .addFunction("getRotation", &Engine::priv::ComponentTransformLUABinder::getRotation)
            .addFunction("getPosition", &Engine::priv::ComponentTransformLUABinder::getPosition)
            .addFunction("getScale", &Engine::priv::ComponentTransformLUABinder::getScale)
            .addFunction("getLocalPosition", &Engine::priv::ComponentTransformLUABinder::getLocalPosition)
            .addFunction("getForward", &Engine::priv::ComponentTransformLUABinder::getForward)
            .addFunction("getRight", &Engine::priv::ComponentTransformLUABinder::getRight)
            .addFunction("getUp", &Engine::priv::ComponentTransformLUABinder::getUp)
            //.addFunction("getLinearVelocity", &Engine::priv::ComponentTransformLUABinder::getLinearVelocity)
            .addFunction("getWorldMatrix", &Engine::priv::ComponentTransformLUABinder::getWorldMatrix)
            .addFunction("getWorldMatrixRendering", &Engine::priv::ComponentTransformLUABinder::getWorldMatrixRendering)
            .addFunction("getLocalMatrix", &Engine::priv::ComponentTransformLUABinder::getLocalMatrix)

            //.addFunction("setLinearVelocity", &Engine::priv::ComponentTransformLUABinder::setLinearVelocity)

        .endClass()
#pragma endregion

#pragma region ComponentCamera
        .beginClass<Engine::priv::ComponentCameraLUABinder>("ComponentCamera")
            //.addFunction("getRight", &Engine::priv::ComponentCameraLUABinder::getRight)
            //.addFunction("getUp", &Engine::priv::ComponentCameraLUABinder::getUp)

            //.addFunction("getAngle", &Engine::priv::ComponentCameraLUABinder::getAngle)
            //.addFunction("getAspectRatio", &Engine::priv::ComponentCameraLUABinder::getAspectRatio)
            //.addFunction("getNear", &Engine::priv::ComponentCameraLUABinder::getNear)
            //.addFunction("getFar", &Engine::priv::ComponentCameraLUABinder::getFar)
/*
            .addFunction("getView", &Engine::priv::ComponentCameraLUABinder::getView)
            .addFunction("getProjection", &Engine::priv::ComponentCameraLUABinder::getProjection)
            .addFunction("getViewInverse", &Engine::priv::ComponentCameraLUABinder::getViewInverse)
            .addFunction("getProjectionInverse", &Engine::priv::ComponentCameraLUABinder::getProjectionInverse)

            .addFunction("getViewProjection", &Engine::priv::ComponentCameraLUABinder::getViewProjection)
            .addFunction("getViewProjectionInverse", &Engine::priv::ComponentCameraLUABinder::getViewProjectionInverse)
            .addFunction("getViewVector", &Engine::priv::ComponentCameraLUABinder::getViewVector)
*/


        .endClass()
#pragma endregion

#pragma region ComponentLogic
        //component logics
        .beginClass<Engine::priv::ComponentLogicLUABinder>("ComponentLogic")
            //.addFunction("call", &Engine::priv::ComponentLogicLUABinder::call)
            //.addFunction("setFunctor", static_cast<void(ComponentLogic::*)(luabridge::LuaRef)>(&Engine::priv::ComponentLogicLUABinder::setFunctor))
        .endClass()
#pragma endregion

#pragma region ComponentLogic1
        .beginClass<Engine::priv::ComponentLogic1LUABinder>("ComponentLogic1")
            //.addFunction("call", &Engine::priv::ComponentLogic1LUABinder::call)
            //.addFunction("setFunctor", static_cast<void(ComponentLogic1::*)(luabridge::LuaRef)>(&Engine::priv::ComponentLogic1LUABinder::setFunctor))
        .endClass()
#pragma endregion

#pragma region ComponentLogic2
        .beginClass<Engine::priv::ComponentLogic2LUABinder>("ComponentLogic2")
            //.addFunction("call", &Engine::priv::ComponentLogic2LUABinder::call)
            //.addFunction("setFunctor", static_cast<void(ComponentLogic2::*)(luabridge::LuaRef)>(&Engine::priv::ComponentLogic2LUABinder::setFunctor))
        .endClass()
#pragma endregion

#pragma region ComponentLogic3
        .beginClass<Engine::priv::ComponentLogic3LUABinder>("ComponentLogic3")
            //.addFunction("call", &Engine::priv::ComponentLogic3LUABinder::call)
            //.addFunction("setFunctor", static_cast<void(ComponentLogic3::*)(luabridge::LuaRef)>(&Engine::priv::ComponentLogic3LUABinder::setFunctor))
        .endClass()
#pragma endregion

#pragma region ComponentModel
        .beginClass<Engine::priv::ComponentModelLUABinder>("ComponentModel")
            /*
            .addFunction("getBoundingBox", &Engine::priv::ComponentModelLUABinder::getBoundingBox)
            .addFunction("getRadius", &Engine::priv::ComponentModelLUABinder::getRadius)
            .addFunction("getNumModels", &Engine::priv::ComponentModelLUABinder::getNumModels)
            .addFunction("setStage", &Engine::priv::ComponentModelLUABinder::setStage)
            .addFunction("getOwner", &Engine::priv::ComponentModelLUABinder::getOwner)
            .addFunction("removeModel", &Engine::priv::ComponentModelLUABinder::removeModel)
            .addFunction("hide", &Engine::priv::ComponentModelLUABinder::hide)
            .addFunction("show", &Engine::priv::ComponentModelLUABinder::show)
            .addFunction("getModel", &Engine::priv::ComponentModelLUABinder::getModel)
            */
        .endClass()
#pragma endregion

#pragma region ComponentScript
        .beginClass<Engine::priv::ComponentScriptLUABinder>("ComponentScript")

        .endClass()
#pragma endregion

#pragma region ModelInstance
        .beginClass<ModelInstance>("ModelInstance")
            .addFunction("getColor", &ModelInstance::getColor)
            .addFunction("setColor", static_cast<void(ModelInstance::*)(const float, const float, const float, const float)>(&ModelInstance::setColor))
            .addFunction("forceRender", &ModelInstance::forceRender)
            .addFunction("getScale", &ModelInstance::getScale)
            .addFunction("setScale", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::setScale))
            .addFunction("rotate", static_cast<void(ModelInstance::*)(const float, const float, const float, bool)>(&ModelInstance::rotate))
            .addFunction("translate", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::translate))
            .addFunction("scale", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::scale))
            .addFunction("getPosition", &ModelInstance::getPosition)
            .addFunction("getRotation", &ModelInstance::getRotation)
            .addFunction("getGodRaysColor", &ModelInstance::getGodRaysColor)
            .addFunction("setGodRaysColor", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::setGodRaysColor))
            .addFunction("getOwner", &ModelInstance::getOwner)
            .addFunction("hasPassedRenderCheck", &ModelInstance::hasPassedRenderCheck)
            .addFunction("hide", &ModelInstance::hide)
            .addFunction("show", &ModelInstance::show)
            .addFunction("isVisible", &ModelInstance::isVisible)
            .addFunction("getIndex", &ModelInstance::getIndex)
            .addFunction("isForceRendered", &ModelInstance::isForceRendered)
            .addFunction("playAnimation", static_cast<void(ModelInstance::*)(std::string_view, float, float, uint32_t)>(&ModelInstance::playAnimation))
            .addFunction("setOrientation", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::setOrientation))
        .endClass()
#pragma endregion

    ;
}

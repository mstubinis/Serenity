#include <serenity/lua/LuaScript.h>
#include <LuaBridge/LuaBridge.h>

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

using namespace Engine::priv;

void print(const std::string& s) { std::cout << s << "\n"; }
void tostring(luabridge::LuaRef r) { std::cout << r.tostring() << "\n"; }

LUABinder::LUABinder()
    : m_LUA_STATE{ std::make_unique<LUAState>() }
{
    lua_State* L{ m_LUA_STATE->getState() };
    luabridge::getGlobalNamespace(L)
        .addFunction("print", &print)
        .addFunction("tostring", &tostring)
        .addFunction("getDeltaTime", &Engine::Resources::dt)
        .addFunction("dt", &Engine::Resources::dt)

        .addFunction("playSoundEffect", &Engine::Sound::playEffect)
        .addFunction("playSoundMusic", &Engine::Sound::playMusic)
        .addFunction("stopAllSoundEffects", &Engine::Sound::stop_all_effects)
        .addFunction("stopAllSoundMusic", &Engine::Sound::stop_all_music)

#pragma region Matrices
        //glm mat4 TODO: add more to this
        .beginClass<glm::mat4>("mat4")

        .endClass()
        //glm mat3 TODO: add more to this
        .beginClass<glm::mat3>("mat3")

        .endClass()
        //glm mat2 TODO: add more to this
        .beginClass<glm::mat2>("mat2")

        .endClass()
#pragma endregion

#pragma region Vectors



        //glm ivec2 TODO: add more to this
        .beginClass<glm::ivec2>("ivec2")
            .addConstructor<void(*)(int, int)>()
            .addData("x", &glm::ivec2::x)
            .addData("y", &glm::ivec2::y)
        .endClass()
        //glm ivec3 TODO: add more to this
        .beginClass<glm::ivec3>("ivec3")
            .addConstructor<void(*)(int, int, int)>()
            .addData("x", &glm::ivec3::x)
            .addData("y", &glm::ivec3::y)
            .addData("z", &glm::ivec3::z)
        .endClass()
        //glm ivec4 TODO: add more to this
        .beginClass<glm::ivec4>("ivec4")
            .addConstructor<void(*)(int, int, int, int)>()
            .addData("x", &glm::ivec4::x)
            .addData("y", &glm::ivec4::y)
            .addData("z", &glm::ivec4::z)
            .addData("w", &glm::ivec4::w)
        .endClass()




        //glm dvec2 TODO: add more to this
        .beginClass<glm::dvec2>("dvec2")
            .addConstructor<void(*)(double, double)>()
            .addData("x", &glm::dvec2::x)
            .addData("y", &glm::dvec2::y)
        .endClass()
        //glm dvec3 TODO: add more to this
        .beginClass<glm::dvec3>("dvec3")
            .addConstructor<void(*)(double, double, double)>()
            .addData("x", &glm::dvec3::x)
            .addData("y", &glm::dvec3::y)
            .addData("z", &glm::dvec3::z)
        .endClass()
        //glm dvec4 TODO: add more to this
        .beginClass<glm::dvec4>("dvec4")
            .addConstructor<void(*)(double, double, double, double)>()
            .addData("x", &glm::dvec4::x)
            .addData("y", &glm::dvec4::y)
            .addData("z", &glm::dvec4::z)
            .addData("w", &glm::dvec4::w)
        .endClass()




        //glm vec2 TODO: add more to this
        .beginClass<glm::vec2>("fvec2")
            .addConstructor<void(*)(float, float)>()
            .addData("x", &glm::vec2::x)
            .addData("y", &glm::vec2::y)
        .endClass()
        //glm vec3 TODO: add more to this
        .beginClass<glm::vec3>("fvec3")
            .addConstructor<void(*)(float, float, float)>()
            .addData("x", &glm::vec3::x)
            .addData("y", &glm::vec3::y)
            .addData("z", &glm::vec3::z)
        .endClass()
        //glm vec4 TODO: add more to this
        .beginClass<glm::vec4>("fvec4")
            .addConstructor<void(*)(float, float, float, float)>()
            .addData("x", &glm::vec4::x)
            .addData("y", &glm::vec4::y)
            .addData("z", &glm::vec4::z)
            .addData("w", &glm::vec4::w)
        .endClass()

        //glm vec2 TODO: add more to this
        .beginClass<glm_vec2>("vec2")
            .addConstructor<void(*)(decimal, decimal)>()
            .addData("x", &glm_vec2::x)
            .addData("y", &glm_vec2::y)
        .endClass()
        //glm vec3 TODO: add more to this
        .beginClass<glm_vec3>("vec3")
            .addConstructor<void(*)(decimal, decimal, decimal)>()
            .addData("x", &glm_vec3::x)
            .addData("y", &glm_vec3::y)
            .addData("z", &glm_vec3::z)
        .endClass()
        //glm vec4 TODO: add more to this
        .beginClass<glm_vec4>("vec4")
            .addConstructor<void(*)(decimal, decimal, decimal, decimal)>()
            .addData("x", &glm_vec4::x)
            .addData("y", &glm_vec4::y)
            .addData("z", &glm_vec4::z)
            .addData("w", &glm_vec4::w)
        .endClass()
        //bullet vector3 TODO: add more to this
        .beginClass<btVector3>("btVector3")
            .addConstructor<void(*)(const btScalar&, const btScalar&, const btScalar&)>()
            .addProperty("x", &btVector3::x, &btVector3::setX)
            .addProperty("y", &btVector3::y, &btVector3::setY)
            .addProperty("z", &btVector3::z, &btVector3::setZ)
            .addProperty("w", &btVector3::w, &btVector3::setW)
            .addFunction("angle", &btVector3::angle)
            .addFunction("cross", &btVector3::cross)
            .addFunction("absolute", &btVector3::absolute)
            .addFunction("closestAxis", &btVector3::closestAxis)
            .addFunction("dot", &btVector3::dot)
            .addFunction("distance", &btVector3::distance)
            .addFunction("distance2", &btVector3::distance2)
            .addFunction("isZero", &btVector3::isZero)
            .addFunction("rotate", &btVector3::rotate)
            .addFunction("triple", &btVector3::triple)
            .addFunction("dot3", &btVector3::dot3)
            .addFunction("lerp", &btVector3::lerp)
            .addFunction("minAxis", &btVector3::minAxis)
            .addFunction("maxAxis", &btVector3::maxAxis)
            .addFunction("normalize",&btVector3::normalize)
            .addFunction("norm", &btVector3::norm)
            .addFunction("normalized", &btVector3::normalized)
            .addFunction("furthestAxis",&btVector3::furthestAxis)
        .endClass()
#pragma endregion

        //camera stuff
        .beginClass<Camera>("Camera")
            .addFunction("getAngle", &Camera::getAngle)
            .addFunction("setAngle", &Camera::setAngle)
            .addFunction("getAspectRatio", &Camera::getAspectRatio)
            .addFunction("setAspectRatio", &Camera::setAspectRatio)
            .addFunction("getNear", &Camera::getNear)
            .addFunction("setNear", &Camera::setNear)
            .addFunction("getFar", &Camera::getFar)
            .addFunction("setFar", &Camera::setFar)
            .addFunction("getView", &Camera::getView)
            .addFunction("setView", &Camera::setViewMatrix)
            .addFunction("getProjection", &Camera::getProjection)
            .addFunction("setProjection", &Camera::setProjectionMatrix)
        .endClass()
        //skybox stuff
        .beginClass<Skybox>("Skybox")

        .endClass()
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
            .addFunction("setBackgroundColor", &Viewport::setBackgroundColor)
            .addFunction("setViewportDimensions", &Viewport::setViewportDimensions)
            .addFunction("getViewportDimensions", &Viewport::getViewportDimensions)
        .endClass()
        //scene stuff
        .beginClass<Scene>("Scene")
            //.addProperty("name", &Scene::name, &Scene::setName)
            .addFunction("setName", static_cast<void(Scene::*)(std::string_view)>(&Scene::setName))
            .addFunction("createEntity", &Scene::createEntity)
            .addFunction("id", &Scene::id)
            .addFunction("setActiveCamera", &Scene::setActiveCamera)
            .addFunction("getActiveCamera", &Scene::getActiveCamera)
            .addFunction("centerSceneToObject", &Scene::centerSceneToObject)
            .addFunction("getBackgroundColor", &Scene::getBackgroundColor)
            .addFunction("setBackgroundColor", static_cast<void(Scene::*)(float, float, float, float)>(&Scene::setBackgroundColor))
            .addFunction("setGlobalIllumination", static_cast<void(Scene::*)(float, float, float)>(&Scene::setGlobalIllumination))
            .addFunction("getGlobalIllumination", &Scene::getGlobalIllumination)
        .endClass()
        .addFunction("getCurrentScene", &Engine::Resources::getCurrentScene)
        .addFunction("setCurrentScene", static_cast<bool(*)(Scene*)>(&Engine::Resources::setCurrentScene))
        .addFunction("setCurrentSceneByName", static_cast<bool(*)(std::string_view)>(&Engine::Resources::setCurrentScene))
        //entity stuff
        .beginClass<Entity>("Entity")
            .addFunction("scene", &Entity::scene)
            .addFunction("sceneID", &Entity::sceneID)
            .addFunction("id", &Entity::id)
            .addFunction("null", &Entity::null)
            .addFunction("destroy", &Entity::destroy)
            .addFunction("hasParent", &Entity::hasParent)
            .addFunction("versionID", &Entity::versionID)
            .addFunction("addChild", &Entity::addChild)
            .addFunction("removeChild", &Entity::removeChild)
            .addFunction("removeComponent", static_cast<bool(Entity::*)(std::string_view)>(&Entity::removeComponent))
            .addFunction("getComponent", static_cast<luabridge::LuaRef(Entity::*)(std::string_view)>(&Entity::getComponent))
            .addFunction("addComponent", static_cast<bool(Entity::*)(std::string_view, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef)>(&Entity::addComponent))
        .endClass()
        //component name
        .beginClass<ComponentName>("ComponentName")
            .addFunction("name", &ComponentName::name)
            .addFunction("size", &ComponentName::size)
            .addFunction("empty", &ComponentName::empty)
        .endClass()
        //component body
        //TODO: add component rigid body
        .beginClass<ComponentTransform>("ComponentTransform")
            .addFunction("addChild", static_cast<void(ComponentTransform::*)(Entity)const>(&ComponentTransform::addChild))
            .addFunction("removeChild", static_cast<void(ComponentTransform::*)(Entity)const>(&ComponentTransform::removeChild))
            .addFunction("hasParent", &ComponentTransform::hasParent)

            .addFunction("setPosition", static_cast<void(ComponentTransform::*)(decimal, decimal, decimal)>(&ComponentTransform::setPosition))
            .addFunction("setScale",    static_cast<void(ComponentTransform::*)(float, float, float)>(&ComponentTransform::setScale))
            .addFunction("setRotation", static_cast<void(ComponentTransform::*)(float, float, float, float)>(&ComponentTransform::setRotation))

            .addFunction("translate", static_cast<void(ComponentTransform::*)(decimal, decimal, decimal, bool)>(&ComponentTransform::translate))
            .addFunction("scale", static_cast<void(ComponentTransform::*)(float, float, float)>(&ComponentTransform::scale))
            .addFunction("rotate", static_cast<void(ComponentTransform::*)(float, float, float, bool)>(&ComponentTransform::rotate))
            .addFunction("getDistance", &ComponentTransform::getDistance)
            .addFunction("getScreenCoordinates", &ComponentTransform::getScreenCoordinates)
            .addFunction("alignTo", static_cast<void(ComponentTransform::*)(const glm::vec3&)>(&ComponentTransform::alignTo))
            .addFunction("alignTo", static_cast<void(ComponentTransform::*)(float, float, float)>(&ComponentTransform::alignTo))

            .addFunction("getRotation", &ComponentTransform::getRotation)
            .addFunction("getPosition", &ComponentTransform::getPosition)
            .addFunction("getScale", &ComponentTransform::getScale)
            .addFunction("getLocalPosition", &ComponentTransform::getLocalPosition)
            .addFunction("getForward", &ComponentTransform::getForward)
            .addFunction("getRight", &ComponentTransform::getRight)
            .addFunction("getUp", &ComponentTransform::getUp)
            //.addFunction("getLinearVelocity", &ComponentTransform::getLinearVelocity)
            .addFunction("getWorldMatrix", &ComponentTransform::getWorldMatrix)
            .addFunction("getWorldMatrixRendering", &ComponentTransform::getWorldMatrixRendering)
            .addFunction("getLocalMatrix", &ComponentTransform::getLocalMatrix)

            //.addFunction("setLinearVelocity", static_cast<void(ComponentTransform::*)(decimal, decimal, decimal, bool)>(&ComponentTransform::setLinearVelocity))

        .endClass()
        //component camera
        .beginClass<ComponentCamera>("ComponentCamera")
            .addFunction("getRight", &ComponentCamera::getRight)
            .addFunction("getUp", &ComponentCamera::getUp)

            .addFunction("getAngle", &ComponentCamera::getAngle)
            .addFunction("getAspectRatio", &ComponentCamera::getAspectRatio)
            .addFunction("getNear", &ComponentCamera::getNear)
            .addFunction("getFar", &ComponentCamera::getFar)
/*
            .addFunction("getView", &ComponentCamera::getView)
            .addFunction("getProjection", &ComponentCamera::getProjection)
            .addFunction("getViewInverse", &ComponentCamera::getViewInverse)
            .addFunction("getProjectionInverse", &ComponentCamera::getProjectionInverse)

            .addFunction("getViewProjection", &ComponentCamera::getViewProjection)
            .addFunction("getViewProjectionInverse", &ComponentCamera::getViewProjectionInverse)
            .addFunction("getViewVector", &ComponentCamera::getViewVector)
*/


        .endClass()
        //component logics
        .beginClass<ComponentLogic>("ComponentLogic")
            .addFunction("call", &ComponentLogic::call)
            .addFunction("setFunctor", static_cast<void(ComponentLogic::*)(luabridge::LuaRef)>(&ComponentLogic::setFunctor))
        .endClass()
        .beginClass<ComponentLogic1>("ComponentLogic1")
            .addFunction("call", &ComponentLogic1::call)
            .addFunction("setFunctor", static_cast<void(ComponentLogic1::*)(luabridge::LuaRef)>(&ComponentLogic1::setFunctor))
        .endClass()
        .beginClass<ComponentLogic2>("ComponentLogic2")
            .addFunction("call", &ComponentLogic2::call)
            .addFunction("setFunctor", static_cast<void(ComponentLogic2::*)(luabridge::LuaRef)>(&ComponentLogic2::setFunctor))
        .endClass()
        .beginClass<ComponentLogic3>("ComponentLogic3")
            .addFunction("call", &ComponentLogic3::call)
            .addFunction("setFunctor", static_cast<void(ComponentLogic3::*)(luabridge::LuaRef)>(&ComponentLogic3::setFunctor))
        .endClass()
        //component model TODO add more
        .beginClass<ComponentModel>("ComponentModel")
            .addFunction("getBoundingBox", &ComponentModel::getBoundingBox)
            .addFunction("getRadius", &ComponentModel::getRadius)
            .addFunction("getNumModels", &ComponentModel::getNumModels)
            .addFunction("setStage", &ComponentModel::setStage)
            .addFunction("getOwner", &ComponentModel::getOwner)
            .addFunction("removeModel", &ComponentModel::removeModel)
            .addFunction("hide", &ComponentModel::hide)
            .addFunction("show", &ComponentModel::show)
            .addFunction("getModel", &ComponentModel::getModel)
        .endClass()
        //model instance
        .beginClass<ModelInstance>("ModelInstance")
            .addFunction("getColor", &ModelInstance::getColor)
            .addFunction("setColor", static_cast<void(ModelInstance::*)(const float, const float, const float, const float)>(&ModelInstance::setColor))
            .addFunction("forceRender", &ModelInstance::forceRender)
            .addFunction("getScale", &ModelInstance::getScale)
            .addFunction("setScale", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::setScale))
            .addFunction("rotate", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::rotate))
            .addFunction("translate", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::translate))
            .addFunction("scale", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::scale))
            .addFunction("getPosition", &ModelInstance::getPosition)
            .addFunction("getRotation", &ModelInstance::getRotation)
            .addFunction("getGodRaysColor", &ModelInstance::getGodRaysColor)
            .addFunction("setGodRaysColor", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::setGodRaysColor))
            .addFunction("getParent", &ModelInstance::getParent)
            .addFunction("hasPassedRenderCheck", &ModelInstance::hasPassedRenderCheck)
            .addFunction("hide", &ModelInstance::hide)
            .addFunction("show", &ModelInstance::show)
            .addFunction("isVisible", &ModelInstance::isVisible)
            .addFunction("getIndex", &ModelInstance::getIndex)
            .addFunction("isForceRendered", &ModelInstance::isForceRendered)
            .addFunction("playAnimation", static_cast<void(ModelInstance::*)(std::string_view, float, float, uint32_t)>(&ModelInstance::playAnimation))
            .addFunction("setOrientation", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::setOrientation))
        .endClass()
    ;
}

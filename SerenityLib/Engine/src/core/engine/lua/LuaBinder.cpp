#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lua/LuaScript.h>
#include <LuaBridge/LuaBridge.h>

#include <core/engine/lua/LuaBinder.h>
#include <core/engine/lua/LuaState.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/sounds/SoundModule.h>

#include <core/engine/scene/Scene.h>
#include <core/engine/scene/SceneOptions.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/scene/Skybox.h>

#include <ecs/Entity.h>
#include <ecs/Components.h>

using namespace std;
using namespace Engine::priv;

void print(const string& s) { cout << s << "\n"; }
void tostring(luabridge::LuaRef r) { cout << r.tostring() << "\n"; }

LUABinder::LUABinder() {
    m_LUA_STATE = NEW LUAState();
    lua_State* L = m_LUA_STATE->getState();
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
            .addProperty("angle", &Camera::getAngle, &Camera::setAngle)
            .addProperty("aspectRatio", &Camera::getAspect, &Camera::setAspect)
            .addProperty("near", &Camera::getNear, &Camera::setNear)
            .addProperty("far", &Camera::getFar, &Camera::setFar)
            .addProperty("projection",&Camera::getProjection, &Camera::setProjectionMatrix)
            .addProperty("view", &Camera::getView, &Camera::setViewMatrix)
        .endClass()
        //skybox stuff
        .beginClass<Skybox>("Skybox")

        .endClass()
        //viewport stuff
        .beginClass<Viewport>("Viewport")
            .addFunction("id", &Viewport::id)
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
            .addFunction("setName", &Scene::setName)
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
        .addFunction("setCurrentSceneByName", static_cast<bool(*)(string_view)>(&Engine::Resources::setCurrentScene))
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
            .addFunction("removeComponent", static_cast<bool(Entity::*)(const string&)>(&Entity::removeComponent))
            .addFunction("getComponent", static_cast<luabridge::LuaRef(Entity::*)(const string&)>(&Entity::getComponent))
            .addFunction("addComponent", &Entity::addComponent)
        .endClass()
        //component name
        .beginClass<ComponentName>("ComponentName")
            .addFunction("name", &ComponentName::name)
            .addFunction("size", &ComponentName::size)
            .addFunction("empty", &ComponentName::empty)
        .endClass()
        //component body
        .beginClass<ComponentBody>("ComponentBody")
            .addFunction("addChild", static_cast<void(ComponentBody::*)(Entity)const>(&ComponentBody::addChild))
            .addFunction("removeChild", static_cast<void(ComponentBody::*)(Entity)const>(&ComponentBody::removeChild))
            //.addFunction("addChildBody", static_cast<void(ComponentBody::*)(const ComponentBody&)const>(&ComponentBody::addChild))
            //.addFunction("removeChildBody", static_cast<void(ComponentBody::*)(const ComponentBody&)const>(&ComponentBody::removeChild))
            .addFunction("hasParent", &ComponentBody::hasParent)
            .addFunction("hasPhysics", &ComponentBody::hasPhysics)

            .addFunction("setPosition", static_cast<void(ComponentBody::*)(decimal, decimal, decimal)>(&ComponentBody::setPosition))
            .addFunction("setScale",    static_cast<void(ComponentBody::*)(decimal, decimal, decimal)>(&ComponentBody::setScale))
            .addFunction("setRotation", static_cast<void(ComponentBody::*)(decimal, decimal, decimal, decimal)>(&ComponentBody::setRotation))

            .addFunction("translate", static_cast<void(ComponentBody::*)(decimal, decimal, decimal, bool)>(&ComponentBody::translate))
            .addFunction("scale", static_cast<void(ComponentBody::*)(decimal, decimal, decimal)>(&ComponentBody::scale))
            .addFunction("rotate", static_cast<void(ComponentBody::*)(decimal, decimal, decimal, bool)>(&ComponentBody::rotate))
            .addFunction("mass", &ComponentBody::mass)
            .addFunction("getDistance", &ComponentBody::getDistance)
            .addFunction("getScreenCoordinates", &ComponentBody::getScreenCoordinates)
            .addFunction("alignTo", static_cast<void(ComponentBody::*)(const glm_vec3&)>(&ComponentBody::alignTo))
            .addFunction("alignTo", static_cast<void(ComponentBody::*)(decimal, decimal, decimal)>(&ComponentBody::alignTo))

            .addFunction("getRotation", &ComponentBody::getRotation)
            .addFunction("getPosition", &ComponentBody::getPosition)
            .addFunction("getScale", &ComponentBody::getScale)
            .addFunction("getLocalPosition", &ComponentBody::getLocalPosition)
            .addFunction("getPositionRender", &ComponentBody::getPositionRender)
            .addFunction("forward", &ComponentBody::forward)
            .addFunction("right", &ComponentBody::right)
            .addFunction("up", &ComponentBody::up)
            .addFunction("getLinearVelocity", &ComponentBody::getLinearVelocity)
            .addFunction("getAngularVelocity", &ComponentBody::getAngularVelocity)
            .addFunction("modelMatrix", &ComponentBody::modelMatrix)
            .addFunction("modelMatrixRendering", &ComponentBody::modelMatrixRendering)

            .addFunction("setDamping", &ComponentBody::setDamping)
            .addFunction("setDynamic", &ComponentBody::setDynamic)
            .addFunction("setMass", &ComponentBody::setMass)
            .addFunction("setGravity", &ComponentBody::setGravity)

            .addFunction("clearLinearForces", &ComponentBody::clearLinearForces)
            .addFunction("clearAngularForces", &ComponentBody::clearAngularForces)
            .addFunction("clearAllForces", &ComponentBody::clearAllForces)

            .addFunction("setLinearVelocity", static_cast<void(ComponentBody::*)(decimal, decimal, decimal, bool)>(&ComponentBody::setLinearVelocity))
            .addFunction("setAngularVelocity", static_cast<void(ComponentBody::*)(decimal, decimal, decimal, bool)>(&ComponentBody::setAngularVelocity))
            .addFunction("applyForce", static_cast<void(ComponentBody::*)(decimal, decimal, decimal, bool)>(&ComponentBody::applyForce))
            .addFunction("applyImpulse", static_cast<void(ComponentBody::*)(decimal, decimal, decimal, bool)>(&ComponentBody::applyImpulse))
            .addFunction("applyTorque", static_cast<void(ComponentBody::*)(decimal, decimal, decimal, bool)>(&ComponentBody::applyTorque))
            .addFunction("applyTorqueImpulse", static_cast<void(ComponentBody::*)(decimal, decimal, decimal, bool)>(&ComponentBody::applyTorqueImpulse))

        .endClass()
        //component camera
        .beginClass<ComponentCamera>("ComponentCamera")
            .addFunction("forward", &ComponentCamera::forward)
            .addFunction("right", &ComponentCamera::right)
            .addFunction("up", &ComponentCamera::up)

            .addFunction("getAngle", &ComponentCamera::getAngle)
            .addFunction("getAspect", &ComponentCamera::getAspect)
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
            .addFunction("boundingBox", &ComponentModel::boundingBox)
            .addFunction("radius", &ComponentModel::radius)
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
            .addFunction("color", &ModelInstance::color)
            .addFunction("setColor", static_cast<void(ModelInstance::*)(const float, const float, const float, const float)>(&ModelInstance::setColor))
            .addFunction("forceRender", &ModelInstance::forceRender)
            .addFunction("getScale", &ModelInstance::getScale)
            .addFunction("setScale", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::setScale))
            .addFunction("rotate", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::rotate))
            .addFunction("translate", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::translate))
            .addFunction("scale", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::scale))
            .addFunction("position", &ModelInstance::position)
            .addFunction("orientation", &ModelInstance::orientation)
            .addFunction("godRaysColor", &ModelInstance::godRaysColor)
            .addFunction("setGodRaysColor", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::setGodRaysColor))
            .addFunction("parent", &ModelInstance::parent)
            .addFunction("passedRenderCheck", &ModelInstance::passedRenderCheck)
            .addFunction("hide", &ModelInstance::hide)
            .addFunction("show", &ModelInstance::show)
            .addFunction("visible", &ModelInstance::visible)
            .addFunction("index", &ModelInstance::index)
            .addFunction("isForceRendered", &ModelInstance::isForceRendered)
            .addFunction("playAnimation", &ModelInstance::playAnimation)
            .addFunction("setOrientation", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::setOrientation))
        .endClass()
    ;
}
LUABinder::~LUABinder() {
    SAFE_DELETE(m_LUA_STATE);
}

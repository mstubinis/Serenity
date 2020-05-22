#include <core/engine/lua/LuaScript.h>
#include <LuaBridge/LuaBridge.h>

#include <core/engine/lua/LuaBinder.h>
#include <core/engine/lua/LuaState.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/SceneOptions.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/scene/Skybox.h>

#include <ecs/Entity.h>
#include <ecs/Components.h>

#include <iostream>
#include <functional>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;
using namespace Engine::priv;

void print(const string& str) { cout << str << "\n"; }

void addComponentBody(Entity entity, const string& type = "") { 
    string hardCpy = type;
    transform(hardCpy.begin(), hardCpy.end(), hardCpy.begin(), ::tolower);
    if (hardCpy == "convex") {
        entity.addComponent<ComponentBody>(CollisionType::Type::ConvexHull);
    }else if (hardCpy == "box") {
        entity.addComponent<ComponentBody>(CollisionType::Type::Box);
    }else if (hardCpy == "sphere") {
        entity.addComponent<ComponentBody>(CollisionType::Type::Sphere);
    }else if (hardCpy == "triangleshape") {
        entity.addComponent<ComponentBody>(CollisionType::Type::TriangleShape);
    }else if (hardCpy == "triangleshapestatic") {
        entity.addComponent<ComponentBody>(CollisionType::Type::TriangleShapeStatic);
    }else if (hardCpy == "compound") {
        entity.addComponent<ComponentBody>(CollisionType::Type::Compound);
    }else if (hardCpy == "none" || type == "null") {
        entity.addComponent<ComponentBody>(CollisionType::Type::None);
    }else {
        entity.addComponent<ComponentBody>();
    }
}
void addComponentModel(Entity entity, Handle mesh, Handle material, Handle shaderProgram, const unsigned int renderStage) { 
    entity.addComponent<ComponentModel>(mesh, material, shaderProgram, static_cast<RenderStage::Stage>(renderStage));
}
void addComponentName(Entity entity, const string& name) { entity.addComponent<ComponentName>(name); }
void addComponentLogic(Entity entity) { entity.addComponent<ComponentLogic>(); }
void addComponentLogic1(Entity entity) { entity.addComponent<ComponentLogic1>(); }
void addComponentLogic2(Entity entity) { entity.addComponent<ComponentLogic2>(); }
void addComponentLogic3(Entity entity) { entity.addComponent<ComponentLogic3>(); }
void addComponentCamera(Entity entity, const string& type, const float Near, const float Far) {
    string hardCpy = type;
    const auto window = glm::vec2(Engine::Resources::getWindowSize());
    transform(hardCpy.begin(), hardCpy.end(), hardCpy.begin(), ::tolower);
    if (hardCpy == "orthographic" || hardCpy == "ortho") {
        entity.addComponent<ComponentCamera>(0.0f, window.x, 0.0f, window.y, Near, Far);
    }else{
        entity.addComponent<ComponentCamera>(60.0f, window.x / window.y, Near, Far);
    }
}
ComponentBody* getComponentBody(Entity entity) { return entity.getComponent<ComponentBody>(); }
ComponentModel* getComponentModel(Entity entity) { return entity.getComponent<ComponentModel>(); }
ComponentName* getComponentName(Entity entity) { return entity.getComponent<ComponentName>(); }
ComponentLogic* getComponentLogic(Entity entity) { return entity.getComponent<ComponentLogic>(); }
ComponentLogic1* getComponentLogic1(Entity entity) { return entity.getComponent<ComponentLogic1>(); }
ComponentLogic2* getComponentLogic2(Entity entity) { return entity.getComponent<ComponentLogic2>(); }
ComponentLogic3* getComponentLogic3(Entity entity) { return entity.getComponent<ComponentLogic3>(); }
ComponentCamera* getComponentCamera(Entity entity) { return entity.getComponent<ComponentCamera>(); }

LUABinder::LUABinder() {
    m_LUA_STATE = NEW LUAState();
    auto* L = m_LUA_STATE->getState();

    luabridge::getGlobalNamespace(L)
        .addFunction("print", &print)
        .addFunction("getDeltaTime", &Engine::Resources::dt)
        .addFunction("dt", &Engine::Resources::dt)

        .addFunction("addComponentBody", &addComponentBody)
        .addFunction("addComponentModel", &addComponentModel)
        .addFunction("addComponentName", &addComponentName)
        .addFunction("addComponentLogic", &addComponentLogic)
        .addFunction("addComponentLogic1", &addComponentLogic1)
        .addFunction("addComponentLogic2", &addComponentLogic2)
        .addFunction("addComponentLogic3", &addComponentLogic3)
        .addFunction("addComponentCamera", &addComponentCamera)

        .addFunction("getComponentBody", &getComponentBody)
        .addFunction("getComponentModel", &getComponentModel)
        .addFunction("getComponentName", &getComponentName)
        .addFunction("getComponentLogic", &getComponentLogic)
        .addFunction("getComponentLogic1", &getComponentLogic1)
        .addFunction("getComponentLogic2", &getComponentLogic2)
        .addFunction("getComponentLogic3", &getComponentLogic3)
        .addFunction("getComponentCamera", &getComponentCamera)

#pragma region Matrices
        //glm mat4 TODO: add more to this
        .beginClass<glm::mat4>("mat4")

        .endClass()
#pragma endregion

#pragma region Vectors
        //glm vec2 TODO: add more to this
        .beginClass<glm::vec2>("fvec2")
            .addData("x", &glm::vec2::x)
            .addData("y", &glm::vec2::y)
        .endClass()
        //glm vec3 TODO: add more to this
        .beginClass<glm::vec3>("fvec3")
            .addData("x", &glm::vec3::x)
            .addData("y", &glm::vec3::y)
            .addData("z", &glm::vec3::z)
        .endClass()
        //glm vec4 TODO: add more to this
        .beginClass<glm::vec4>("fvec4")
            .addData("x", &glm::vec4::x)
            .addData("y", &glm::vec4::y)
            .addData("z", &glm::vec4::z)
            .addData("w", &glm::vec4::w)
        .endClass()

        //glm vec2 TODO: add more to this
        .beginClass<glm_vec2>("vec2")
            .addData("x", &glm_vec2::x)
            .addData("y", &glm_vec2::y)
        .endClass()
        //glm vec3 TODO: add more to this
        .beginClass<glm_vec3>("vec3")
            .addData("x", &glm_vec3::x)
            .addData("y", &glm_vec3::y)
            .addData("z", &glm_vec3::z)
        .endClass()
        //glm vec4 TODO: add more to this
        .beginClass<glm_vec4>("vec4")
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
            .addProperty("camera", &Viewport::getCamera, &Viewport::setCamera)
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
            .addProperty("name", &Scene::name, &Scene::setName)
            .addFunction("setName", &Scene::setName)
            .addFunction("createEntity", &Scene::createEntity)
            .addFunction("id", &Scene::id)
            .addFunction("setActiveCamera", &Scene::setActiveCamera)
            .addFunction("getActiveCamera", &Scene::getActiveCamera)
            .addFunction("centerSceneToObject", &Scene::centerSceneToObject)
            .addFunction("getBackgroundColor", &Scene::getBackgroundColor)
            .addFunction("setBackgroundColor", static_cast<void(Scene::*)(const float, const float, const float, const float)>(&Scene::setBackgroundColor))
            .addFunction("setGlobalIllumination", static_cast<void(Scene::*)(const float, const float, const float)>(&Scene::setGlobalIllumination))
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
            .addFunction("addChild", static_cast<void(ComponentBody::*)(const Entity)const>(&ComponentBody::addChild))
            .addFunction("removeChild", static_cast<void(ComponentBody::*)(const Entity)const>(&ComponentBody::removeChild))
            //.addFunction("addChildBody", static_cast<void(ComponentBody::*)(const ComponentBody&)const>(&ComponentBody::addChild))
            //.addFunction("removeChildBody", static_cast<void(ComponentBody::*)(const ComponentBody&)const>(&ComponentBody::removeChild))
            .addFunction("hasParent", &ComponentBody::hasParent)
            .addFunction("hasPhysics", &ComponentBody::hasPhysics)

            .addFunction("setPosition", static_cast<void(ComponentBody::*)(const decimal&, const decimal&, const decimal&)>(&ComponentBody::setPosition))
            .addFunction("setScale",    static_cast<void(ComponentBody::*)(const decimal&, const decimal&, const decimal&)>(&ComponentBody::setScale))
            .addFunction("setRotation", static_cast<void(ComponentBody::*)(const decimal&, const decimal&, const decimal&, const decimal&)>(&ComponentBody::setRotation))

            .addFunction("translate", static_cast<void(ComponentBody::*)(const decimal&, const decimal&, const decimal&, const bool)>(&ComponentBody::translate))
            .addFunction("scale", static_cast<void(ComponentBody::*)(const decimal&, const decimal&, const decimal&)>(&ComponentBody::scale))
            .addFunction("rotate", static_cast<void(ComponentBody::*)(const decimal&, const decimal&, const decimal&, const bool)>(&ComponentBody::rotate))
            .addFunction("mass", &ComponentBody::mass)
            .addFunction("getDistance", &ComponentBody::getDistance)
            .addFunction("getScreenCoordinates", &ComponentBody::getScreenCoordinates)
            .addFunction("alignTo", &ComponentBody::alignTo)

            .addFunction("rotation", &ComponentBody::rotation)
            .addFunction("position", &ComponentBody::position)
            .addFunction("getScale", &ComponentBody::getScale)
            .addFunction("localPosition", &ComponentBody::localPosition)
            .addFunction("position_render", &ComponentBody::position_render)
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

            .addFunction("setLinearVelocity", static_cast<void(ComponentBody::*)(const decimal&, const decimal&, const decimal&, const bool)>(&ComponentBody::setLinearVelocity))
            .addFunction("setAngularVelocity", static_cast<void(ComponentBody::*)(const decimal&, const decimal&, const decimal&, const bool)>(&ComponentBody::setAngularVelocity))
            .addFunction("applyForce", static_cast<void(ComponentBody::*)(const decimal&, const decimal&, const decimal&, const bool)>(&ComponentBody::applyForce))
            .addFunction("applyImpulse", static_cast<void(ComponentBody::*)(const decimal&, const decimal&, const decimal&, const bool)>(&ComponentBody::applyImpulse))
            .addFunction("applyTorque", static_cast<void(ComponentBody::*)(const decimal&, const decimal&, const decimal&, const bool)>(&ComponentBody::applyTorque))
            .addFunction("applyTorqueImpulse", static_cast<void(ComponentBody::*)(const decimal&, const decimal&, const decimal&, const bool)>(&ComponentBody::applyTorqueImpulse))

        .endClass()
        //component camera
        .beginClass<ComponentCamera>("ComponentCamera")
/*
            .addFunction("forward", &ComponentCamera::forward)
            .addFunction("right", &ComponentCamera::right)
            .addFunction("up", &ComponentCamera::up)
*/
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
            .addFunction("setFunctor", &ComponentLogic::setFunctor)
        .endClass()
        .beginClass<ComponentLogic1>("ComponentLogic1")
            .addFunction("call", &ComponentLogic1::call)
            .addFunction("setFunctor", &ComponentLogic1::setFunctor)
        .endClass()
        .beginClass<ComponentLogic2>("ComponentLogic2")
            .addFunction("call", &ComponentLogic2::call)
            .addFunction("setFunctor", &ComponentLogic2::setFunctor)
        .endClass()
        .beginClass<ComponentLogic3>("ComponentLogic3")
            .addFunction("call", &ComponentLogic3::call)
            .addFunction("setFunctor", &ComponentLogic3::setFunctor)
        .endClass();
}
LUABinder::~LUABinder() {
    SAFE_DELETE(m_LUA_STATE);
}
LUAState* LUABinder::getState() const {
    return m_LUA_STATE;
}

#include "ComponentGameCamera.h"
#include "core/engine/Engine_Resources.h"
#include "core/engine/Engine_Math.h"
#include "core/engine/Engine_ThreadManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

struct epriv::ComponentGameCameraFunctions final {
    static void RebuildProjectionMatrix(ComponentGameCamera& cam) {
        if (cam._type == ComponentGameCamera::Type::Perspective) {
            cam._projectionMatrix = glm::perspective(cam._angle, cam._aspectRatio, cam._nearPlane, cam._farPlane);
        }else{
            cam._projectionMatrix = glm::ortho(cam._left, cam._right, cam._bottom, cam._top, cam._nearPlane, cam._farPlane);
        }
    }
};

#pragma region Component

ComponentGameCamera::ComponentGameCamera(Entity& _e, float angle, float aspectRatio, float nearPlane, float farPlane) : ComponentBaseClass(_e) {
    _eye = glm::vec3(0.0f); _up = glm::vec3(0.0f, 1.0f, 0.0f);
    _angle = glm::radians(angle); _aspectRatio = aspectRatio; _nearPlane = nearPlane; _farPlane = farPlane;
    _projectionMatrix = glm::perspective(_angle, _aspectRatio, _nearPlane, _farPlane);
    _viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    _type = Type::Perspective;
}
ComponentGameCamera::ComponentGameCamera(Entity& _e, float left, float right, float bottom, float top, float nearPlane, float farPlane) : ComponentBaseClass(_e) {
    _eye = glm::vec3(0.0f); _up = glm::vec3(0.0f, 1.0f, 0.0f);
    _left = left; _right = right; _bottom = bottom; _top = top; _nearPlane = nearPlane; _farPlane = farPlane;
    _projectionMatrix = glm::ortho(_left, _right, _bottom, _top, _nearPlane, _farPlane);
    _viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    _type = Type::Orthographic;
}
ComponentGameCamera::~ComponentGameCamera() {}
void ComponentGameCamera::update(const float& dt) {}
void ComponentGameCamera::resize(uint width, uint height) {
    if (_type == Type::Perspective) {
        _aspectRatio = width / (float)height;
    }
    epriv::ComponentGameCameraFunctions::RebuildProjectionMatrix(*this);
}
uint ComponentGameCamera::pointIntersectTest(glm::vec3& position) {
    for (int i = 0; i < 6; ++i) {
        float d = _planes[i].x * position.x + _planes[i].y * position.y + _planes[i].z * position.z + _planes[i].w;
        if (d > 0.0f) return 0; //outside
    }
    return 1;//inside
}
uint ComponentGameCamera::sphereIntersectTest(glm::vec3& position, float radius) {
    uint res = 1; //inside the viewing frustum
    if (radius <= 0.0f) return 0;
    for (int i = 0; i < 6; ++i) {
        float d = _planes[i].x * position.x + _planes[i].y * position.y + _planes[i].z * position.z + _planes[i].w;
        if (d > radius * 2.0f) return 0; //outside the viewing frustrum
        else if (d > 0.0f) res = 2; //intersecting the viewing plane
    }
    return res;
}
void ComponentGameCamera::lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    _eye = eye;
    _up = up;
    _viewMatrix = glm::lookAt(_eye, center, _up);
    _viewMatrixNoTranslation = glm::lookAt(glm::vec3(0.0f), center - _eye, _up);
}
glm::mat4 ComponentGameCamera::getProjection() { return _projectionMatrix; }
glm::mat4 ComponentGameCamera::getProjectionInverse() { return glm::inverse(_projectionMatrix); }
glm::mat4 ComponentGameCamera::getView() { return _viewMatrix; }
glm::mat4 ComponentGameCamera::getViewInverse() { return glm::inverse(_viewMatrix); }
glm::mat4 ComponentGameCamera::getViewProjection() { return _projectionMatrix * _viewMatrix; }
glm::mat4 ComponentGameCamera::getViewProjectionInverse() { return glm::inverse(_projectionMatrix * _viewMatrix); }
glm::vec3 ComponentGameCamera::getViewVector() { return glm::vec3(_viewMatrix[0][2], _viewMatrix[1][2], _viewMatrix[2][2]); }
float ComponentGameCamera::getAngle() { return _angle; }
float ComponentGameCamera::getAspect() { return _aspectRatio; }
float ComponentGameCamera::getNear() { return _nearPlane; }
float ComponentGameCamera::getFar() { return _farPlane; }
void ComponentGameCamera::setAngle(float a) { _angle = a; epriv::ComponentGameCameraFunctions::RebuildProjectionMatrix(*this); }
void ComponentGameCamera::setAspect(float a) { _aspectRatio = a; epriv::ComponentGameCameraFunctions::RebuildProjectionMatrix(*this); }
void ComponentGameCamera::setNear(float n) { _nearPlane = n; epriv::ComponentGameCameraFunctions::RebuildProjectionMatrix(*this); }
void ComponentGameCamera::setFar(float f) { _farPlane = f; epriv::ComponentGameCameraFunctions::RebuildProjectionMatrix(*this); }

#pragma endregion

#pragma region System

struct epriv::ComponentGameCameraUpdateFunction final {
    static void _defaultUpdate(vector<uint>& _vec, vector<ComponentGameCamera>& _components, const float& dt) {
        for (uint j = 0; j < _vec.size(); ++j) {
            ComponentGameCamera& b = _components[_vec[j]];
            b.update(dt);//custom camera user code
            Math::extractViewFrustumPlanesHartmannGribbs(b._projectionMatrix * b._viewMatrix, b._planes);//update view frustrum 
        }
    }
    void operator()(void* _componentPool, const float& dt) const {
        auto& pool = *(ECSComponentPool<Entity, ComponentGameCamera>*)_componentPool;
        auto& components = pool.pool();

        auto split = epriv::threading::splitVectorIndices(components);
        for (auto& vec : split) {
            epriv::threading::addJobRef(_defaultUpdate, vec, components, dt);
        }
        epriv::threading::waitForAll();
    }
};
struct epriv::ComponentGameCameraComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {

}};
struct epriv::ComponentGameCameraEntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {

}};
struct epriv::ComponentGameCameraSceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};
struct epriv::ComponentGameCameraSceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};

ComponentGameCameraSystem::ComponentGameCameraSystem() {
    setUpdateFunction(epriv::ComponentGameCameraUpdateFunction());
    setOnComponentAddedToEntityFunction(epriv::ComponentGameCameraComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(epriv::ComponentGameCameraEntityAddedToSceneFunction());
    setOnSceneEnteredFunction(epriv::ComponentGameCameraSceneEnteredFunction());
    setOnSceneLeftFunction(epriv::ComponentGameCameraSceneLeftFunction());
}

#pragma endregion
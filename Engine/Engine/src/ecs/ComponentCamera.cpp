#include "ecs/ComponentCamera.h"
#include "core/engine/Engine_Resources.h"
#include "core/engine/Engine_Math.h"
#include "core/engine/Engine_ThreadManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;
using namespace std;


ComponentCamera::ComponentCamera(Entity& _e) : ComponentBaseClass(_e){
    _eye = glm::vec3(0.0f); _up = glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::uvec2& winSize = Resources::getWindowSize();
    _angle = glm::radians(60.0f); _aspectRatio = winSize.x / (float)winSize.y;
    _nearPlane = 0.1f; _farPlane = 5000.0f;
    _projectionMatrix = glm::perspective(_angle, _aspectRatio, _nearPlane, _farPlane);
    _viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    _type = Type::Perspective;
}
ComponentCamera::ComponentCamera(Entity& _e,float angle, float aspectRatio, float nearPlane, float farPlane) : ComponentBaseClass(_e) {
    _eye = glm::vec3(0.0f); _up = glm::vec3(0.0f, 1.0f, 0.0f);
    _angle = glm::radians(angle); _aspectRatio = aspectRatio; _nearPlane = nearPlane; _farPlane = farPlane;
    _projectionMatrix = glm::perspective(_angle, _aspectRatio, _nearPlane, _farPlane);
    _viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    _type = Type::Perspective;
}
ComponentCamera::ComponentCamera(Entity& _e,float left, float right, float bottom, float top, float nearPlane, float farPlane) : ComponentBaseClass(_e) {
    _eye = glm::vec3(0.0f); _up = glm::vec3(0.0f, 1.0f, 0.0f);
    _left = left; _right = right; _bottom = bottom; _top = top; _nearPlane = nearPlane; _farPlane = farPlane;
    _projectionMatrix = glm::ortho(_left, _right, _bottom, _top, _nearPlane, _farPlane);
    _viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    _type = Type::Orthographic;
}
ComponentCamera::~ComponentCamera() {}
void ComponentCamera::update(const float& dt) {}
void ComponentCamera::resize(uint width, uint height) {
    if (_type == Type::Perspective) {
        _aspectRatio = width / (float)height;
    }
    //epriv::ComponentCameraSystem::RebuildProjectionMatrix(*this);
}
uint ComponentCamera::pointIntersectTest(glm::vec3& position) {
    for (int i = 0; i < 6; ++i) {
        float d = _planes[i].x * position.x + _planes[i].y * position.y + _planes[i].z * position.z + _planes[i].w;
        if (d > 0.0f) return 0; //outside
    }
    return 1;//inside
}
uint ComponentCamera::sphereIntersectTest(glm::vec3& position, float radius) {
    uint res = 1; //inside the viewing frustum
    if (radius <= 0.0f) return 0;
    for (int i = 0; i < 6; ++i) {
        float d = _planes[i].x * position.x + _planes[i].y * position.y + _planes[i].z * position.z + _planes[i].w;
        if (d > radius * 2.0f) return 0; //outside the viewing frustrum
        else if (d > 0.0f) res = 2; //intersecting the viewing plane
    }
    return res;
}
void ComponentCamera::lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    _eye = eye;
    _up = up;
    _viewMatrix = glm::lookAt(_eye, center, _up);
    _viewMatrixNoTranslation = glm::lookAt(glm::vec3(0.0f), center - _eye, _up);
}
glm::mat4 ComponentCamera::getProjection() { return _projectionMatrix; }
glm::mat4 ComponentCamera::getProjectionInverse() { return glm::inverse(_projectionMatrix); }
glm::mat4 ComponentCamera::getView() { return _viewMatrix; }
glm::mat4 ComponentCamera::getViewInverse() { return glm::inverse(_viewMatrix); }
glm::mat4 ComponentCamera::getViewProjection() { return _projectionMatrix * _viewMatrix; }
glm::mat4 ComponentCamera::getViewProjectionInverse() { return glm::inverse(_projectionMatrix * _viewMatrix); }
glm::vec3 ComponentCamera::getViewVector() { return glm::vec3(_viewMatrix[0][2], _viewMatrix[1][2], _viewMatrix[2][2]); }
float ComponentCamera::getAngle() { return _angle; }
float ComponentCamera::getAspect() { return _aspectRatio; }
float ComponentCamera::getNear() { return _nearPlane; }
float ComponentCamera::getFar() { return _farPlane; }
void ComponentCamera::setAngle(float a) { _angle = a; /*epriv::ComponentCameraSystem::RebuildProjectionMatrix(*this);*/ }
void ComponentCamera::setAspect(float a) { _aspectRatio = a; /*epriv::ComponentCameraSystem::RebuildProjectionMatrix(*this);*/ }
void ComponentCamera::setNear(float n) { _nearPlane = n; /*epriv::ComponentCameraSystem::RebuildProjectionMatrix(*this);*/ }
void ComponentCamera::setFar(float f) { _farPlane = f; /*epriv::ComponentCameraSystem::RebuildProjectionMatrix(*this);*/ }


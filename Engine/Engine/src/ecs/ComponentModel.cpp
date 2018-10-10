#include "ecs/ComponentModel.h"
#include "ecs/ComponentBody.h"
#include "ecs/ComponentCamera.h"

#include "core/engine/Engine_Resources.h"
#include "core/engine/Engine_Math.h"
#include "core/MeshInstance.h"
#include "core/Camera.h"
#include "core/Mesh.h"

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

struct epriv::ComponentModelFunctions final {
    static float CalculateRadius(ComponentModel& super) {
        float maxLength = 0;
        glm::vec3 boundingBox = glm::vec3(0.0f);
        for (uint i = 0; i < super.models.size(); ++i) {
            auto& pair = *super.models[i];
            glm::mat4& m = pair.model();
            glm::vec3 localPosition = glm::vec3(m[3][0], m[3][1], m[3][2]);
            float length = glm::length(localPosition) + pair.mesh()->getRadius() * Engine::Math::Max(pair.getScale());
            glm::vec3 box = localPosition + pair.mesh()->getRadiusBox() * Engine::Math::Max(pair.getScale());
            if (length > maxLength) { maxLength = length; }
            if (box.x > boundingBox.x || box.y > boundingBox.y || box.z > boundingBox.z) { boundingBox = box; }
        }
        super._radius = maxLength;
        super._radiusBox = boundingBox;
        auto* body = super.owner.getComponent<ComponentBody>();
        if (body) {
            float _bodyScale = Engine::Math::Max(body->getScale());
            super._radius *= _bodyScale;
            super._radiusBox *= _bodyScale;
        }
        return super._radius;
    }
};


#pragma region Component

ComponentModel::ComponentModel(Entity& _e, Handle& mesh, Handle& mat, ShaderP* _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e){
    if (!mesh.null()) addModel(mesh, mat, _prog, _stage);
}
ComponentModel::ComponentModel(Entity& _e, Mesh* mesh, Handle& mat,  ShaderP* _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    if (mesh) addModel(mesh, (Material*)mat.get(), _prog, _stage);
}
ComponentModel::ComponentModel(Entity& _e, Handle& mesh, Material* mat,  ShaderP* _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    if (!mesh.null()) addModel((Mesh*)mesh.get(), mat, _prog, _stage);
}
ComponentModel::ComponentModel(Entity& _e, Mesh* mesh, Material* mat, ShaderP* _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    if (mesh) addModel(mesh, mat, _prog, _stage);
}
ComponentModel::ComponentModel(Entity& _e, Handle& mesh, Handle& mat, Handle& _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    if (!mesh.null()) addModel(mesh, mat, (ShaderP*)_prog.get(), _stage);
}
ComponentModel::ComponentModel(Entity& _e, Mesh* mesh, Handle& mat, Handle& _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    if (mesh) addModel(mesh, (Material*)mat.get(), (ShaderP*)_prog.get(), _stage);
}
ComponentModel::ComponentModel(Entity& _e, Handle& mesh, Material* mat, Handle& _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    if (!mesh.null()) addModel((Mesh*)mesh.get(), mat, (ShaderP*)_prog.get(), _stage);
}
ComponentModel::ComponentModel(Entity& _e, Mesh* mesh, Material* mat, Handle& _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    if (mesh) addModel(mesh, mat, (ShaderP*)_prog.get(), _stage);
}
ComponentModel::~ComponentModel() {
    SAFE_DELETE_VECTOR(models);
}
uint ComponentModel::getNumModels() { return models.size(); }
MeshInstance& ComponentModel::getModel(uint index) { return *models[index]; }
void ComponentModel::show() { for (auto model : models) model->show(); }
void ComponentModel::hide() { for (auto model : models) model->hide(); }
float ComponentModel::radius() { return _radius; }
glm::vec3 ComponentModel::boundingBox() { return _radiusBox; }
uint ComponentModel::addModel(Handle& mesh, Handle& mat, ShaderP* shaderProgram, RenderStage::Stage _stage) { return ComponentModel::addModel((Mesh*)mesh.get(), (Material*)mat.get(), shaderProgram, _stage); }
uint ComponentModel::addModel(Mesh* mesh, Material* material, ShaderP* shaderProgram, RenderStage::Stage _stage) {
    //models.emplace_back(owner, mesh, material, shaderProgram);
    models.push_back(new MeshInstance(owner, mesh, material, shaderProgram));
    auto& instance = *models[models.size() - 1];
    auto& _scene = owner.scene();
    instance.setStage(_stage);
    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, _stage);
    ComponentModelFunctions::CalculateRadius(*this);
    return models.size() - 1;
}
void ComponentModel::setModel(Handle& mesh, Handle& mat, uint index, ShaderP* shaderProgram, RenderStage::Stage _stage) { ComponentModel::setModel((Mesh*)mesh.get(), (Material*)mat.get(), index, shaderProgram, _stage); }
void ComponentModel::setModel(Mesh* mesh, Material* material, uint index, ShaderP* shaderProgram, RenderStage::Stage _stage) {
    auto& instance = *models[index];
    auto& _scene = owner.scene();
    epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(_scene, instance, instance.stage());
    instance.setShaderProgram(shaderProgram);
    instance.setMesh(mesh);
    instance.setMaterial(material);
    instance.setStage(_stage);
    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, _stage);
    ComponentModelFunctions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(ShaderP* shaderProgram, uint index, RenderStage::Stage _stage) {
    auto& instance = *models[index];
    auto& _scene = owner.scene();
    epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(_scene, instance, instance.stage());
    instance.setShaderProgram(shaderProgram);
    instance.setStage(_stage);
    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, _stage);
    ComponentModelFunctions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(Handle& shaderPHandle, uint index, RenderStage::Stage _stage) { ComponentModel::setModelShaderProgram((ShaderP*)shaderPHandle.get(), index, _stage); }
void ComponentModel::setModelMesh(Mesh* mesh, uint index, RenderStage::Stage _stage) {
    auto& instance = *models[index];
    auto& _scene = owner.scene();
    epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(_scene, instance, instance.stage());
    instance.setMesh(mesh);
    instance.setStage(_stage);
    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, _stage);
    ComponentModelFunctions::CalculateRadius(*this);
}
void ComponentModel::setModelMesh(Handle& mesh, uint index, RenderStage::Stage _stage) { ComponentModel::setModelMesh((Mesh*)mesh.get(), index, _stage); }
void ComponentModel::setModelMaterial(Material* material, uint index, RenderStage::Stage _stage) {
    auto& instance = *models[index];
    auto& _scene = owner.scene();
    epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(_scene, instance, instance.stage());
    instance.setMaterial(material);
    instance.setStage(_stage);
    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, _stage);
}
void ComponentModel::setModelMaterial(Handle& mat, uint index, RenderStage::Stage _stage) { ComponentModel::setModelMaterial((Material*)mat.get(), index, _stage); }
bool ComponentModel::rayIntersectSphere(ComponentCamera& camera) {
    auto& body = *owner.getComponent<ComponentBody>();
    return Math::rayIntersectSphere(body.position(), _radius, camera._eye, camera.getViewVector());
}

#pragma endregion

#pragma region System

struct epriv::ComponentModelUpdateFunction final {
    static void _defaultUpdate(vector<uint>& vec, vector<ComponentModel>& _components,Camera* _camera) {
        for (uint j = 0; j < vec.size(); ++j) {
            ComponentModel& m = _components[vec[j]];
            ComponentBody& b = *m.owner.getComponent<ComponentBody>();
            for (uint k = 0; k < m.models.size(); ++k) {
                auto& meshInstance = *m.models[k];
                auto pos = b.position() + meshInstance.position();
                //per mesh instance radius instead?
                uint sphereTest = _camera->sphereIntersectTest(pos, m._radius);                //per mesh instance radius instead?
                if (!meshInstance.visible() || sphereTest == 0 || _camera->getDistance(pos) > m._radius * 1100.0f) {
                    meshInstance.setPassedRenderCheck(false);
                    continue;
                }
                meshInstance.setPassedRenderCheck(true);
            }
        }
    }
    void operator()(void* _componentPool, const float& dt) const {
        auto* camera = Resources::getCurrentScene()->getActiveCamera();
        auto& pool = *(ECSComponentPool<Entity, ComponentModel>*)_componentPool;
        auto& components = pool.pool();
        auto split = epriv::threading::splitVectorIndices(components);
        //TODO: might have to pass camera as a reference to a pointer (*&)
        for (auto& vec : split) {
            epriv::threading::addJobRef(_defaultUpdate, vec, components, camera);
        }
        epriv::threading::waitForAll();
    }
};
struct epriv::ComponentModelComponentAddedToEntityFunction final {void operator()(void* _component) const {
    ComponentModel& componentModel = *(ComponentModel*)_component;
    ComponentModelFunctions::CalculateRadius(componentModel);
}};
struct epriv::ComponentModelEntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity) const {
    auto& scene = _entity.scene();
    auto& pool = *(ECSComponentPool<Entity, ComponentModel>*)_componentPool;
    auto& component = *pool.getComponent(_entity);
    for (uint i = 0; i < component.models.size(); ++i) {
        auto& meshInstance = *component.models[i];
        InternalScenePublicInterface::AddMeshInstanceToPipeline(scene, meshInstance, meshInstance.stage());
    }
}};
struct epriv::ComponentModelSceneEnteredFunction final {void operator()(void* _componentPool, Scene& _Scene) const {

}};
struct epriv::ComponentModelSceneLeftFunction final {void operator()(void* _componentPool, Scene& _Scene) const {

}};


ComponentModelSystem::ComponentModelSystem() {
    setUpdateFunction(epriv::ComponentModelUpdateFunction());
    setOnComponentAddedToEntityFunction(epriv::ComponentModelComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(epriv::ComponentModelEntityAddedToSceneFunction());
    setOnSceneEnteredFunction(epriv::ComponentModelSceneEnteredFunction());
    setOnSceneLeftFunction(epriv::ComponentModelSceneLeftFunction());
}

#pragma endregion
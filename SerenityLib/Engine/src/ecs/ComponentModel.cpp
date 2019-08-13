#include <ecs/ComponentModel.h>
#include <ecs/ComponentBody.h>
#include <ecs/ComponentCamera.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/math/Engine_Math.h>
#include <core/ModelInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/mesh/Mesh.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

float ComponentModel_Functions::CalculateRadius(ComponentModel& super) {
    float maxLength = 0;
    glm::vec3 boundingBox = glm::vec3(0.0f);
    for (uint i = 0; i < super._modelInstances.size(); ++i) {
        auto& modelInstance = *super._modelInstances[i];
        const glm::mat4& m = modelInstance.modelMatrix();
        const glm::vec3& localPosition = glm::vec3(m[3][0], m[3][1], m[3][2]);
        const auto& modelInstanceScale = Math::Max(modelInstance.getScale());
        const float length = glm::length(localPosition) + modelInstance.mesh()->getRadius() * modelInstanceScale;
        const glm::vec3 box = localPosition + modelInstance.mesh()->getRadiusBox() * modelInstanceScale;
        if (length > maxLength) { 
            maxLength = length; 
        }
        if (box.x > boundingBox.x || box.y > boundingBox.y || box.z > boundingBox.z) { 
            boundingBox = box; 
        }
    }
    super._radius = maxLength;
    super._radiusBox = boundingBox;
    auto* body = super.owner.getComponent<ComponentBody>();
    if (body) {
        const float& bodyScale = Math::Max(body->getScale());
        super._radius    *= bodyScale;
        super._radiusBox *= bodyScale;
    }
    return super._radius; //now modified by the body scale
};


#pragma region Component

ComponentModel::ComponentModel(const Entity& entity, Handle& mesh, Handle& material, ShaderProgram* shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity){
    addModel(mesh, material, shaderProgram, stage);
}
ComponentModel::ComponentModel(const Entity& entity, Mesh* mesh, Handle& material,  ShaderProgram* shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, (Material*)material.get(), shaderProgram, stage);
}
ComponentModel::ComponentModel(const Entity& entity, Handle& mesh, Material* material,  ShaderProgram* shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel((Mesh*)mesh.get(), material, shaderProgram, stage);
}
ComponentModel::ComponentModel(const Entity& entity, Mesh* mesh, Material* material, ShaderProgram* shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, material, shaderProgram, stage);
}
ComponentModel::ComponentModel(const Entity& entity, Handle& mesh, Handle& material, Handle& shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, material, (ShaderProgram*)shaderProgram.get(), stage);
}
ComponentModel::ComponentModel(const Entity& entity, Mesh* mesh, Handle& material, Handle& shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, (Material*)material.get(), (ShaderProgram*)shaderProgram.get(), stage);
}
ComponentModel::ComponentModel(const Entity& entity, Handle& mesh, Material* material, Handle& shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel((Mesh*)mesh.get(), material, (ShaderProgram*)shaderProgram.get(), stage);
}
ComponentModel::ComponentModel(const Entity& entity, Mesh* mesh, Material* material, Handle& shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, material, (ShaderProgram*)shaderProgram.get(), stage);
}
ComponentModel::~ComponentModel() {
    SAFE_DELETE_VECTOR(_modelInstances);
}
const uint& ComponentModel::getNumModels() const {
    return _modelInstances.size();
}
ModelInstance& ComponentModel::getModel(const uint& index) {
    return *_modelInstances[index];
}
void ComponentModel::show() { 
    for (auto& modelInstance : _modelInstances)
        modelInstance->show();
}
void ComponentModel::hide() { 
    for (auto& modelInstance : _modelInstances)
        modelInstance->hide();
}
const float& ComponentModel::radius() const {
    return _radius; 
}
const glm::vec3& ComponentModel::boundingBox() const {
    return _radiusBox;
}
ModelInstance& ComponentModel::addModel(Handle& mesh, Handle& material, ShaderProgram* shaderProgram, const RenderStage::Stage& stage) {
    return ComponentModel::addModel((Mesh*)mesh.get(), (Material*)material.get(), shaderProgram, stage);
}
ModelInstance& ComponentModel::addModel(Mesh* mesh, Material* material, ShaderProgram* shaderProgram, const RenderStage::Stage& stage) {
    auto modelInstance = new ModelInstance(owner, mesh, material, shaderProgram);
    _modelInstances.push_back(modelInstance);
    auto& instance = *modelInstance;
    auto& _scene = owner.scene();
    instance.m_Stage = stage;
    InternalScenePublicInterface::AddModelInstanceToPipeline(_scene, instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
    return instance;
}
void ComponentModel::setModel(Handle& mesh, Handle& material, const uint& index, ShaderProgram* shaderProgram, const RenderStage::Stage& stage) {
    ComponentModel::setModel((Mesh*)mesh.get(), (Material*)material.get(), index, shaderProgram, stage);
}
void ComponentModel::setModel(Mesh* mesh, Material* material, const uint& index, ShaderProgram* shaderProgram, const RenderStage::Stage& stage) {
    auto& instance = *_modelInstances[index];
    auto& _scene = owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(_scene, instance, instance.stage());

    instance.m_ShaderProgram = shaderProgram;
    instance.m_Mesh          = mesh;
    instance.m_Material      = material;
    instance.m_Stage         = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(_scene, instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(ShaderProgram* shaderProgram, const uint& index, const RenderStage::Stage& stage) {
    auto& instance = *_modelInstances[index];
    auto& scene   = owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, instance, instance.stage());

    instance.m_ShaderProgram = shaderProgram;
    instance.m_Stage         = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(Handle& shaderPHandle, const uint& index, const RenderStage::Stage& stage) {
    ComponentModel::setModelShaderProgram((ShaderProgram*)shaderPHandle.get(), index, stage); 
}
void ComponentModel::setStage(const RenderStage::Stage& stage, const uint& index) {
    auto& instance = *_modelInstances[index];
    auto& scene = owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, instance, instance.stage());

    instance.m_Stage = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, instance, stage);
}
void ComponentModel::setModelMesh(Mesh* mesh, const uint& index, const RenderStage::Stage& stage) {
    auto& instance = *_modelInstances[index];
    auto& scene   = owner.scene();

    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, instance, instance.stage());

    instance.m_Mesh  = mesh;
    instance.m_Stage = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelMesh(Handle& mesh, const uint& index, const RenderStage::Stage& stage) {
    ComponentModel::setModelMesh((Mesh*)mesh.get(), index, stage); 
}
void ComponentModel::setModelMaterial(Material* material, const uint& index, const RenderStage::Stage& stage) {
    auto& instance = *_modelInstances[index];
    auto& scene   = owner.scene();
    InternalScenePublicInterface::RemoveModelInstanceFromPipeline(scene, instance, instance.stage());

    instance.m_Material = material;
    instance.m_Stage    = stage;

    InternalScenePublicInterface::AddModelInstanceToPipeline(scene, instance, stage);
}
void ComponentModel::setModelMaterial(Handle& material, const uint& index, const RenderStage::Stage& stage) {
    ComponentModel::setModelMaterial((Material*)(material.get()), index, stage);
}
const bool ComponentModel::rayIntersectSphere(const ComponentCamera& camera) {
    auto& body = *owner.getComponent<ComponentBody>();
    return Math::rayIntersectSphere(body.position(), _radius, camera.m_Eye, camera.getViewVector());
}

#pragma endregion

#pragma region System

struct epriv::ComponentModel_UpdateFunction final {
    static void _defaultUpdate(vector<uint>& vec, vector<ComponentModel>& components, Camera* camera) {

    }
    void operator()(void* componentPool, const double& dt, Scene& scene) const {

    }
};
struct epriv::ComponentModel_ComponentAddedToEntityFunction final {void operator()(void* component, Entity& entity) const {

}};
struct epriv::ComponentModel_EntityAddedToSceneFunction final {void operator()(void* componentPool, Entity& entity, Scene& scene) const { 

}};
struct epriv::ComponentModel_SceneEnteredFunction final {void operator()(void* componentPool, Scene& scene) const {

}};
struct epriv::ComponentModel_SceneLeftFunction final {void operator()(void* componentPool, Scene& scene) const {

}};


ComponentModel_System::ComponentModel_System() {
    setUpdateFunction(ComponentModel_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentModel_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentModel_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentModel_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentModel_SceneLeftFunction());
}

#pragma endregion
#include <ecs/ComponentModel.h>
#include <ecs/ComponentBody.h>
#include <ecs/ComponentCamera.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/math/Engine_Math.h>
#include <core/MeshInstance.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/mesh/Mesh.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

float ComponentModel_Functions::CalculateRadius(ComponentModel& super) {
    float maxLength = 0;
    glm::vec3 boundingBox = glm::vec3(0.0f);
    for (uint i = 0; i < super._meshInstances.size(); ++i) {
        auto& meshInstance = *super._meshInstances[i];
        const glm::mat4& m = meshInstance.modelMatrix();
        const glm::vec3& localPosition = glm::vec3(m[3][0], m[3][1], m[3][2]);
        const auto& meshInstanceScale = meshInstance.getScale();
        const float length = glm::length(localPosition) + meshInstance.mesh()->getRadius() * Engine::Math::Max(meshInstanceScale);
        const glm::vec3 box = localPosition + meshInstance.mesh()->getRadiusBox() * Engine::Math::Max(meshInstanceScale);
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
        const float& bodyScale = Engine::Math::Max(body->getScale());
        super._radius    *= bodyScale;
        super._radiusBox *= bodyScale;
    }
    return super._radius; //now modified by the body scale
};


#pragma region Component

ComponentModel::ComponentModel(const Entity& entity, Handle& mesh, Handle& material, ShaderP* shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity){
    addModel(mesh, material, shaderProgram, stage);
}
ComponentModel::ComponentModel(const Entity& entity, Mesh* mesh, Handle& material,  ShaderP* shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, (Material*)material.get(), shaderProgram, stage);
}
ComponentModel::ComponentModel(const Entity& entity, Handle& mesh, Material* material,  ShaderP* shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel((Mesh*)mesh.get(), material, shaderProgram, stage);
}
ComponentModel::ComponentModel(const Entity& entity, Mesh* mesh, Material* material, ShaderP* shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, material, shaderProgram, stage);
}
ComponentModel::ComponentModel(const Entity& entity, Handle& mesh, Handle& material, Handle& shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, material, (ShaderP*)shaderProgram.get(), stage);
}
ComponentModel::ComponentModel(const Entity& entity, Mesh* mesh, Handle& material, Handle& shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, (Material*)material.get(), (ShaderP*)shaderProgram.get(), stage);
}
ComponentModel::ComponentModel(const Entity& entity, Handle& mesh, Material* material, Handle& shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel((Mesh*)mesh.get(), material, (ShaderP*)shaderProgram.get(), stage);
}
ComponentModel::ComponentModel(const Entity& entity, Mesh* mesh, Material* material, Handle& shaderProgram, const RenderStage::Stage& stage) : ComponentBaseClass(entity) {
    addModel(mesh, material, (ShaderP*)shaderProgram.get(), stage);
}
ComponentModel::~ComponentModel() {
    SAFE_DELETE_VECTOR(_meshInstances);
}
const uint& ComponentModel::getNumModels() const {
    return _meshInstances.size(); 
}
MeshInstance& ComponentModel::getModel(const uint& index) {
    return *_meshInstances[index]; 
}
void ComponentModel::show() { 
    for (auto& meshInstance : _meshInstances)
        meshInstance->show();
}
void ComponentModel::hide() { 
    for (auto& meshInstance : _meshInstances)
        meshInstance->hide();
}
const float& ComponentModel::radius() const {
    return _radius; 
}
const glm::vec3& ComponentModel::boundingBox() const {
    return _radiusBox; 
}
const uint ComponentModel::addModel(Handle& mesh, Handle& material, ShaderP* shaderProgram, const RenderStage::Stage& stage) {
    return ComponentModel::addModel((Mesh*)mesh.get(), (Material*)material.get(), shaderProgram, stage);
}
const uint ComponentModel::addModel(Mesh* mesh, Material* material, ShaderP* shaderProgram, const RenderStage::Stage& stage) {
    _meshInstances.push_back(new MeshInstance(owner, mesh, material, shaderProgram));
    auto& instance = *_meshInstances[_meshInstances.size() - 1];
    auto& _scene = owner.scene();
    instance.m_Stage = stage;
    InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
    return _meshInstances.size() - 1;
}
void ComponentModel::setModel(Handle& mesh, Handle& material, const uint& index, ShaderP* shaderProgram, const RenderStage::Stage& stage) {
    ComponentModel::setModel((Mesh*)mesh.get(), (Material*)material.get(), index, shaderProgram, stage);
}
void ComponentModel::setModel(Mesh* mesh, Material* material, const uint& index, ShaderP* shaderProgram, const RenderStage::Stage& stage) {
    auto& instance = *_meshInstances[index];
    auto& _scene = owner.scene();
    InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(_scene, instance, instance.stage());

    instance.m_ShaderProgram = shaderProgram;
    instance.m_Mesh          = mesh;
    instance.m_Material      = material;
    instance.m_Stage         = stage;

    InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(ShaderP* shaderProgram, const uint& index, const RenderStage::Stage& stage) {
    auto& instance = *_meshInstances[index];
    auto& scene   = owner.scene();
    InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(scene, instance, instance.stage());

    instance.m_ShaderProgram = shaderProgram;
    instance.m_Stage         = stage;

    InternalScenePublicInterface::AddMeshInstanceToPipeline(scene, instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(Handle& shaderPHandle, const uint& index, const RenderStage::Stage& stage) {
    ComponentModel::setModelShaderProgram((ShaderP*)shaderPHandle.get(), index, stage); 
}
void ComponentModel::setModelMesh(Mesh* mesh, const uint& index, const RenderStage::Stage& stage) {
    auto& instance = *_meshInstances[index];
    auto& scene   = owner.scene();

    InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(scene, instance, instance.stage());

    instance.m_Mesh  = mesh;
    instance.m_Stage = stage;

    InternalScenePublicInterface::AddMeshInstanceToPipeline(scene, instance, stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelMesh(Handle& mesh, const uint& index, const RenderStage::Stage& stage) {
    ComponentModel::setModelMesh((Mesh*)mesh.get(), index, stage); 
}
void ComponentModel::setModelMaterial(Material* material, const uint& index, const RenderStage::Stage& stage) {
    auto& instance = *_meshInstances[index];
    auto& scene   = owner.scene();
    InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(scene, instance, instance.stage());

    instance.m_Material = material;
    instance.m_Stage    = stage;

    InternalScenePublicInterface::AddMeshInstanceToPipeline(scene, instance, stage);
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
    auto& pool = *static_cast<ECSComponentPool<Entity, ComponentModel>*>(componentPool);
    auto* component = pool.getComponent(entity);
    if (component) {
        auto& _component = *component;
        for (uint i = 0; i < _component._meshInstances.size(); ++i) {
            auto& meshInstance = *_component._meshInstances[i];
            InternalScenePublicInterface::AddMeshInstanceToPipeline(scene, meshInstance, meshInstance.stage());
        }
    }
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
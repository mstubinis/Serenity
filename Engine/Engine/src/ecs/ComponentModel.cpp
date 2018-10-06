#include "ecs/ComponentModel.h"
#include "ecs/ComponentBody.h"
#include "ecs/ComponentCamera.h"

#include "core/engine/Engine_Resources.h"
#include "core/engine/Engine_Math.h"
#include "core/MeshInstance.h"


using namespace Engine;
using namespace std;

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
MeshInstance* ComponentModel::getModel(uint index) { return models[index]; }
void ComponentModel::show() { for (auto model : models) model->show(); }
void ComponentModel::hide() { for (auto model : models) model->hide(); }
float ComponentModel::radius() { return _radius; }
glm::vec3 ComponentModel::boundingBox() { return _radiusBox; }
uint ComponentModel::addModel(Handle& mesh, Handle& mat, ShaderP* shaderProgram, RenderStage::Stage _stage) { return ComponentModel::addModel((Mesh*)mesh.get(), (Material*)mat.get(), shaderProgram, _stage); }
uint ComponentModel::addModel(Mesh* mesh, Material* material, ShaderP* shaderProgram, RenderStage::Stage _stage) {
    MeshInstance* instance = new MeshInstance(owner, mesh, material, shaderProgram);
    models.push_back(instance);
    auto& _scene = owner.scene();
    instance->setStage(_stage);
    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, *instance, _stage);
    //epriv::ComponentInternalFunctionality::CalculateRadius(*this);
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
    //epriv::ComponentInternalFunctionality::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(ShaderP* shaderProgram, uint index, RenderStage::Stage _stage) {
    auto& instance = *models[index];
    auto& _scene = owner.scene();
    epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(_scene, instance, instance.stage());
    instance.setShaderProgram(shaderProgram);
    instance.setStage(_stage);
    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, _stage);
    //epriv::ComponentInternalFunctionality::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(Handle& shaderPHandle, uint index, RenderStage::Stage _stage) { ComponentModel::setModelShaderProgram((ShaderP*)shaderPHandle.get(), index, _stage); }
void ComponentModel::setModelMesh(Mesh* mesh, uint index, RenderStage::Stage _stage) {
    auto& instance = *models[index];
    auto& _scene = owner.scene();
    epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(_scene, instance, instance.stage());
    instance.setMesh(mesh);
    instance.setStage(_stage);
    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, _stage);
    //epriv::ComponentInternalFunctionality::CalculateRadius(*this);
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
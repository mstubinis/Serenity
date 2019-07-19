#include <ecs/ComponentModel.h>
#include <ecs/ComponentBody.h>
#include <ecs/ComponentCamera.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/math/Engine_Math.h>
#include <core/MeshInstance.h>
#include <core/Camera.h>
#include <core/engine/mesh/Mesh.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

float epriv::ComponentModel_Functions::CalculateRadius(ComponentModel& super) {
    float maxLength = 0;
    glm::vec3 boundingBox = glm::vec3(0.0f);
    for (uint i = 0; i < super._meshInstances.size(); ++i) {
        auto& meshInstance = *super._meshInstances[i];
        glm::mat4& m = meshInstance.modelMatrix();
        glm::vec3 localPosition = glm::vec3(m[3][0], m[3][1], m[3][2]);
        auto& meshInstanceScale = meshInstance.getScale();
        float length = glm::length(localPosition) + meshInstance.mesh()->getRadius() * Engine::Math::Max(meshInstanceScale);
        glm::vec3 box = localPosition + meshInstance.mesh()->getRadiusBox() * Engine::Math::Max(meshInstanceScale);
        if (length > maxLength) { 
            maxLength = length; 
        }
        if (box.x > boundingBox.x || box.y > boundingBox.y || box.z > boundingBox.z) { 
            boundingBox = box; 
        }
    }
    super._radius = maxLength;
    super._radiusBox = boundingBox;
    ComponentBody* body = super.owner.getComponent<ComponentBody>();
    if (body) {
        float _bodyScale = Engine::Math::Max(body->getScale());
        super._radius    *= _bodyScale;
        super._radiusBox *= _bodyScale;
    }
    return super._radius; //now modified by the body scale
};


#pragma region Component

ComponentModel::ComponentModel(const Entity& _e, Handle& mesh, Handle& mat, ShaderP* _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e){
    addModel(mesh, mat, _prog, _stage);
}
ComponentModel::ComponentModel(const Entity& _e, Mesh* mesh, Handle& mat,  ShaderP* _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    addModel(mesh, (Material*)mat.get(), _prog, _stage);
}
ComponentModel::ComponentModel(const Entity& _e, Handle& mesh, Material* mat,  ShaderP* _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    addModel((Mesh*)mesh.get(), mat, _prog, _stage);
}
ComponentModel::ComponentModel(const Entity& _e, Mesh* mesh, Material* mat, ShaderP* _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    addModel(mesh, mat, _prog, _stage);
}
ComponentModel::ComponentModel(const Entity& _e, Handle& mesh, Handle& mat, Handle& _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    addModel(mesh, mat, (ShaderP*)_prog.get(), _stage);
}
ComponentModel::ComponentModel(const Entity& _e, Mesh* mesh, Handle& mat, Handle& _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    addModel(mesh, (Material*)mat.get(), (ShaderP*)_prog.get(), _stage);
}
ComponentModel::ComponentModel(const Entity& _e, Handle& mesh, Material* mat, Handle& _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    addModel((Mesh*)mesh.get(), mat, (ShaderP*)_prog.get(), _stage);
}
ComponentModel::ComponentModel(const Entity& _e, Mesh* mesh, Material* mat, Handle& _prog, RenderStage::Stage _stage) : ComponentBaseClass(_e) {
    addModel(mesh, mat, (ShaderP*)_prog.get(), _stage);
}
ComponentModel::~ComponentModel() {
    SAFE_DELETE_VECTOR(_meshInstances);
}
uint ComponentModel::getNumModels() { 
    return _meshInstances.size(); 
}
MeshInstance& ComponentModel::getModel(uint index) { 
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
float ComponentModel::radius() { 
    return _radius; 
}
glm::vec3 ComponentModel::boundingBox() { 
    return _radiusBox; 
}
uint ComponentModel::addModel(Handle& mesh, Handle& mat, ShaderP* shaderProgram, RenderStage::Stage _stage) { 
    return ComponentModel::addModel((Mesh*)mesh.get(), (Material*)mat.get(), shaderProgram, _stage); 
}
uint ComponentModel::addModel(Mesh* mesh, Material* material, ShaderP* shaderProgram, RenderStage::Stage _stage) {
    _meshInstances.push_back(new MeshInstance(owner, mesh, material, shaderProgram));
    auto& instance = *_meshInstances[_meshInstances.size() - 1];
    auto& _scene = owner.scene();
    instance.m_Stage = _stage;
    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, _stage);
    ComponentModel_Functions::CalculateRadius(*this);
    return _meshInstances.size() - 1;
}
void ComponentModel::setModel(Handle& mesh, Handle& mat, uint index, ShaderP* shaderProgram, RenderStage::Stage _stage) { 
    ComponentModel::setModel((Mesh*)mesh.get(), (Material*)mat.get(), index, shaderProgram, _stage); 
}
void ComponentModel::setModel(Mesh* mesh, Material* material, uint index, ShaderP* shaderProgram, RenderStage::Stage _stage) {
    auto& instance = *_meshInstances[index];
    auto& _scene = owner.scene();
    epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(_scene, instance, instance.stage());

    instance.m_ShaderProgram = shaderProgram;
    instance.m_Mesh          = mesh;
    instance.m_Material      = material;
    instance.m_Stage         = _stage;

    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, _stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(ShaderP* shaderProgram, uint index, RenderStage::Stage _stage) {
    auto& instance = *_meshInstances[index];
    auto& _scene   = owner.scene();
    epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(_scene, instance, instance.stage());

    instance.m_ShaderProgram = shaderProgram;
    instance.m_Stage         = _stage;

    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, _stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelShaderProgram(Handle& shaderPHandle, uint index, RenderStage::Stage _stage) { 
    ComponentModel::setModelShaderProgram((ShaderP*)shaderPHandle.get(), index, _stage); 
}
void ComponentModel::setModelMesh(Mesh* mesh, uint index, RenderStage::Stage _stage) {
    auto& instance = *_meshInstances[index];
    auto& _scene   = owner.scene();

    epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(_scene, instance, instance.stage());

    instance.m_Mesh  = mesh;
    instance.m_Stage = _stage;

    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, _stage);
    ComponentModel_Functions::CalculateRadius(*this);
}
void ComponentModel::setModelMesh(Handle& mesh, uint index, RenderStage::Stage _stage) { 
    ComponentModel::setModelMesh((Mesh*)mesh.get(), index, _stage); 
}
void ComponentModel::setModelMaterial(Material* material, uint index, RenderStage::Stage _stage) {
    auto& instance = *_meshInstances[index];
    auto& _scene   = owner.scene();
    epriv::InternalScenePublicInterface::RemoveMeshInstanceFromPipeline(_scene, instance, instance.stage());

    instance.m_Material = material;
    instance.m_Stage    = _stage;

    epriv::InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, instance, _stage);
}
void ComponentModel::setModelMaterial(Handle& mat, uint index, RenderStage::Stage _stage) { 
    ComponentModel::setModelMaterial((Material*)mat.get(), index, _stage); 
}
bool ComponentModel::rayIntersectSphere(ComponentCamera& camera) {
    auto& body = *owner.getComponent<ComponentBody>();
    return Math::rayIntersectSphere(body.position(), _radius, camera.m_Eye, camera.getViewVector());
}

#pragma endregion

#pragma region System

struct epriv::ComponentModel_UpdateFunction final {
    static void _defaultUpdate(vector<uint>& vec, vector<ComponentModel>& _components,Camera* _camera) {
        for (uint j = 0; j < vec.size(); ++j) {
            ComponentModel& m = _components[vec[j]];
            ComponentBody* b = m.owner.getComponent<ComponentBody>();
            if (b) {
                ComponentBody& _b = *b;
                for (uint k = 0; k < m._meshInstances.size(); ++k) {
                    auto& meshInstance = *m._meshInstances[k];
                    auto pos = _b.position() + meshInstance.position();
                    //per mesh instance radius instead?
                    uint sphereTest = _camera->sphereIntersectTest(pos, m._radius);                //per mesh instance radius instead?
                    auto comparison = m._radius * 1100.0f;
                    //if (!meshInstance.visible() || sphereTest == 0 || _camera->getDistance(pos) > comparison) {
                    if (!meshInstance.visible() || sphereTest == 0 || _camera->getDistanceSquared(pos) > comparison*comparison) { //optimization: using squared distance to remove the sqrt()
                        meshInstance.setPassedRenderCheck(false);
                        continue;
                    }
                    meshInstance.setPassedRenderCheck(true);
                }
            }else{
                for (uint k = 0; k < m._meshInstances.size(); ++k) {
                    auto& meshInstance = *m._meshInstances[k];
                    meshInstance.setPassedRenderCheck(false);
                }
            }
        }
    }
    void operator()(void* _componentPool, const double& dt, Scene& _scene) const {
        auto* camera = _scene.getActiveCamera();
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
struct epriv::ComponentModel_ComponentAddedToEntityFunction final {void operator()(void* _component, Entity& _entity) const {
}};
struct epriv::ComponentModel_EntityAddedToSceneFunction final {void operator()(void* _componentPool, Entity& _entity, Scene& _scene) const {   
    auto& pool = *(ECSComponentPool<Entity, ComponentModel>*)_componentPool;
    auto* component = pool.getComponent(_entity);
    if (component) {
        auto& _component = *component;
        for (uint i = 0; i < _component._meshInstances.size(); ++i) {
            auto& meshInstance = *_component._meshInstances[i];
            InternalScenePublicInterface::AddMeshInstanceToPipeline(_scene, meshInstance, meshInstance.stage());
        }
    }
}};
struct epriv::ComponentModel_SceneEnteredFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};
struct epriv::ComponentModel_SceneLeftFunction final {void operator()(void* _componentPool, Scene& _scene) const {

}};


ComponentModel_System::ComponentModel_System() {
    setUpdateFunction(ComponentModel_UpdateFunction());
    setOnComponentAddedToEntityFunction(ComponentModel_ComponentAddedToEntityFunction());
    setOnEntityAddedToSceneFunction(ComponentModel_EntityAddedToSceneFunction());
    setOnSceneEnteredFunction(ComponentModel_SceneEnteredFunction());
    setOnSceneLeftFunction(ComponentModel_SceneLeftFunction());
}

#pragma endregion
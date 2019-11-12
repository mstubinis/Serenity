#include <core/engine/physics/Collision.h>
#include <core/engine/math/Engine_Math.h>

#include <ecs/ComponentModel.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/model/ModelInstance.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <LinearMath/btIDebugDraw.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

void Collision::_init(ComponentBody* body, vector<ModelInstance*>& modelInstances, const float& _mass, const CollisionType::Type _type) {
    btCompoundShape* btCompound = new btCompoundShape();
    btTransform localTransform;  
    for (size_t i = 0; i < modelInstances.size(); ++i) {
        auto& instance = *modelInstances[i];
        auto* mesh = instance.mesh();
        btCollisionShape* btShape = InternalMeshPublicInterface::BuildCollision(&instance, _type, true);
        localTransform = btTransform(Math::glmToBTQuat(instance.orientation()), Math::btVectorFromGLM(instance.position()));
        btShape->setMargin(0.001f);
        btShape->calculateLocalInertia(_mass, m_BtInertia); //this is important
        btCompound->addChildShape(localTransform, btShape);
    }
    btCompound->setMargin(0.001f);
    btCompound->recalculateLocalAabb();
    btCompound->setUserPointer(body);
    m_BtShape = btCompound;
    setMass(_mass);
}
void Collision::_baseInit(const CollisionType::Type _type, const float& _mass) {
    m_BtInertia = btVector3(0.0f, 0.0f, 0.0f);
    m_Type = _type;
    setMass(_mass);
}
Collision::Collision() {
    //construtor
    m_BtInertia = btVector3(0.0f, 0.0f, 0.0f);
    m_Type = CollisionType::None;
    m_BtShape = nullptr;
    setMass(0.0f);
}
Collision::Collision(btHeightfieldTerrainShape& heightField, const CollisionType::Type _type, const float& _mass) {
    _baseInit(_type, _mass);
    m_BtShape = &heightField;
}

Collision::Collision(const CollisionType::Type _type, ModelInstance* modelInstance, const float& _mass) {
    //construtor
    btCollisionShape* btShape = InternalMeshPublicInterface::BuildCollision(modelInstance, _type);
    m_BtShape = btShape;
    _baseInit(_type, _mass);
}
Collision::Collision(const CollisionType::Type _type, Mesh& mesh, const float& _mass) {
    //construtor
    btCollisionShape* btShape = InternalMeshPublicInterface::BuildCollision(&mesh, _type);
    m_BtShape = btShape;
    _baseInit(_type, _mass);
}

Collision::Collision(ComponentBody* body, ComponentModel& _modelComponent, const float& _mass, const CollisionType::Type _type) {
    //construtor
    vector<ModelInstance*> modelInstances;
    for (size_t i = 0; i < _modelComponent.getNumModels(); ++i) {
        modelInstances.push_back(&_modelComponent.getModel(i));
    }
    _init(body, modelInstances, _mass, _type);
}
Collision::~Collision() {
    //destructor
    /*
    if (m_BtShape) {
        btCompoundShape* compoundCast = dynamic_cast<btCompoundShape*>(m_Shape);
        if (compoundCast) {
            int numChildShapes = compoundCast->getNumChildShapes();
            for (int i = 0; i < numChildShapes; ++i) {
                btCollisionShape* shape = compoundCast->getChildShape(i);
                SAFE_DELETE(shape);
            }
        }
    }
    */
    SAFE_DELETE(m_BtShape);
}
Collision::Collision(const Collision& other) {
    //copy constructor
    m_BtInertia = other.m_BtInertia;
    m_Type      = other.m_Type;
    if (other.m_BtShape) 
        m_BtShape = other.m_BtShape;
    else                 
        m_BtShape = nullptr;
}
Collision& Collision::operator=(const Collision& other) {
    //copy assignment
    m_BtInertia = other.m_BtInertia;
    m_Type      = other.m_Type;
    if (other.m_BtShape) 
        m_BtShape = other.m_BtShape;
    else                 
        m_BtShape = nullptr;
    return *this;
}
Collision::Collision(Collision&& other) noexcept {
    //move constructor
    using std::swap;
    swap(m_BtInertia, other.m_BtInertia);
    swap(m_Type, other.m_Type);
    swap(m_BtShape, other.m_BtShape);
    other.m_BtShape = nullptr;
}
Collision& Collision::operator=(Collision&& other) noexcept {
    //move assignment
    using std::swap;
    swap(m_BtInertia, other.m_BtInertia);
    swap(m_Type, other.m_Type);
    swap(m_BtShape, other.m_BtShape);
    other.m_BtShape = nullptr;
    return *this;
}


void Collision::setMass(const float _mass) {
    if (!m_BtShape || m_Type == CollisionType::TriangleShapeStatic || m_Type == CollisionType::None)
        return;
    if (m_BtShape->getShapeType() != EMPTY_SHAPE_PROXYTYPE) {
        /*
        auto* compound = dynamic_cast<btCompoundShape*>(m_BtShape);
        if (compound) {
            const auto numChildren = compound->getNumChildShapes();
            if (numChildren > 0) {
                for (int i = 0; i < numChildren; ++i) {
                    auto* child_shape = compound->getChildShape(i);
                    if(child_shape)
                        child_shape->calculateLocalInertia(_mass, m_BtInertia);
                }
            }
        }
        */
        m_BtShape->calculateLocalInertia(_mass, m_BtInertia);
    }
}
const btVector3& Collision::getBtInertia() const {
    return m_BtInertia;
}
btCollisionShape* Collision::getBtShape() const { 
    return m_BtShape; 
}
const unsigned int& Collision::getType() const {
    return m_Type; 
}
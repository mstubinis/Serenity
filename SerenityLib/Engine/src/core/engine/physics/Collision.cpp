#include <core/engine/physics/Collision.h>
#include <core/engine/math/Engine_Math.h>

#include <ecs/ComponentModel.h>
#include <core/engine/mesh/Mesh.h>
#include <core/ModelInstance.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <LinearMath/btIDebugDraw.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

void Collision::_init(ComponentBody* body, const vector<Mesh*>& _meshes, const float& _mass) {
    btCompoundShape* btCompound = new btCompoundShape();
    btTransform t = btTransform(btQuaternion(0, 0, 0, 1));
    for (auto& mesh : _meshes) {
        btCollisionShape* btShape = InternalMeshPublicInterface::BuildCollision(mesh, CollisionType::ConvexHull);
        btShape->setUserPointer(body);
        btCompound->addChildShape(t, btShape);
    }
    btCompound->setMargin(0.001f);
    btCompound->recalculateLocalAabb();
    btCompound->setUserPointer(body);
    m_BtShape = btCompound;
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
Collision::Collision(ComponentBody* body, ComponentModel& _modelComponent, const float& _mass) {
    //construtor
    vector<Mesh*> meshes;
    for (uint i = 0; i < _modelComponent.getNumModels(); ++i) {
        meshes.push_back(_modelComponent.getModel(i).mesh());
    }
    _init(body, meshes, _mass);
}
Collision::Collision(const CollisionType::Type _type, Mesh* _mesh, const float& _mass) {
    //construtor
    btCollisionShape* btShape = InternalMeshPublicInterface::BuildCollision(_mesh, _type);
    m_BtShape = btShape;
    _baseInit(_type, _mass);
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


void Collision::setMass(float _mass) {
    if (!m_BtShape || m_Type == CollisionType::TriangleShapeStatic || m_Type == CollisionType::None)
        return;
    if (m_BtShape->getShapeType() != EMPTY_SHAPE_PROXYTYPE) {
        m_BtShape->calculateLocalInertia(_mass, m_BtInertia);
    }
}
const btVector3& Collision::getBtInertia() const {
    return m_BtInertia;
}
btCollisionShape* Collision::getBtShape() const { 
    return m_BtShape; 
}
const uint Collision::getType() const { 
    return m_Type; 
}
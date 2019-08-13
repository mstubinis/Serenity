#include <core/engine/physics/Collision.h>

#include <ecs/ComponentModel.h>
#include <core/engine/mesh/Mesh.h>
#include <core/ModelInstance.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <LinearMath/btIDebugDraw.h>

using namespace Engine;
using namespace std;

void Collision::_init(ComponentBody* body, const vector<Mesh*>& _meshes, const float& _mass) {
    btCompoundShape* compound = new btCompoundShape();
    btTransform t = btTransform(btQuaternion(0, 0, 0, 1));
    for (auto& mesh : _meshes) {
        btCollisionShape* shape = epriv::InternalMeshPublicInterface::BuildCollision(mesh, CollisionType::ConvexHull);
        shape->setUserPointer(body);
        compound->addChildShape(t, shape);
    }
    compound->setMargin(0.001f);
    compound->recalculateLocalAabb();
    compound->setUserPointer(body);
    m_Shape = compound;
}
void Collision::_baseInit(const CollisionType::Type _type, const float& _mass) {
    m_Inertia = btVector3(0.0f, 0.0f, 0.0f);
    m_Type = _type;
    setMass(_mass);
}
Collision::Collision() {
    //construtor
    m_Inertia = btVector3(0.0f, 0.0f, 0.0f);
    m_Type = CollisionType::None;
    m_Shape = nullptr;
    setMass(0.0f);
}
Collision::Collision(btHeightfieldTerrainShape& heightField, const CollisionType::Type _type, const float& _mass) {
    _baseInit(_type, _mass);
    m_Shape = &heightField;
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
    btCollisionShape* shape = epriv::InternalMeshPublicInterface::BuildCollision(_mesh, _type);
    m_Shape = shape;
    _baseInit(_type, _mass);
}
Collision::~Collision() {
    //destructor
    if (m_Shape) {
        /*
        btCompoundShape* compoundCast = dynamic_cast<btCompoundShape*>(m_Shape);
        if (compoundCast) {
            int numChildShapes = compoundCast->getNumChildShapes();
            for (int i = 0; i < numChildShapes; ++i) {
                btCollisionShape* shape = compoundCast->getChildShape(i);
                SAFE_DELETE(shape);
            }
        }
        */
    }
    SAFE_DELETE(m_Shape);
}
Collision::Collision(const Collision& other) {
    //copy constructor
    m_Inertia = other.m_Inertia;
    m_Type = other.m_Type;
    if (other.m_Shape) m_Shape = other.m_Shape;
    else               m_Shape = nullptr;
}
Collision& Collision::operator=(const Collision& other) {
    //copy assignment
    m_Inertia = other.m_Inertia;
    m_Type = other.m_Type;
    if (other.m_Shape) m_Shape = other.m_Shape;
    else               m_Shape = nullptr;
    return *this;
}
Collision::Collision(Collision&& other) noexcept {
    //move constructor
    using std::swap;
    swap(m_Inertia, other.m_Inertia);
    swap(m_Type, other.m_Type);
    swap(m_Shape, other.m_Shape);
    other.m_Shape = nullptr;
}
Collision& Collision::operator=(Collision&& other) noexcept {
    //move assignment
    using std::swap;
    swap(m_Inertia, other.m_Inertia);
    swap(m_Type, other.m_Type);
    swap(m_Shape, other.m_Shape);
    other.m_Shape = nullptr;
    return *this;
}


void Collision::setMass(float _mass) {
    if (!m_Shape || m_Type == CollisionType::TriangleShapeStatic || m_Type == CollisionType::None) 
        return;
    if (m_Shape->getShapeType() != EMPTY_SHAPE_PROXYTYPE) {
        m_Shape->calculateLocalInertia(_mass, m_Inertia);
    }
}
const btVector3& Collision::getInertia() const { 
    return m_Inertia; 
}
btCollisionShape* Collision::getShape() const { 
    return m_Shape; 
}
const uint Collision::getType() const { 
    return m_Type; 
}
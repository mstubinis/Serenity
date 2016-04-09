#include "ObjectDynamic.h"
#include "ObjectDisplay.h"
#include "ShaderProgram.h"
#include "Engine_Renderer.h"
#include "Engine_Physics.h"
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include "Engine_Resources.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"

using namespace Engine;

void ObjectDynamic::setDynamic(bool dynamic){
	if(dynamic){
		Physics::removeRigidBody(this);
		m_RigidBody->setCollisionFlags(btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED);
		Physics::addRigidBody(this);
		m_RigidBody->activate();
	}
	else{
		Physics::removeRigidBody(this);
		m_RigidBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
		clearAllForces();
		Physics::addRigidBody(this);
	}
}
void ObjectDynamic::collisionResponse(ObjectDynamic* other){
	// inherit this virtual method for derived classes for collision detection. 
	// if this collides with other, execute the following code:
}

ObjectDynamic::ObjectDynamic(std::string mesh, std::string mat, glm::v3 pos, glm::vec3 scl, std::string name,Collision* col,Scene* scene): Object(name,scene){
    m_Forward = glm::v3(0,0,-1);
    m_Right = glm::v3(1,0,0);
    m_Up = glm::v3(0,1,0);
    m_Radius = 0;
    m_Visible = true;
    m_BoundingBoxRadius = glm::vec3(0);
    m_DisplayItems.push_back(new DisplayItem(Resources::getMesh(mesh),Resources::getMaterial(mat)));
    m_Color = glm::vec4(1);
    
    m_Collision = col;
    calculateRadius();
    m_Mass = 0.5f * m_Radius;
    if(m_Collision == nullptr){
        if(m_DisplayItems.size() > 0){
            btCompoundShape* shape = new btCompoundShape();
            for(auto item:m_DisplayItems){
                btTransform t;
                glm::mat4 m = glm::mat4(1);
                m = glm::translate(m,item->position);
                m *= glm::mat4_cast(item->orientation);
                m = glm::scale(m,item->scale);
                t.setFromOpenGLMatrix(glm::value_ptr(m));

                shape->addChildShape(t,item->mesh->getCollision()->getCollisionShape());
            }
            m_Collision = new Collision(shape,COLLISION_TYPE_COMPOUND, m_Mass);
        }
    }

    btTransform tr;
    m_Model = glm::m4(1);
    m_Model = glm::translate(m_Model,pos);
    m_Model *= glm::m4(glm::mat4_cast(glm::quat()));
    m_Model = glm::scale(m_Model,glm::v3(glm::vec3(1)));

    tr.setFromOpenGLMatrix(glm::value_ptr(glm::mat4(m_Model)));

    m_MotionState = new btDefaultMotionState(tr);

    calculateRadius();
    m_Mass = 0.5f * m_Radius;

    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(m_Mass,m_MotionState,m_Collision->getCollisionShape(),*(m_Collision->getInertia()));
    m_RigidBody = new btRigidBody(rigidBodyCI);
    m_RigidBody->setSleepingThresholds(0.015f,0.015f);
    m_RigidBody->setFriction(0.3f);
    m_RigidBody->setDamping(0.1f,0.4f);//this makes the objects slowly slow down in space, like air friction

    if(Resources::getCurrentScene() == scene)
        Physics::addRigidBody(m_RigidBody);

    if(m_Parent == nullptr){
        ObjectDynamic::update(0);
    }
	m_Collision->getCollisionShape()->setUserPointer(this);
	m_RigidBody->setUserPointer(this);
}
ObjectDynamic::~ObjectDynamic(){
    Physics::removeRigidBody(m_RigidBody);
    SAFE_DELETE(m_RigidBody);
    SAFE_DELETE(m_MotionState);
	for(auto item:m_DisplayItems) SAFE_DELETE(item);
}
void ObjectDynamic::translate(glm::nType x, glm::nType y, glm::nType z,bool local){
    m_RigidBody->activate();
    btTransform t = m_RigidBody->getWorldTransform();
    btVector3 pos = t.getOrigin();
    glm::v3 p = glm::v3(pos.x(),pos.y(),pos.z());
    if(local){
        p += getForward() * z;
        p += getRight() * x;
        p += getUp() * y;
    }
    else{
        p += glm::vec3(x,y,z);
    }
    setPosition(getPosition() + p);
}
void ObjectDynamic::setColor(float r, float g, float b, float a){
	Math::setColor(m_Color,r,g,b,a);
}
void ObjectDynamic::setColor(glm::vec4 color){ ObjectDynamic::setColor(color.r,color.g,color.b,color.a); }
void ObjectDynamic::translate(glm::v3 t,bool l){ ObjectDynamic::translate(t.x,t.y,t.z,l); }
void ObjectDynamic::update(float dt){
    glm::mat4 m(1);

    btTransform tr;
    m_RigidBody->getMotionState()->getWorldTransform(tr);
    tr.getOpenGLMatrix(glm::value_ptr(m));

    m_Forward = Engine::Math::getForward(m_RigidBody);
    m_Right = Engine::Math::getRight(m_RigidBody);
    m_Up = Engine::Math::getUp(m_RigidBody);

    m_Model = glm::m4(m);
    if(m_Parent != nullptr){
        m_Model =  m_Parent->getModel() * m_Model;
    }
}
void ObjectDynamic::render(GLuint shader,bool debug){
    //add to render queue
    if(shader == 0){
        shader = Resources::getShader("Deferred")->getShaderProgram();
    }
    Engine::Renderer::Detail::RenderManagement::getObjectRenderQueue().push_back(GeometryRenderInfo(this,shader));
}
void ObjectDynamic::draw(GLuint shader, bool debug){
    Camera* camera = Resources::getActiveCamera();
    if((m_DisplayItems.size() == 0 || m_Visible == false) || (!camera->sphereIntersectTest(this)) || (camera->getDistance(this) > 1100 * getRadius()))
        return;	
    glUseProgram(shader);

    glEnablei(GL_BLEND,0);
    glBlendFunci(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,0);

    glUniformMatrix4fv(glGetUniformLocation(shader, "VP" ), 1, GL_FALSE, glm::value_ptr(camera->getViewProjection()));
    glUniform1f(glGetUniformLocation(shader, "far"),camera->getFar());
    glUniform1f(glGetUniformLocation(shader, "C"),1.0f);
    glUniform4f(glGetUniformLocation(shader, "Object_Color"),m_Color.x,m_Color.y,m_Color.z,m_Color.w);

    for(auto item:m_DisplayItems){
        glm::mat4 m = glm::mat4(m_Model);
        m = glm::translate(m,item->position);
        m *= glm::mat4_cast(item->orientation);
        m = glm::scale(m,item->scale);

        glUniform1i(glGetUniformLocation(shader, "Shadeless"),static_cast<int>(item->material->getShadeless()));
        glUniform1f(glGetUniformLocation(shader, "BaseGlow"),item->material->getBaseGlow());
        glUniform1f(glGetUniformLocation(shader, "Specularity"),item->material->getSpecularity());

        glUniformMatrix4fv(glGetUniformLocation(shader, "World" ), 1, GL_FALSE, glm::value_ptr(m));

        for(auto component:item->material->getComponents())
            item->material->bindTexture(component.first,shader,Engine::Resources::getAPI());
        item->mesh->render();
    }
    glUseProgram(0);
}
glm::v3 ObjectDynamic::getPosition(){
    glm::mat4 m(1);
    btTransform tr; m_RigidBody->getMotionState()->getWorldTransform(tr);
    tr.getOpenGLMatrix(glm::value_ptr(m));
    return glm::v3(m[3][0],m[3][1],m[3][2]);
}
void ObjectDynamic::scale(float x,float y,float z){
    btVector3 localScale = m_Collision->getCollisionShape()->getLocalScaling();
    m_Collision->getCollisionShape()->setLocalScaling(btVector3(localScale.x()+x,localScale.y()+y,localScale.z()+z));
    this->calculateRadius();
}
void ObjectDynamic::scale(glm::vec3 s){ ObjectDynamic::scale(s.x,s.y,s.z); }
void ObjectDynamic::setScale(float x, float y, float z){
    m_Collision->getCollisionShape()->setLocalScaling(btVector3(x,y,z));
    this->calculateRadius();
}
void ObjectDynamic::setScale(glm::vec3 s){ ObjectDynamic::setScale(s.x,s.y,s.z); }
void ObjectDynamic::setPosition(glm::nType x, glm::nType y, glm::nType z){
    btTransform initialTransform;

    initialTransform.setOrigin(btVector3(static_cast<btScalar>(x),static_cast<btScalar>(y),static_cast<btScalar>(z)));
    initialTransform.setRotation(m_RigidBody->getOrientation());

    m_RigidBody->setWorldTransform(initialTransform);
    m_RigidBody->setCenterOfMassTransform(initialTransform);
    m_MotionState->setWorldTransform(initialTransform);
}
void ObjectDynamic::setOrientation(glm::quat q){
    btTransform t;
    btQuaternion quat(q.x,q.y,q.z,q.w);

    t.setOrigin(m_RigidBody->getWorldTransform().getOrigin());
    t.setRotation(quat);

    m_RigidBody->setWorldTransform(t);
    m_RigidBody->setCenterOfMassTransform(t);
    m_MotionState->setWorldTransform(t);

    clearAngularForces();
}
void ObjectDynamic::setPosition(glm::v3 p){ ObjectDynamic::setPosition(p.x,p.y,p.z); }
void ObjectDynamic::applyForce(float x,float y,float z,bool local){ 
    m_RigidBody->activate();
    if(local){
        glm::vec3 res = glm::vec3(getRight()) * x;
        res += glm::vec3(getUp()) * y;
        res += glm::vec3(getForward()) * z;
        x = res.x; y = res.y; z = res.z;
    }
    m_RigidBody->applyCentralForce(btVector3(x,y,z)); 
}
void ObjectDynamic::applyForce(glm::vec3 force,glm::vec3 relPos,bool local){ 
    m_RigidBody->activate();
    if(local){
        glm::vec3 res = glm::vec3(getRight()) * force.x;
        res += glm::vec3(getUp()) * force.y;
        res += glm::vec3(getForward()) * force.z;
        force.x = res.x; force.y = res.y; force.z = res.z;
    }
    m_RigidBody->applyForce(btVector3(force.x,force.y,force.z),btVector3(relPos.x,relPos.y,relPos.z)); 
}
void ObjectDynamic::applyForceX(float x,bool l){ ObjectDynamic::applyForce(x,0,0,l); }
void ObjectDynamic::applyForceY(float y,bool l){ ObjectDynamic::applyForce(0,y,0,l); }
void ObjectDynamic::applyForceZ(float z,bool l){ ObjectDynamic::applyForce(0,0,z,l); }
void ObjectDynamic::applyImpulse(float x,float y,float z,bool local){ 
    m_RigidBody->activate();
    if(local){
        glm::vec3 res = glm::vec3(getForward()) * z; 
        res += glm::vec3(getUp()) * y;
        res += glm::vec3(getRight()) * x;
        x = res.x; y = res.y; z = res.z;
    }
    m_RigidBody->applyCentralImpulse(btVector3(x,y,z));
}
void ObjectDynamic::applyImpulse(glm::vec3 impulse,glm::vec3 relPos,bool local){ 
    m_RigidBody->activate();
    if(local){
        glm::vec3 res = glm::vec3(getForward()) * impulse.z; 
        res += glm::vec3(getUp()) * impulse.y;
        res += glm::vec3(getRight()) * impulse.x;
        impulse.x = res.x; impulse.y = res.y; impulse.z = res.z;
    }
    m_RigidBody->applyImpulse(btVector3(impulse.x,impulse.y,impulse.z),btVector3(relPos.x,relPos.y,relPos.z));
}
void ObjectDynamic::applyImpulseX(float x,bool l){ ObjectDynamic::applyImpulse(x,0,0,l); }
void ObjectDynamic::applyImpulseY(float y,bool l){ ObjectDynamic::applyImpulse(0,y,0,l); }
void ObjectDynamic::applyImpulseZ(float z,bool l){ ObjectDynamic::applyImpulse(0,0,z,l); }
void ObjectDynamic::applyTorque(float x,float y,float z,bool local){
    m_RigidBody->activate();
    btVector3 t(x,y,z);
    if(local){
        t = m_RigidBody->getInvInertiaTensorWorld().inverse()*(m_RigidBody->getWorldTransform().getBasis()*t);
    }
    m_RigidBody->applyTorque(t);
}
void ObjectDynamic::applyTorque(glm::vec3 t,bool l){ ObjectDynamic::applyTorque(t.x,t.y,t.z,l); }
void ObjectDynamic::applyTorqueX(float x,bool l){ ObjectDynamic::applyTorque(x,0,0,l); }
void ObjectDynamic::applyTorqueY(float y,bool l){ ObjectDynamic::applyTorque(0,y,0,l); }
void ObjectDynamic::applyTorqueZ(float z,bool l){ ObjectDynamic::applyTorque(0,0,z,l); }
void ObjectDynamic::applyTorqueImpulse(float x,float y,float z,bool local){
    m_RigidBody->activate();
    btVector3 t(x,y,z);
    if(local){
        t = m_RigidBody->getInvInertiaTensorWorld().inverse()*(m_RigidBody->getWorldTransform().getBasis()*t);
    }
    m_RigidBody->applyTorqueImpulse(t);
}
void ObjectDynamic::applyTorqueImpulse(glm::vec3 t,bool l){ ObjectDynamic::applyTorqueImpulse(t.x,t.y,t.z,l); }
void ObjectDynamic::applyTorqueImpulseX(float x,bool l){ ObjectDynamic::applyTorqueImpulse(x,0,0,l); }
void ObjectDynamic::applyTorqueImpulseY(float y,bool l){ ObjectDynamic::applyTorqueImpulse(0,y,0,l); }
void ObjectDynamic::applyTorqueImpulseZ(float z,bool l){ ObjectDynamic::applyTorqueImpulse(0,0,z,l); }
void ObjectDynamic::setLinearVelocity(float x, float y, float z, bool local){
    m_RigidBody->activate();
    if(local){
        glm::vec3 res = glm::vec3(getRight()) * x;
        res += glm::vec3(getUp()) * y;
        res += glm::vec3(getForward()) * z;
        x = res.x; y = res.y; z = res.z;
    }
    m_RigidBody->setLinearVelocity(btVector3(x,y,z)); 
}
void ObjectDynamic::setLinearVelocity(glm::vec3 v, bool l){ ObjectDynamic::setLinearVelocity(v.x,v.y,v.z,l); }
void ObjectDynamic::setLinearVelocityX(float x, bool l){ 
    btVector3 v = m_RigidBody->getLinearVelocity();
    ObjectDynamic::setLinearVelocity(x,v.y(),v.z(),l); 
}
void ObjectDynamic::setLinearVelocityY(float y, bool l){ 
    btVector3 v = m_RigidBody->getLinearVelocity();
    ObjectDynamic::setLinearVelocity(v.x(),y,v.z(),l); 
}
void ObjectDynamic::setLinearVelocityZ(float z, bool l){ 
    btVector3 v = m_RigidBody->getLinearVelocity();
    ObjectDynamic::setLinearVelocity(v.x(),v.y(),z,l); 
}
void ObjectDynamic::setAngularVelocity(float x, float y, float z,bool local){ 
    m_RigidBody->activate();
    if(local){
        glm::vec3 res = glm::vec3(getRight()) * x;
        res += glm::vec3(getUp()) * y;
        res += glm::vec3(getForward()) * z;
        x = res.x; y = res.y; z = res.z;
    }
    m_RigidBody->setAngularVelocity(btVector3(x,y,z)); 
}
void ObjectDynamic::setAngularVelocity(glm::vec3 v,bool l){ ObjectDynamic::setAngularVelocity(v.x,v.y,v.z,l); }
void ObjectDynamic::setAngularVelocityX(float x, bool l){ 
    btVector3 v = m_RigidBody->getAngularVelocity();
    ObjectDynamic::setAngularVelocity(x,v.y(),v.z(),l); 
}
void ObjectDynamic::setAngularVelocityY(float y, bool l){ 
    btVector3 v = m_RigidBody->getAngularVelocity();
    ObjectDynamic::setAngularVelocity(v.x(),y,v.z(),l); 
}
void ObjectDynamic::setAngularVelocityZ(float z, bool l){ 
    btVector3 v = m_RigidBody->getAngularVelocity();
    ObjectDynamic::setAngularVelocity(v.x(),v.y(),z,l); 
}
void ObjectDynamic::setMass(float mass){
    m_Mass = mass;
    m_Collision->setMass(m_Mass);
    if(m_RigidBody != nullptr)
        m_RigidBody->setMassProps(m_Mass,*(m_Collision->getInertia()));
}
void ObjectDynamic::alignTo(glm::v3 direction, float time,bool overTime){
    ObjectDynamic::clearAngularForces();
    btQuaternion btQ = m_RigidBody->getOrientation();
    glm::quat q(btQ.x(),btQ.y(),btQ.z(),btQ.w());
    Engine::Math::alignTo(q,glm::vec3(direction), time, overTime);
    btQ.setX(q.w); btQ.setY(q.x); btQ.setZ(q.y); btQ.setW(q.z);
    m_RigidBody->getWorldTransform().setRotation(btQ);
}
void ObjectDynamic::rotate(float x,float y,float z,bool overTime){
    ObjectDynamic::clearAngularForces();
    if(overTime){
        x *= Resources::dt(); y *= Resources::dt(); z *= Resources::dt();
    }
    float threshold = 0;
    if(abs(x) < threshold && abs(y) < threshold && abs(z) < threshold)
        return;

    if(abs(x) >= threshold) this->applyTorqueY(-x);   //pitch
    if(abs(y) >= threshold) this->applyTorqueX(-y);   //yaw
    if(abs(z) >= threshold) this->applyTorqueZ(z);   //roll

    m_Forward = Engine::Math::getForward(m_RigidBody);
    m_Right = Engine::Math::getRight(m_RigidBody);
    m_Up = Engine::Math::getUp(m_RigidBody);
}
void ObjectDynamic::rotate(glm::vec3 r, bool overTime){ ObjectDynamic::rotate(r.x,r.y,r.z,overTime); }
void ObjectDynamic::clearLinearForces(){
    m_RigidBody->setActivationState(0);
    ObjectDynamic::setLinearVelocity(0,0,0);
}
void ObjectDynamic::clearAngularForces(){
    m_RigidBody->setActivationState(0);
    ObjectDynamic::setAngularVelocity(0,0,0);
}
void ObjectDynamic::clearAllForces(){
    m_RigidBody->setActivationState(0);
    ObjectDynamic::setLinearVelocity(0,0,0);
    ObjectDynamic::setAngularVelocity(0,0,0);
}

bool ObjectDynamic::rayIntersectSphere(Camera* cam){ return cam->rayIntersectSphere(this); }
void ObjectDynamic::calculateRadius(){
    if(m_DisplayItems.size() == 0){
        m_BoundingBoxRadius = glm::vec3(0);
        return;
    }
    float maxLength = 0;
    for(auto item:m_DisplayItems){
        float length = 0;
        glm::mat4 m = glm::mat4(1);
        m = glm::translate(m,item->position);
        m *= glm::mat4_cast(item->orientation);
        m = glm::scale(m,item->scale);

        glm::vec3 localPosition = glm::vec3(m[3][0],m[3][1],m[3][2]);
        
        length = glm::length(localPosition) + item->mesh->getRadius() * glm::max(glm::abs(item->scale.z), glm::max(glm::abs(item->scale.x),glm::abs(item->scale.y)));

        if(length > maxLength){
            maxLength = length;
        }
    }
    glm::vec3 scale(1);
    if(m_Collision != nullptr){
        btVector3 s = m_Collision->getCollisionShape()->getLocalScaling();
        scale = glm::vec3(s.x(),s.y(),s.z());
    }
    m_BoundingBoxRadius = maxLength * scale;
    m_Radius = glm::max(glm::abs(m_BoundingBoxRadius.x),glm::max(glm::abs(m_BoundingBoxRadius.y),glm::abs(m_BoundingBoxRadius.z)));
}
bool ObjectDynamic::rayIntersectSphere(glm::v3 A, glm::vec3 rayVector){
    glm::vec3 a1 = glm::vec3(A);
    glm::vec3 B = a1 + rayVector;

    glm::vec3 C = glm::vec3(getPosition());
    float r = getRadius();

    //check if point is behind
    float dot = glm::dot(rayVector,C-a1);
    if(dot >= 0)
        return false;

    glm::nType a = ((B.x-A.x)*(B.x-A.x))  +  ((B.y - A.y)*(B.y - A.y))  +  ((B.z - A.z)*(B.z - A.z));
    glm::nType b = 2* ((B.x - A.x)*(A.x - C.x)  +  (B.y - A.y)*(A.y - C.y)  +  (B.z - A.z)*(A.z-C.z));
    glm::nType c = (((A.x-C.x)*(A.x-C.x))  +  ((A.y - C.y)*(A.y - C.y))  +  ((A.z - C.z)*(A.z - C.z))) - (r*r);

    glm::nType Delta = (b*b) - (4*a*c);

    if(Delta < 0)
        return false;
    return true;
}

glm::quat ObjectDynamic::getOrientation(){
    btQuaternion q = m_RigidBody->getOrientation();
    return glm::quat(q.w(),q.x(),q.y(),q.z());
}
glm::vec3 ObjectDynamic::getScale(){
    btVector3 localScale = m_Collision->getCollisionShape()->getLocalScaling();
    return glm::vec3(localScale.x(),localScale.y(),localScale.z());
}
glm::m4 ObjectDynamic::getModel(){
    glm::mat4 m1(1);
    btTransform tr;
    m_RigidBody->getMotionState()->getWorldTransform(tr);
    tr.getOpenGLMatrix(glm::value_ptr(m1));
    return glm::m4(m1);
}
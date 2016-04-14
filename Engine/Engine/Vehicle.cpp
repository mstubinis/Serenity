#include "Vehicle.h"
#include "Engine_Physics.h"

using namespace Engine;

Wheel::Wheel(std::string mesh,std::string mat,glm::v3 pos,glm::vec3 scl,std::string name,Collision* collision,Scene* scene):ObjectDynamic(mesh,mat,pos,scl,name,collision,scene){
}
Wheel::~Wheel(){
}
void Wheel::on_applyForce(float,float,float,bool local){
}
void Wheel::on_setLinearVelocity(float,float,float,bool local){
}
void Wheel::on_setAngularVelocity(float,float,float,bool local){
}

Vehicle::Vehicle(std::string mesh,std::string mat,glm::v3 pos,glm::vec3 scl,std::string name,Collision* collision,Scene* scene):ObjectDynamic(mesh,mat,pos,scl,name,collision,scene){
	m_Tuning = new btRaycastVehicle::btVehicleTuning();
	m_Raycaster = new btDefaultVehicleRaycaster(Physics::Detail::PhysicsManagement::m_dynamicsWorld);
	m_Vehicle = new btRaycastVehicle(*m_Tuning,m_RigidBody,m_Raycaster);
}
Vehicle::~Vehicle(){
	delete(m_Tuning);
	delete(m_Raycaster);
	delete(m_Vehicle);
}
void Vehicle::setFrictionSlip(float slip){ m_Tuning->m_frictionSlip = slip; }
void Vehicle::setMaxSuspensionForce(float max){ m_Tuning->m_maxSuspensionForce = max; }
void Vehicle::setMaxSuspensionTravelCm(float max){ m_Tuning->m_maxSuspensionTravelCm = max; }
void Vehicle::setSuspensionCompression(float compression){ m_Tuning->m_suspensionCompression = compression; }
void Vehicle::setSuspensionDamping(float damping){ m_Tuning->m_suspensionDamping = damping; }
void Vehicle::setSuspensionStiffness(float stiffness){ m_Tuning->m_suspensionStiffness = stiffness; }
void Vehicle::applyEngineForce(float force, unsigned int wheelIndex){ m_Vehicle->applyEngineForce(force,wheelIndex); }
void Vehicle::setBrake(float force, unsigned int wheelIndex){ m_Vehicle->setBrake(force,wheelIndex); }
void Vehicle::setSteeringValue(float value, unsigned int wheelIndex){ m_Vehicle->setSteeringValue(value,wheelIndex); }
void Vehicle::resetSuspension(){ m_Vehicle->resetSuspension(); }
float Vehicle::getCurrentSpeedKmHour(){ return m_Vehicle->getCurrentSpeedKmHour(); }

void Vehicle::addWheel(Wheel* wheel,glm::v3 p,float suspensionRestLength,bool isFront){
	m_Wheels.push_back(wheel);
	btVector3 pos = btVector3(btScalar(p.x),btScalar(p.y),btScalar(p.z));
	m_Vehicle->addWheel(pos,btVector3(0,0,-1),btVector3(1,0,0),suspensionRestLength,wheel->getRadius(),*m_Tuning,isFront);
}

void Vehicle::applyForce(float x,float y,float z,bool local){
	ObjectDynamic::applyForce(x,y,z,local);
	for(auto wheel:m_Wheels)
		wheel->on_applyForce(x,y,z,local);
}
void Vehicle::applyForce(glm::vec3 force,glm::vec3 relPos,bool local){ Vehicle::applyForce(force.x,force.y,force.z,local); }
void Vehicle::applyForceX(float x,bool l){ Vehicle::applyForce(x,0,0,l); }
void Vehicle::applyForceY(float y,bool l){ Vehicle::applyForce(0,y,0,l); }
void Vehicle::applyForceZ(float z,bool l){ Vehicle::applyForce(0,0,z,l); }

void Vehicle::setLinearVelocity(float x, float y, float z, bool local){
	ObjectDynamic::setLinearVelocity(x,y,z,local);
	for(auto wheel:m_Wheels)
		wheel->on_setLinearVelocity(x,y,z,local);
}
void Vehicle::setLinearVelocity(glm::vec3 v, bool l){ Vehicle::setLinearVelocity(v.x,v.y,v.z,l); }
void Vehicle::setLinearVelocityX(float x, bool l){ 
    btVector3 v = m_RigidBody->getLinearVelocity();
    Vehicle::setLinearVelocity(x,v.y(),v.z(),l); 
}
void Vehicle::setLinearVelocityY(float y, bool l){ 
    btVector3 v = m_RigidBody->getLinearVelocity();
    Vehicle::setLinearVelocity(v.x(),y,v.z(),l); 
}
void Vehicle::setLinearVelocityZ(float z, bool l){ 
    btVector3 v = m_RigidBody->getLinearVelocity();
    Vehicle::setLinearVelocity(v.x(),v.y(),z,l); 
}
void Vehicle::setAngularVelocity(float x, float y, float z,bool local){ 
	ObjectDynamic::setAngularVelocity(x,y,z,local);
	for(auto wheel:m_Wheels)
		wheel->on_setAngularVelocity(x,y,z,local);
}
void Vehicle::setAngularVelocity(glm::vec3 v,bool l){ Vehicle::setAngularVelocity(v.x,v.y,v.z,l); }
void Vehicle::setAngularVelocityX(float x, bool l){ 
    btVector3 v = m_RigidBody->getAngularVelocity();
    Vehicle::setAngularVelocity(x,v.y(),v.z(),l); 
}
void Vehicle::setAngularVelocityY(float y, bool l){ 
    btVector3 v = m_RigidBody->getAngularVelocity();
    Vehicle::setAngularVelocity(v.x(),y,v.z(),l); 
}
void Vehicle::setAngularVelocityZ(float z, bool l){ 
    btVector3 v = m_RigidBody->getAngularVelocity();
    Vehicle::setAngularVelocity(v.x(),v.y(),z,l); 
}
void Vehicle::update(float dt){
	m_Vehicle->updateAction(Physics::Detail::PhysicsManagement::m_dynamicsWorld,dt);
	m_Vehicle->updateFriction(dt);
	m_Vehicle->updateSuspension(dt);
	m_Vehicle->updateVehicle(dt);
	for(int i = 0; i < m_Vehicle->getNumWheels(); i++)
		m_Vehicle->updateWheelTransform(i);
	ObjectDynamic::update(dt);
}
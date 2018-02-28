#include "Components.h"
#include "Engine_Resources.h"
#include "Engine.h"
#include "Object.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;
using namespace std;

struct epriv::MeshMaterialPair final{
	Mesh* _mesh;
	Material* _material;
	glm::mat4 _modelMatrix;
	glm::vec3 _position, _scale, _forward, _right, _up;
	glm::quat _rotation;
	MeshMaterialPair(Mesh* mesh,Material* material){
		_mesh = mesh; _material = material; _modelMatrix = glm::mat4(1.0f);
		_position = glm::vec3(0.0f); _rotation = glm::quat(); _scale = glm::vec3(1.0f);
		Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
	}

	void translate(glm::vec3& translation){ translate(translation.x,translation.y,translation.z); }
	void translate(float x,float y,float z){
		_position.x += x; _position.y += y; _position.z += z;
	}
	void setPosition(glm::vec3& newPosition){ setPosition(newPosition.x,newPosition.y,newPosition.z); }
	void setPosition(float x,float y,float z){
		_position.x = x; _position.y = y; _position.z = z;
	}
	void rotate(glm::vec3& rotation){ rotate(rotation.x,rotation.y,rotation.z); }
	void rotate(float pitch,float yaw,float roll){
		if(abs(pitch) < Object::m_RotationThreshold && abs(yaw) < Object::m_RotationThreshold && abs(roll) < Object::m_RotationThreshold)
			return;
		if(abs(pitch) >= Object::m_RotationThreshold) _rotation = _rotation * (glm::angleAxis(-pitch, glm::vec3(1,0,0)));   //pitch
		if(abs(yaw) >= Object::m_RotationThreshold) _rotation = _rotation * (glm::angleAxis(-yaw, glm::vec3(0,1,0)));   //yaw
		if(abs(roll) >= Object::m_RotationThreshold) _rotation = _rotation * (glm::angleAxis(roll,  glm::vec3(0,0,1)));   //roll
		Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
	}
	void scale(glm::vec3& amount){ scale(amount.x,amount.y,amount.z); }
	void scale(float x,float y,float z){
		_scale.x += x; _scale.y += y; _scale.z += z;
	}
	void setRotation(glm::quat& newRotation){
		newRotation = glm::normalize(newRotation);
		_rotation = newRotation;
		Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
	}
	void setRotation(float x,float y,float z,float w){ setRotation(glm::quat(w,x,y,z)); }
	void setScale(glm::vec3& newScale){ setScale(newScale.x,newScale.y,newScale.z); }
	void setScale(float x,float y,float z){
		_scale.x = x; _scale.y = y; _scale.z = z;
	}
};


class epriv::ComponentManager::impl final{
    public:

		vector<ComponentTransform*> m_ComponentTransforms;
		vector<ComponentModel*>     m_ComponentModels;

		//implement this workflow somehow with ComponentModels

		//list of shaders
		    //bind shader
		    //list of materials
		       //bind material
		       //list of meshes
		          //bind mesh
		          //list of objects using meshes
		              //bind object
		              //render object
		              //unbind object
		          //unbind mesh
		       //unbind material
		    //unbind shader

		void _init(const char* name, uint& w, uint& h){
		}
		void _postInit(const char* name, uint& w, uint& h){
		}
		void _destruct(){
			for(auto c:m_ComponentTransforms) delete(c);
			for(auto c:m_ComponentModels)     delete(c);
		}

		void _performTransformation(Object* parent,glm::vec3& position,glm::quat& rotation,glm::vec3& scale,glm::mat4& modelMatrix){
			if(parent == nullptr){
				modelMatrix = glm::mat4(1.0f);
			}
			else{
				modelMatrix = parent->getModel();
			}
			glm::mat4 translationMat = glm::translate(position);
			glm::mat4 rotationMat = glm::mat4_cast(rotation);
			glm::mat4 scaleMat = glm::scale(scale);
			modelMatrix = translationMat * rotationMat * scaleMat * modelMatrix;
		}
		void _updateComponentTransforms(float& dt){
			for(auto c:m_ComponentTransforms){
				_performTransformation(c->m_Owner->parent(),c->_position,c->_rotation,c->_scale,c->_modelMatrix);
			}
		}
		void _updateComponentModels(float& dt){
			for(auto c:m_ComponentModels){ 
				for(auto model:c->models){
					if(model._mesh != nullptr){
						_performTransformation(c->m_Owner->parent(),model._position,model._rotation,model._scale,model._modelMatrix);
					}
				}
			}
		}
		void _update(float& dt){
			_updateComponentTransforms(dt);
			_updateComponentModels(dt);
		}
		void _render(){
			
		}
};
epriv::ComponentManager::ComponentManager(const char* name, uint w, uint h):m_i(new impl){ m_i->_init(name,w,h); }
epriv::ComponentManager::~ComponentManager(){ m_i->_destruct(); }

void epriv::ComponentManager::_init(const char* name, uint w, uint h){ m_i->_postInit(name,w,h); }
void epriv::ComponentManager::_update(float& dt){ m_i->_update(dt); }





ComponentBaseClass::ComponentBaseClass(Object* owner){
	m_Owner = owner;
}
ComponentBaseClass::~ComponentBaseClass(){}
void ComponentBaseClass::setOwner(Object* owner){
	m_Owner = owner;
}



ComponentTransform::ComponentTransform(Object* owner):ComponentBaseClass(owner){
	_position = glm::vec3(0.0f);
	_scale = glm::vec3(1.0f);
	_rotation = glm::quat();
	_modelMatrix = glm::mat4(1.0f);
	Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
}
ComponentTransform::~ComponentTransform(){
}
void ComponentTransform::translate(glm::vec3& translation){ ComponentTransform::translate(translation.x,translation.y,translation.z); }
void ComponentTransform::translate(float x,float y,float z){
	_position.x += x; _position.y += y; _position.z += z;
}
void ComponentTransform::setPosition(glm::vec3& newPosition){ ComponentTransform::setPosition(newPosition.x,newPosition.y,newPosition.z); }
void ComponentTransform::setPosition(float x,float y,float z){
	_position.x = x; _position.y = y; _position.z = z;
}
void ComponentTransform::rotate(glm::vec3& rotation){ ComponentTransform::rotate(rotation.x,rotation.y,rotation.z); }
void ComponentTransform::rotate(float pitch,float yaw,float roll){
    if(abs(pitch) < Object::m_RotationThreshold && abs(yaw) < Object::m_RotationThreshold && abs(roll) < Object::m_RotationThreshold)
        return;
    if(abs(pitch) >= Object::m_RotationThreshold) _rotation = _rotation * (glm::angleAxis(-pitch, glm::vec3(1,0,0)));   //pitch
    if(abs(yaw) >= Object::m_RotationThreshold) _rotation = _rotation * (glm::angleAxis(-yaw, glm::vec3(0,1,0)));   //yaw
    if(abs(roll) >= Object::m_RotationThreshold) _rotation = _rotation * (glm::angleAxis(roll,  glm::vec3(0,0,1)));   //roll
    Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
}
void ComponentTransform::scale(glm::vec3& amount){ ComponentTransform::scale(amount.x,amount.y,amount.z); }
void ComponentTransform::scale(float x,float y,float z){
	_scale.x += x; _scale.y += y; _scale.z += z;
}
void ComponentTransform::setRotation(glm::quat& newRotation){
    newRotation = glm::normalize(newRotation);
    _rotation = newRotation;
    Engine::Math::recalculateForwardRightUp(_rotation,_forward,_right,_up);
}
void ComponentTransform::setRotation(float x,float y,float z,float w){ ComponentTransform::setRotation(glm::quat(w,x,y,z)); }
void ComponentTransform::setScale(glm::vec3& newScale){ ComponentTransform::setScale(newScale.x,newScale.y,newScale.z); }
void ComponentTransform::setScale(float x,float y,float z){
	_scale.x = x; _scale.y = y; _scale.z = z;
}



ComponentModel::ComponentModel(Mesh* mesh,Material* material,Object* owner):ComponentBaseClass(owner){
	if(mesh && material){
		models.push_back( epriv::MeshMaterialPair(mesh,material) );
	}
}
ComponentModel::~ComponentModel(){
	models.clear();
}
uint ComponentModel::addModel(Handle& meshHandle, Handle& materialHandle){ return ComponentModel::addModel(Resources::getMesh(meshHandle),Resources::getMaterial(materialHandle)); }
uint ComponentModel::addModel(Mesh* mesh,Material* material){
	models.push_back( epriv::MeshMaterialPair(mesh,material) );
	return models.size();
}

void ComponentModel::setModel(Handle& meshHandle,Handle& materialHandle,uint index){ ComponentModel::setModel(Resources::getMesh(meshHandle),Resources::getMaterial(materialHandle), index); }
void ComponentModel::setModel(Mesh* mesh,Material* material,uint index){
	epriv::MeshMaterialPair& pair = models.at(index);
	pair._mesh = mesh;
	pair._material = material;
}

void ComponentModel::setModelMesh(Mesh* mesh,uint index){
	epriv::MeshMaterialPair& pair = models.at(index);
	pair._mesh = mesh;
}
void ComponentModel::setModelMesh(Handle& meshHandle, uint index){ ComponentModel::setModelMesh(Resources::getMesh(meshHandle),index); }
		
void ComponentModel::setModelMaterial(Material* material,uint index){
	epriv::MeshMaterialPair& pair = models.at(index);
	pair._material = material;
}
void ComponentModel::setModelMaterial(Handle& materialHandle,uint index){ ComponentModel::setModelMaterial(Resources::getMaterial(materialHandle),index); }
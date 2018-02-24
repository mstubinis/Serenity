#include "Components.h"
#include "Engine.h"
#include "Object.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;
using namespace std;

struct epriv::MeshMaterialPair final{
	Mesh* mesh;
	Material* material;
	glm::mat4 modelMatrix;
	glm::vec3 position, scale;
	glm::quat rotation;
	MeshMaterialPair(Mesh* _mesh,Material* _material){
		mesh = _mesh; material = _material; modelMatrix = glm::mat4(1.0f);
		position = glm::vec3(0.0f); rotation = glm::quat(); scale = glm::vec3(1.0f);
	}
};


class epriv::ComponentManager::impl final{
    public:
		vector<ComponentTransform*> m_ComponentTransforms;
		vector<ComponentModel*> m_ComponentModels;

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
				_performTransformation(c->m_Owner->parent(),c->position,c->rotation,c->scale,c->modelMatrix);
			}
		}
		void _updateComponentModels(float& dt){
			for(auto c:m_ComponentModels){ 
				for(auto model:c->models){
					if(model.mesh != nullptr){
						_performTransformation(c->m_Owner->parent(),model.position,model.rotation,model.scale,model.modelMatrix);
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





epriv::ComponentBaseClass::ComponentBaseClass(Object* owner){
	m_Owner = owner;
}
epriv::ComponentBaseClass::~ComponentBaseClass(){}
void epriv::ComponentBaseClass::setOwner(Object* owner){
	m_Owner = owner;
}



ComponentTransform::ComponentTransform(Object* owner):epriv::ComponentBaseClass(owner){
	position = glm::vec3(0.0f);
	scale = glm::vec3(1.0f);
	rotation = glm::quat();
	modelMatrix = glm::mat4(1.0f);
}
ComponentTransform::~ComponentTransform(){
}


ComponentModel::ComponentModel(Mesh* mesh,Material* material,Object* owner):epriv::ComponentBaseClass(owner){
	if(mesh && material){
		models.push_back( epriv::MeshMaterialPair(mesh,material) );
	}
}
ComponentModel::~ComponentModel(){
	models.clear();
}
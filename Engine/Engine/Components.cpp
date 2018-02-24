#include "Components.h"
#include "Engine.h"

using namespace Engine;
using namespace std;


class epriv::ComponentManager::impl final{
    public:


		void _init(const char* name, uint& w, uint& h){
		}
		void _postInit(const char* name, uint& w, uint& h){
		}
		void _destruct(){
		}
		void _update(float& dt){
		}
};
epriv::ComponentManager::ComponentManager(const char* name, uint w, uint h):m_i(new impl){ m_i->_init(name,w,h); }
epriv::ComponentManager::~ComponentManager(){ m_i->_destruct(); }

void epriv::ComponentManager::_init(const char* name, uint w, uint h){ m_i->_postInit(name,w,h); }
void epriv::ComponentManager::_update(float& dt){ m_i->_update(dt); }





epriv::ComponentBaseClass::ComponentBaseClass(Object* owner){
	m_Owner = owner;
}
epriv::ComponentBaseClass::~ComponentBaseClass(){
}
void epriv::ComponentBaseClass::setOwner(Object* owner){
	m_Owner = owner;
}
void epriv::ComponentBaseClass::update(float& dt){
}




ComponentTransform::ComponentTransform(Object* owner):epriv::ComponentBaseClass(owner){
}
ComponentTransform::~ComponentTransform(){
}
void ComponentTransform::update(float& dt){
}
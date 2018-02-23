#include "Engine_EventDispatcher.h"
#include "Engine_ResourceBasic.h"
#include "Object.h"
#include <vector>

using namespace Engine;
using namespace std;

class epriv::EventDispatcher::impl final{
    public:

		vector<vector<Object*>> m_Objects;

		void _init(const char* name,uint& w,uint& h){
			m_Objects.resize(EventType::ZZZTotal); //replace later with a constant?
		}
		void _postInit(const char* name,uint& w,uint& h){
		}
		void _destruct(){
			for(auto v:m_Objects){
				v.clear();
			}
			m_Objects.clear();
		}
		void _update(float& dt){
		}
		void _registerObject(Object* obj, EventType::Type& type){
			vector<Object*>& v = m_Objects.at(type);
			for(auto o:v){ if (o == obj){ return; } }
			v.push_back(obj);
		}
		void _unregisterObject(Object* obj, EventType::Type& type){
			vector<Object*>& v = m_Objects.at(type);
			for(uint i = 0; i < v.size(); ++i){
				if(v.at(i) == obj){ v.erase(v.begin() + i); break; }
			}
		}
		void _dispatchEvent(EventType::Type& type,const Event& e){
			vector<Object*>& v = m_Objects.at(type);
			for(auto obj:v){
				obj->onEvent(e);
			}
		}
};


epriv::EventDispatcher::EventDispatcher(const char* name,uint w,uint h):m_i(new impl){ m_i->_init(name,w,h); }
epriv::EventDispatcher::~EventDispatcher(){ m_i->_destruct(); }
void epriv::EventDispatcher::_init(const char* name,uint w,uint h){ m_i->_postInit(name,w,h); }
void epriv::EventDispatcher::_update(float dt){ m_i->_update(dt); }
void epriv::EventDispatcher::_registerObject(Object* obj,EventType::Type type){ m_i->_registerObject(obj,type); }
void epriv::EventDispatcher::_unregisterObject(Object* obj,EventType::Type type){ m_i->_unregisterObject(obj,type); }
void epriv::EventDispatcher::_dispatchEvent(EventType::Type type,const Event& e){ m_i->_dispatchEvent(type,e); }
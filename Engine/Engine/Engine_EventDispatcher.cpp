#include "Engine_EventDispatcher.h"
#include "Engine_ResourceBasic.h"
#include "Components.h"
#include <vector>

using namespace Engine;
using namespace std;

class epriv::EventDispatcher::impl final{
    public:
        vector<vector<EventObserver*>> m_Observers;

        void _init(const char* name,uint& w,uint& h){
            m_Observers.resize(EventType::_TOTAL); //replace later with a constant?
        }
        void _postInit(const char* name,uint& w,uint& h){
        }
        void _destruct(){
            for(auto v:m_Observers){
                vector_clear(v);
            }
            vector_clear(m_Observers);
        }
        void _update(const float& dt){
        }
        void _registerObject(EventObserver* obj, EventType::Type type){
            auto& v = m_Observers.at(type);
            for(auto o:v){ if (o == obj){ return; } }
            v.push_back(obj);
        }
        void _unregisterObject(EventObserver* obj, EventType::Type type){
            auto& v = m_Observers.at(type);
			removeFromVector(v,obj);
        }
        void _dispatchEvent(EventType::Type type,const Event& e){
            auto& v = m_Observers.at(type);
            for(auto obj:v){
                obj->onEvent(e);
            }
        }
};


epriv::EventDispatcher::EventDispatcher(const char* name,uint w,uint h):m_i(new impl){ m_i->_init(name,w,h); }
epriv::EventDispatcher::~EventDispatcher(){ m_i->_destruct(); }
void epriv::EventDispatcher::_init(const char* name,uint w,uint h){ m_i->_postInit(name,w,h); }
void epriv::EventDispatcher::_update(const float& dt){ m_i->_update(dt); }
void epriv::EventDispatcher::_registerObject(EventObserver* obj,EventType::Type type){ m_i->_registerObject(obj,type); }
void epriv::EventDispatcher::_unregisterObject(EventObserver* obj,EventType::Type type){ m_i->_unregisterObject(obj,type); }
void epriv::EventDispatcher::_dispatchEvent(EventType::Type type,const Event& e){ m_i->_dispatchEvent(type,e); }
void epriv::EventDispatcher::_dispatchEvent(const Event& e){ m_i->_dispatchEvent((EventType::Type)e.type,e); }
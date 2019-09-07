#include <core/engine/BindableResource.h>

namespace Engine{
    namespace epriv{
        struct EmptyFunctor{template<class T> void operator()(T* r) const {}};
    };
};

BindableResource::BindableResource(std::string name):EngineResource(name){
    Engine::epriv::EmptyFunctor a;
    setCustomBindFunctor(a); 
    setCustomUnbindFunctor(a);
}
BindableResource::~BindableResource(){
}
void BindableResource::bind(){ 
    m_CustomBindFunctor(); 
}
void BindableResource::unbind(){ 
    m_CustomUnbindFunctor(); 
}

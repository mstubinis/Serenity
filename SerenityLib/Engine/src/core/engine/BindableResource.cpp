#include <core/engine/BindableResource.h>

using namespace std;

namespace Engine{
    namespace epriv{
        struct BindableResourceEmptyFunctor{template<class T> void operator()(T* r) const {}};
    };
};

BindableResource::BindableResource(const ResourceType::Type& type, const string& name) : EngineResource(type, name){
    Engine::epriv::BindableResourceEmptyFunctor empty;
    setCustomBindFunctor(empty);
    setCustomUnbindFunctor(empty);
}
BindableResource::~BindableResource(){
}
void BindableResource::bind(){ 
    m_CustomBindFunctor(); 
}
void BindableResource::unbind(){ 
    m_CustomUnbindFunctor(); 
}

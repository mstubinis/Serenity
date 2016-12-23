#include "BindableResource.h"

struct emptyFunctor{
	template<class T> 
	void operator()(T* r) const {
	}
};

BindableResource::BindableResource(){
	emptyFunctor a; emptyFunctor b;
	setCustomBindFunctor(a); setCustomUnbindFunctor(b);
}
BindableResource::~BindableResource(){
}
void BindableResource::bind(){ m_CustomBindFunctor(); }
void BindableResource::unbind(){ m_CustomUnbindFunctor(); }

#pragma once
#ifndef ENGINE_BINDABLE_RESOURCE_H
#define ENGINE_BINDABLE_RESOURCE_H

#include "Engine_ResourceBasic.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>

class BindableResource: public EngineResource{
    private:
        boost::function<void()> m_CustomBindFunctor, m_CustomUnbindFunctor;
    public:
        BindableResource(std::string name = "");
        virtual ~BindableResource();

        virtual void bind();    virtual void unbind();

        template<class T> void setCustomBindFunctor  (T& functor){ m_CustomBindFunctor   = boost::bind<void>(functor,this); }
        template<class T> void setCustomUnbindFunctor(T& functor){ m_CustomUnbindFunctor = boost::bind<void>(functor,this); }
};
#endif
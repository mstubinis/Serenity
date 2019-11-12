#pragma once
#ifndef ENGINE_BINDABLE_RESOURCE_H
#define ENGINE_BINDABLE_RESOURCE_H

#include <core/engine/resources/Engine_ResourceBasic.h>
#include <functional>

class BindableResource: public EngineResource{
    private:
        std::function<void()> m_CustomBindFunctor, m_CustomUnbindFunctor;
    public:
        BindableResource(const ResourceType::Type& type, const std::string& name = "");
        virtual ~BindableResource();

        virtual void bind();
        virtual void unbind();

        template<class T> void setCustomBindFunctor  (const T& functor){ 
            m_CustomBindFunctor   = std::bind<void>(functor, this); 
        }
        template<class T> void setCustomUnbindFunctor(const T& functor){ 
            m_CustomUnbindFunctor = std::bind<void>(functor, this); 
        }
};
#endif
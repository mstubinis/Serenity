#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

#include <memory>
#include "Engine_EventObject.h"

class Entity;
namespace Engine{
	namespace epriv{
		class EventDispatcher final{
			private:
				class impl; std::unique_ptr<impl> m_i;
			public:
				EventDispatcher(const char* name,uint w,uint h);
				~EventDispatcher();

				void _update(const float& dt);
				void _init(const char* name,uint w,uint h);

				void _registerObject(Entity*,EventType::Type);
				void _unregisterObject(Entity*,EventType::Type);
				void _dispatchEvent(EventType::Type,const Event& e);
		};
	};
};
#endif
#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

#include <memory>
#include "Engine_EventObject.h"


class Object;

namespace Engine{
	namespace epriv{
		class EventDispatcher final{
			private:
				class impl; std::unique_ptr<impl> m_i;
			public:
				EventDispatcher(const char* name,uint w,uint h);
				~EventDispatcher();

				void _update(float);
				void _init(const char* name,uint w,uint h);

				void _registerObject(Object*,EventType::Type);
				void _unregisterObject(Object*,EventType::Type);
				void _dispatchEvent(EventType::Type,const Event& e);
		};
	};
};
#endif
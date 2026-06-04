#pragma once
#include "../defs.h"

namespace nani::canvas::events
{
	class Event;
	class EventTarget;
	struct NANI_API EventFilter
	{
	public:
		EventFilter();
		virtual ~EventFilter();

	public:
		virtual bool Filter(EventTarget* target, Event* e);
	};
}

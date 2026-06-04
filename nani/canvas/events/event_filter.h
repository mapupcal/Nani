#pragma once
#include "event_defs.h"

namespace nani::canvas::events
{
	struct NANI_API EventFilter
	{
	public:
		EventFilter();
		virtual ~EventFilter();

	public:
		virtual bool Filter(EventTarget* target, Event* e);
	};
}

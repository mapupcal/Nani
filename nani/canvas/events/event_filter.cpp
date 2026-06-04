#include "env.h"
#include "event_filter.h"
#include "event_target.h"
#include "event.h"
namespace nani::canvas::events
{
	EventFilter::EventFilter()
	{

	}

	EventFilter::~EventFilter()
	{

	}

	bool EventFilter::Filter(EventTarget* target, Event* e)
	{
		return false;
	}
}

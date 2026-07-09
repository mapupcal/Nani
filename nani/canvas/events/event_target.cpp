#include "env.h"
#include "event_target.h"
#include "event_filter.h"
#include "event.h"
#include <ranges>

namespace nani::canvas::events
{
	EventTarget::EventTarget()
	{

	}

	EventTarget::~EventTarget()
	{
		TargetDestroyedEvent e(this);
		FireEvent(&e);
	}

	void EventTarget::RegisterEventFilter(EventFilter* filter)
	{
		auto iter = std::find(m_eventFilters.cbegin(), m_eventFilters.cend(), filter);
		if (iter != m_eventFilters.cend())
		{
			NANI_MESSAGE("filter already registered.");
			return;
		}
		m_eventFilters.push_back(filter);
	}

	void EventTarget::UnRegisterEventFilter(EventFilter* filter)
	{
		auto iter = std::find(m_eventFilters.cbegin(), m_eventFilters.cend(), filter);
		if (iter == m_eventFilters.cend())
		{
			NANI_MESSAGE("filter not registered.");
			return;
		}
		m_eventFilters.erase(iter);
	}

	void EventTarget::FireEvent(Event* e)
	{
		if (FilterEvent(e))
			return;
		OnEvent(e);
	}

	void EventTarget::OnEvent(Event* e)
	{
		//do nothing.
	}

	bool EventTarget::FilterEvent(Event* e)
	{
		auto eventFilters = m_eventFilters; // copy to avoid modification during iteration
		for (EventFilter* filter : eventFilters | std::views::reverse)
		{
			if (filter->Filter(this, e))
				return true;
		}
		return false;
	}
}

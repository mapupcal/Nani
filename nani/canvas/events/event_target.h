#pragma once
#include "../defs.h"
#include <vector>
namespace nani::canvas::events
{
	class EventFilter;
	class Event;

	class NANI_API EventTarget
	{
	public:
		EventTarget();
		virtual ~EventTarget();

	public:
		void RegisterEventFilter(EventFilter* filter);
		void UnRegisterEventFilter(EventFilter* filter);
		void FireEvent(Event* e);

	protected:
		virtual void OnEvent(Event* e);

	private:
		bool FilterEvent(Event* e);

	private:
		std::vector<EventFilter*> eventFilters;
	};
}

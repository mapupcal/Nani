#include "timer_p.h"
#include "../timer.h"
#include "../events/event.h"

namespace nani::canvas::internal
{
	TimerPrivate::TimerPrivate(Timer* timer_)
		: timer(timer_)
	{
	}

	void TimerPrivate::Fire()
	{
		const auto cb = callback;
		if (cb)
			cb();

		events::TimerEvent event(timer);
		timer->FireEvent(&event);
	}

	void TimerPrivate::ArmFromNow()
	{
		nextFire = Clock::now() + std::chrono::milliseconds(intervalMs);
	}

	void TimerPrivate::MarkInactive()
	{
		active = false;
	}

	bool TimerPrivate::DeleteWhenDone() const
	{
		return deleteWhenDone;
	}
}

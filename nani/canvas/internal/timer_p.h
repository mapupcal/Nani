#pragma once

#include "basic/basic_defs.h"

#include <chrono>

namespace nani::canvas
{
	class Timer;
}

namespace nani::canvas::internal
{
	class TimerPrivate
	{
	public:
		using Clock = std::chrono::steady_clock;

		explicit TimerPrivate(Timer* timer_);
		~TimerPrivate() = default;

	public:
		void Fire();
		void ArmFromNow();
		void MarkInactive();
		bool DeleteWhenDone() const;

	public:
		Timer* timer = nullptr;
		std::function<void()> callback;
		basic::dword intervalMs = 0;
		bool singleShot = false;
		bool active = false;
		bool deleteWhenDone = false;
		Clock::time_point nextFire{};
	};
}

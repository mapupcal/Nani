#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "defs.h"
#include "canvas/timer.h"
#include "canvas/window.h"
#include "canvas/basic/pointf.h"
#include "canvas/basic/sizef.h"
#include "canvas/events/event.h"
#include "canvas/events/event_filter.h"

using namespace nani::canvas;
using namespace nani::canvas::basic;
using namespace nani::canvas::events;

namespace
{
	class TimerEventCounter : public EventFilter
	{
	public:
		bool Filter(EventTarget* target, Event* e) override
		{
			(void)target;
			if (e && e->type == Type::Timer)
				++count;
			return false;
		}

		int count = 0;
	};

	class TimerEventProbe : public EventFilter
	{
	public:
		bool Filter(EventTarget* target, Event* e) override
		{
			(void)target;
			if (e && e->type == Type::Timer)
			{
				auto* te = static_cast<TimerEvent*>(e);
				lastTimer = te->timer;
				++count;
			}
			return false;
		}

		Timer* lastTimer = nullptr;
		int count = 0;
	};

	void PumpUntil(
		Env& env,
		const std::function<bool()>& done,
		int maxIterations = 200,
		int sleepMs = 1)
	{
		for (int i = 0; i < maxIterations && !done(); ++i)
		{
			env.ProcessEvents();
			if (!done())
				std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
		}
	}
}

class TimerTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		env_ = new Env(0, nullptr);
	}

	void TearDown() override
	{
		delete env_;
		env_ = nullptr;
	}

	Env* env_ = nullptr;
};

TEST_F(TimerTest, DefaultIsInactive)
{
	Timer timer;
	EXPECT_FALSE(timer.IsActive());
	EXPECT_EQ(timer.Interval(), 0u);
	EXPECT_FALSE(timer.IsSingleShot());
}

TEST_F(TimerTest, IntervalAndSingleShotAccessors)
{
	Timer timer;
	timer.SetInterval(42);
	EXPECT_EQ(timer.Interval(), 42u);

	timer.SetSingleShot(true);
	EXPECT_TRUE(timer.IsSingleShot());
	timer.SetSingleShot(false);
	EXPECT_FALSE(timer.IsSingleShot());

	timer.Start(7);
	EXPECT_EQ(timer.Interval(), 7u);
	EXPECT_TRUE(timer.IsActive());
	timer.Stop();
}

TEST_F(TimerTest, StartStopToggleActive)
{
	Timer timer;
	timer.SetInterval(1000);
	timer.Start();
	EXPECT_TRUE(timer.IsActive());
	timer.Stop();
	EXPECT_FALSE(timer.IsActive());
}

TEST_F(TimerTest, CallbackFiresOnZeroInterval)
{
	int hits = 0;
	Timer timer;
	timer.SetCallback([&] { ++hits; });
	timer.Start(0);

	env_->ProcessEvents();
	EXPECT_GE(hits, 1);
	EXPECT_TRUE(timer.IsActive());

	timer.Stop();
}

TEST_F(TimerTest, SingleShotFiresOnce)
{
	int hits = 0;
	Timer timer;
	timer.SetSingleShot(true);
	timer.SetCallback([&] { ++hits; });
	timer.Start(5);

	PumpUntil(*env_, [&] { return hits >= 1; });
	EXPECT_EQ(hits, 1);
	EXPECT_FALSE(timer.IsActive());

	env_->ProcessEvents();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	env_->ProcessEvents();
	EXPECT_EQ(hits, 1);
}

TEST_F(TimerTest, RepeatingTimerCanBeStopped)
{
	int hits = 0;
	Timer timer;
	timer.SetCallback([&] { ++hits; });
	timer.Start(5);

	PumpUntil(*env_, [&] { return hits >= 2; });
	EXPECT_GE(hits, 2);
	timer.Stop();

	const int frozen = hits;
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	env_->ProcessEvents();
	EXPECT_EQ(hits, frozen);
}

TEST_F(TimerTest, StaticSingleShotInvokesCallback)
{
	int hits = 0;
	Timer::SingleShot(5, [&] { ++hits; });

	PumpUntil(*env_, [&] { return hits >= 1; });
	EXPECT_EQ(hits, 1);
}

TEST_F(TimerTest, FiresTimerEventWithoutCallback)
{
	Timer timer;
	TimerEventCounter counter;
	timer.RegisterEventFilter(&counter);
	timer.Start(0);

	env_->ProcessEvents();
	EXPECT_GE(counter.count, 1);
	timer.Stop();
	timer.UnRegisterEventFilter(&counter);
}

TEST_F(TimerTest, CallbackAndTimerEventFireTogether)
{
	int hits = 0;
	Timer timer;
	TimerEventProbe probe;
	timer.RegisterEventFilter(&probe);
	timer.SetCallback([&] { ++hits; });
	timer.Start(0);

	env_->ProcessEvents();
	EXPECT_GE(hits, 1);
	EXPECT_GE(probe.count, 1);
	EXPECT_EQ(probe.lastTimer, &timer);

	timer.Stop();
	timer.UnRegisterEventFilter(&probe);
}

TEST_F(TimerTest, SetIntervalWhileActiveRearms)
{
	int hits = 0;
	Timer timer;
	timer.SetCallback([&] { ++hits; });
	timer.Start(10000);
	EXPECT_TRUE(timer.IsActive());

	timer.SetInterval(0);
	EXPECT_EQ(timer.Interval(), 0u);

	env_->ProcessEvents();
	EXPECT_GE(hits, 1);
	timer.Stop();
}

TEST_F(TimerTest, RestartWhileActive)
{
	int hits = 0;
	Timer timer;
	timer.SetCallback([&] { ++hits; });
	timer.Start(10000);
	EXPECT_TRUE(timer.IsActive());

	timer.Start(0);
	EXPECT_TRUE(timer.IsActive());
	EXPECT_EQ(timer.Interval(), 0u);

	env_->ProcessEvents();
	EXPECT_GE(hits, 1);
	timer.Stop();
}

TEST_F(TimerTest, StopInsideCallback)
{
	int hits = 0;
	Timer timer;
	timer.SetCallback([&]
	{
		++hits;
		timer.Stop();
	});
	timer.Start(0);

	env_->ProcessEvents();
	EXPECT_EQ(hits, 1);
	EXPECT_FALSE(timer.IsActive());

	env_->ProcessEvents();
	EXPECT_EQ(hits, 1);
}

TEST_F(TimerTest, DestructorUnregistersActiveTimer)
{
	int hits = 0;
	{
		Timer timer;
		timer.SetCallback([&] { ++hits; });
		timer.Start(0);
		EXPECT_TRUE(timer.IsActive());
	}

	env_->ProcessEvents();
	EXPECT_EQ(hits, 0);
}

TEST_F(TimerTest, MultipleTimersFireIndependently)
{
	int hitsA = 0;
	int hitsB = 0;
	Timer timerA;
	Timer timerB;
	timerA.SetCallback([&] { ++hitsA; });
	timerB.SetCallback([&] { ++hitsB; });
	timerA.Start(0);
	timerB.Start(0);

	env_->ProcessEvents();
	EXPECT_GE(hitsA, 1);
	EXPECT_GE(hitsB, 1);

	timerA.Stop();
	const int frozenA = hitsA;
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	env_->ProcessEvents();
	EXPECT_EQ(hitsA, frozenA);
	EXPECT_GE(hitsB, 2);

	timerB.Stop();
}

TEST_F(TimerTest, EnvTerminateCleansPendingSingleShot)
{
	Timer::SingleShot(60000, [] {});
	delete env_;
	env_ = nullptr;
}

TEST_F(TimerTest, WaitForQuitWakesOnTimer)
{
	auto* window = new Window(PointF(), SizeF(100, 100));
	window->Show();

	Timer::SingleShot(20, [window]
	{
		window->Close();
	});

	EXPECT_EQ(env_->WaitForQuit(), 0);
	delete window;
}

#include "env_p.h"
#include "timer_p.h"
#include "window_p.h"
#include "../timer.h"

namespace nani::canvas::internal
{
	EnvPrivate* EnvPrivate::Instance()
	{
		static EnvPrivate ins;
		return &ins;
	}

	void EnvPrivate::Initialize()
	{
		glfwInit();
	}

	void EnvPrivate::Terminate()
	{
		const std::vector<Timer*> timers = m_timers;
		m_timers.clear();
		for (Timer* timer : timers)
		{
			const bool deleteWhenDone = timer->m_pImpl->DeleteWhenDone();
			timer->m_pImpl->MarkInactive();
			if (deleteWhenDone)
				delete timer;
		}

		glfwTerminate();
	}

	void EnvPrivate::RegisterWindow(GLFWwindow* window)
	{
		auto iter = std::find(m_lstWindows.cbegin(), m_lstWindows.cend(), window);
		if (iter != m_lstWindows.cend())
			return;
		m_lstWindows.push_back(window);
	}

	void EnvPrivate::UnRegisterWindow(GLFWwindow* window)
	{
		auto iter = std::find(m_lstWindows.cbegin(), m_lstWindows.cend(), window);
		if (iter == m_lstWindows.cend())
			return;
		m_lstWindows.erase(iter);
	}

	void EnvPrivate::RegisterActiveTimer(Timer* timer)
	{
		if (!timer)
			return;
		auto iter = std::find(m_timers.cbegin(), m_timers.cend(), timer);
		if (iter != m_timers.cend())
			return;
		m_timers.push_back(timer);
	}

	void EnvPrivate::UnregisterActiveTimer(Timer* timer)
	{
		auto iter = std::find(m_timers.cbegin(), m_timers.cend(), timer);
		if (iter == m_timers.cend())
			return;
		m_timers.erase(iter);
	}

	int EnvPrivate::WaitForQuit()
	{
		while (!m_lstWindows.empty())
		{
			const double timeout = SecondsUntilNextTimer();
			if (timeout < 0.0)
				glfwWaitEvents();
			else
				glfwWaitEventsTimeout(timeout);

			Tick();
			glfwPollEvents();
		}

		return 0;
	}

	void EnvPrivate::ProcessEvents()
	{
		glfwPollEvents();
		Tick();
	}

	EnvPrivate::EnvPrivate()
	{
	}

	EnvPrivate::~EnvPrivate()
	{
	}

	void EnvPrivate::Tick()
	{
		ProcessTimers();

		for (auto glfwWindow : m_lstWindows)
		{
			WindowPrivate* pImpl =
				reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(glfwWindow));
			if (pImpl)
				pImpl->onTick();
		}
	}

	void EnvPrivate::ProcessTimers()
	{
		if (m_timers.empty())
			return;

		const auto now = TimerPrivate::Clock::now();
		std::vector<Timer*> due;
		due.reserve(m_timers.size());
		for (Timer* timer : m_timers)
		{
			if (timer->IsActive() && timer->m_pImpl->nextFire <= now)
				due.push_back(timer);
		}

		for (Timer* timer : due)
		{
			if (!timer->IsActive())
				continue;

			const bool singleShot = timer->IsSingleShot();
			const bool deleteWhenDone = timer->m_pImpl->DeleteWhenDone();

			timer->m_pImpl->Fire();

			if (!timer->IsActive())
			{
				if (deleteWhenDone)
					delete timer;
				continue;
			}

			if (singleShot)
			{
				timer->Stop();
				if (deleteWhenDone)
					delete timer;
			}
			else
			{
				timer->m_pImpl->ArmFromNow();
			}
		}
	}

	double EnvPrivate::SecondsUntilNextTimer() const
	{
		if (m_timers.empty())
			return -1.0;

		const auto now = TimerPrivate::Clock::now();
		auto earliest = TimerPrivate::Clock::time_point::max();
		bool found = false;
		for (Timer* timer : m_timers)
		{
			if (!timer->IsActive())
				continue;
			earliest = std::min(earliest, timer->m_pImpl->nextFire);
			found = true;
		}

		if (!found)
			return -1.0;
		if (earliest <= now)
			return 0.0;

		return std::chrono::duration<double>(earliest - now).count();
	}
}

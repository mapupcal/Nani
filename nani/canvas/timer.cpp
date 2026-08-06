#include "timer.h"
#include "internal/env_p.h"
#include "internal/timer_p.h"

namespace nani::canvas
{
	Timer::Timer()
		: m_pImpl(new internal::TimerPrivate(this))
	{
	}

	Timer::~Timer()
	{
		Stop();
		delete m_pImpl;
		m_pImpl = nullptr;
	}

	void Timer::SetCallback(Callback cb)
	{
		m_pImpl->callback = std::move(cb);
	}

	void Timer::SetInterval(basic::dword intervalMs)
	{
		m_pImpl->intervalMs = intervalMs;
		if (m_pImpl->active)
			m_pImpl->ArmFromNow();
	}

	basic::dword Timer::Interval() const
	{
		return m_pImpl->intervalMs;
	}

	void Timer::SetSingleShot(bool singleShot)
	{
		m_pImpl->singleShot = singleShot;
	}

	bool Timer::IsSingleShot() const
	{
		return m_pImpl->singleShot;
	}

	void Timer::Start()
	{
		if (m_pImpl->active)
			Stop();

		m_pImpl->active = true;
		m_pImpl->ArmFromNow();
		internal::EnvPrivate::Instance()->RegisterActiveTimer(this);
	}

	void Timer::Start(basic::dword intervalMs)
	{
		m_pImpl->intervalMs = intervalMs;
		Start();
	}

	void Timer::Stop()
	{
		if (!m_pImpl || !m_pImpl->active)
			return;

		m_pImpl->active = false;
		internal::EnvPrivate::Instance()->UnregisterActiveTimer(this);
	}

	bool Timer::IsActive() const
	{
		return m_pImpl && m_pImpl->active;
	}

	void Timer::SingleShot(basic::dword intervalMs, Callback cb)
	{
		auto* timer = new Timer();
		timer->m_pImpl->deleteWhenDone = true;
		timer->SetSingleShot(true);
		timer->SetCallback(std::move(cb));
		timer->Start(intervalMs);
	}
}

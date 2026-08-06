#pragma once

#include "defs.h"
#include "events/event_target.h"
#include "basic/basic_defs.h"

namespace nani::canvas::internal
{
	class EnvPrivate;
}

namespace nani::canvas
{
	class NANI_CANVAS_API Timer : public events::EventTarget
	{
		friend class internal::EnvPrivate;

	public:
		using Callback = std::function<void()>;

	public:
		Timer();
		~Timer() override;

	public:
		void SetCallback(Callback cb);

		void SetInterval(basic::dword intervalMs);
		basic::dword Interval() const;

		void SetSingleShot(bool singleShot);
		bool IsSingleShot() const;

		void Start();
		void Start(basic::dword intervalMs);
		void Stop();
		bool IsActive() const;

		static void SingleShot(basic::dword intervalMs, Callback cb);

	private:
		internal::TimerPrivate* m_pImpl = nullptr;
	};
}

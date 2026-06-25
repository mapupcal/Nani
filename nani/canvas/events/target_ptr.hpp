#pragma once
#include "event_defs.h"
#include "event_filter.h"
#include "event_target.h"
#include "event.h"
#include <concepts>
namespace nani::canvas::events
{
	template<typename T>
	concept TargetType = std::derived_from<T, EventTarget>;

	template<TargetType Target>
	struct target_ptr
	{
	private:
		class SafeTarget : public EventFilter
		{
		public:
			SafeTarget() = default;

			explicit SafeTarget(Target* target)
				: EventFilter()
				, m_pTarget(target)
			{
				if (m_pTarget)
					m_pTarget->RegisterEventFilter(this);
			}

			~SafeTarget()
			{
				if (m_pTarget)
					m_pTarget->UnRegisterEventFilter(this);
			}

			SafeTarget(const SafeTarget&) = delete;
			SafeTarget& operator=(const SafeTarget&) = delete;

			SafeTarget(SafeTarget&& other) noexcept
				: EventFilter()
				, m_pTarget(other.m_pTarget)
			{
				if (m_pTarget)
				{
					other.m_pTarget->UnRegisterEventFilter(&other);
					m_pTarget->RegisterEventFilter(this);
				}
				other.m_pTarget = nullptr;
			}

			SafeTarget& operator=(SafeTarget&& other) noexcept
			{
				if (this != &other)
				{
					if (m_pTarget)
						m_pTarget->UnRegisterEventFilter(this);

					m_pTarget = other.m_pTarget;

					if (m_pTarget)
					{
						other.m_pTarget->UnRegisterEventFilter(&other);
						m_pTarget->RegisterEventFilter(this);
					}
					other.m_pTarget = nullptr;
				}
				return *this;
			}

			Target* Get() const
			{
				return m_pTarget;
			}

			bool Filter(EventTarget* target, Event* e) override
			{
				if (m_pTarget && e->type == Type::TargetDestroyed)
				{
					m_pTarget->UnRegisterEventFilter(this);
					m_pTarget = nullptr;
				}
				return false;
			}

		private:
			Target* m_pTarget;
		};

	public:
		target_ptr() = default;
		~target_ptr() = default;

		explicit target_ptr(Target* target)
			: m_safeTarget(target)
		{

		}

		target_ptr(const target_ptr& other)
			: m_safeTarget(other.m_safeTarget.Get())
		{

		}

		target_ptr(target_ptr&& other) noexcept
			: m_safeTarget(std::move(other.m_safeTarget))
		{

		}

		target_ptr& operator=(const target_ptr& other)
		{
			if (this != &other)
			{
				m_safeTarget = SafeTarget(other.m_safeTarget.Get());
			}
			return *this;
		}

		target_ptr& operator=(target_ptr&& other) noexcept
		{
			if (this != &other)
			{
				m_safeTarget = std::move(other.m_safeTarget);
			}
			return *this;
		}

		target_ptr& operator=(std::nullptr_t)
		{
			m_safeTarget = SafeTarget(nullptr);
			return *this;
		}

		target_ptr& operator=(Target* target)
		{
			m_safeTarget = SafeTarget(target);
			return *this;
		}

		Target& operator*() const
		{
			auto ptr = GetRawPtr();
			assert(ptr != nullptr);
			return *ptr;
		}

		Target* operator->() const
		{
			return GetRawPtr();
		}

		operator Target* () const
		{
			return GetRawPtr();
		}

		explicit operator bool() const
		{
			return GetRawPtr() != nullptr;
		}

		bool operator!() const
		{
			return GetRawPtr() == nullptr;
		}

		bool operator==(const target_ptr& other) const
		{
			return GetRawPtr() == other.GetRawPtr();
		}

		bool operator!=(const target_ptr& other) const
		{
			return GetRawPtr() != other.GetRawPtr();
		}

	private:
		Target* GetRawPtr() const
		{
			return m_safeTarget.Get();
		}

	private:
		SafeTarget m_safeTarget;
	};

	template<TargetType Target>
	bool operator==(Target* lhs, const target_ptr<Target>& rhs)
	{
		return rhs == lhs;
	}

	template<TargetType Target>
	bool operator!=(Target* lhs, const target_ptr<Target>& rhs)
	{
		return rhs != lhs;
	}

	template<TargetType Target>
	bool operator==(std::nullptr_t, const target_ptr<Target>& rhs)
	{
		return rhs == nullptr;
	}

	template<TargetType Target>
	bool operator!=(std::nullptr_t, const target_ptr<Target>& rhs)
	{
		return rhs != nullptr;
	}

	template<TargetType Target>
	bool operator==(const target_ptr<Target>& lhs, Target* rhs)
	{
		return rhs == lhs;
	}

	template<TargetType Target>
	bool operator!=(const target_ptr<Target>& lhs, Target* rhs)
	{
		return rhs == lhs;
	}

	template<TargetType Target>
	bool operator==(const target_ptr<Target>& rhs, std::nullptr_t)
	{
		return rhs == nullptr;
	}

	template<TargetType Target>
	bool operator!=(const target_ptr<Target>& rhs, std::nullptr_t)
	{
		return rhs != nullptr;
	}
}

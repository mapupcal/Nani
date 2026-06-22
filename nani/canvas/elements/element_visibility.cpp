#include "element_visibility.h"
#include "element.h"
using namespace nani::canvas::events;

namespace nani::canvas::elements
{
	ElementVisibility::ElementVisibility(Element* owner)
		: m_pOwner(owner)
	{

	}

	ElementVisibility::~ElementVisibility()
	{

	}

	bool ElementVisibility::IsVisible() const
	{
		return m_flags == Visible;
	}

	void ElementVisibility::SetHidden(bool bHidden)
	{
		SetFlag(Hidden, bHidden);
	}

	bool ElementVisibility::IsHidden() const
	{
		return !!(m_flags & Hidden);
	}

	void ElementVisibility::SetCollapsed(bool bCollapsed)
	{
		SetFlag(Collapsed, bCollapsed);
	}

	bool ElementVisibility::IsCollapsed() const
	{
		return !!(m_flags & Collapsed);
	}

	void ElementVisibility::SetFlag(Flags flag, bool bSet)
	{
		if (!!(m_flags & flag) == bSet)
			return;

		if (bSet)
			m_flags = static_cast<Flags>(basic::uint32(m_flags) | flag);
		else
			m_flags = static_cast<Flags>(basic::uint32(m_flags) & ~flag);

		ElementVisibilityChangedEvent event(m_pOwner);
		m_pOwner->FireEvent(&event);
	}
}
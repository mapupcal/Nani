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
		if (IsHidden() == bHidden)
			return;
		m_flags = static_cast<Flags>(m_flags | Hidden);

		ElementVisibilityChangedEvent event(m_pOwner);
		m_pOwner->FireEvent(&event);
	}

	bool ElementVisibility::IsHidden() const
	{
		return !!(m_flags & Hidden);
	}

	void ElementVisibility::SetCollapsed(bool bCollapsed)
	{
		if (IsCollapsed() == bCollapsed)
			return;
		m_flags = static_cast<Flags>(m_flags | Collapsed);

		ElementVisibilityChangedEvent event(m_pOwner);
		m_pOwner->FireEvent(&event);
	}

	bool ElementVisibility::IsCollapsed() const
	{
		return !!(m_flags & Collapsed);
	}
}
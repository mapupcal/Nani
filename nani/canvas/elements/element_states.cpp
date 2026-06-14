#include "element_states.h"
#include "element.h"

using namespace nani::canvas::events;

namespace nani::canvas::elements
{
	ElementStates::ElementStates(Element* owner)
		: m_pOwner(owner)
	{

	}

	ElementStates::~ElementStates()
	{

	}

	void ElementStates::SetEnabled(bool bEnable)
	{
		SetFlag(Enabled, bEnable);
	}

	bool ElementStates::IsEnabled() const
	{
		return IsSetFlag(Enabled);
	}

	void ElementStates::SetHovered(bool bHover)
	{
		SetFlag(Hovered, bHover);
	}

	bool ElementStates::IsHovered() const
	{
		return IsSetFlag(Hovered);
	}

	void ElementStates::SetPressed(bool bPressed)
	{
		SetFlag(Pressed, bPressed);
	}

	bool ElementStates::IsPressed() const
	{
		return IsSetFlag(Pressed);
	}

	void ElementStates::SetFocused(bool bFocus)
	{
		SetFlag(Focused, bFocus);
	}

	bool ElementStates::IsFocused() const
	{
		return IsSetFlag(Focused);
	}

	void ElementStates::SetSelected(bool bSelected)
	{
		SetFlag(Selected, bSelected);
	}

	bool ElementStates::IsSelected() const
	{
		return IsSetFlag(Selected);
	}

	bool ElementStates::IsSetFlag(Flags flag) const
	{
		return m_flags & flag;
	}

	void ElementStates::SetFlag(Flags flag, bool bSet)
	{
		if ((!!(m_flags & flag)) == bSet)
			return;

		if (bSet)
			m_flags = static_cast<Flags>(basic::uint32(m_flags) | flag);
		else
			m_flags = static_cast<Flags>(basic::uint32(m_flags) & ~(flag));

		ElementStatesChangedEvent event(m_pOwner);
		m_pOwner->FireEvent(&event);
	}
}
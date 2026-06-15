#pragma once
#include "elements_defs.h"
namespace nani::canvas::elements
{
	class NANI_CANVAS_API ElementVisibility
	{
	public:
		enum Flags : basic::uint32
		{
			Visible			= 0,
			Hidden			= 1 << 0,
			Collapsed		= 1 << 1,
		};

	public:
		ElementVisibility(Element* owner);
		~ElementVisibility();

	public:
		bool IsVisible() const;

		void SetHidden(bool bHidden);
		bool IsHidden() const;

		void SetCollapsed(bool bCollapsed);
		bool IsCollapsed() const;

	private:
		Element* m_pOwner = nullptr;
		Flags m_flags = Visible;
	};
}
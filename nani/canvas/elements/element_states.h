#pragma once
#include "elements_defs.h"
namespace nani::canvas::elements
{
	class NANI_CANVAS_API ElementStates
	{
	public:
		enum Flags : basic::uint32
		{
			Enabled				= 1 << 0,
			Hovered				= 1 << 1,
			Pressed				= 1 << 2,
			Selected			= 1 << 3,
			Focused				= 1 << 4
		};

	public:
		ElementStates(Element* owner);
		~ElementStates();

	public:
		void SetEnabled(bool bEnable);
		bool IsEnabled() const;

		void SetHovered(bool bHover);
		bool IsHovered() const;

		void SetPressed(bool bPressed);
		bool IsPressed() const;

		void SetFocused(bool bFocus);
		bool IsFocused() const;

		void SetSelected(bool bSelected);
		bool IsSelected() const;

		const char8_t* GetStateProps() const;

	private:
		bool IsSetFlag(Flags flag) const;
		void SetFlag(Flags flag, bool bSet);

	private:
		Element* m_pOwner = nullptr;
		Flags m_flags = Enabled;
	};
}
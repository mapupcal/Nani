#pragma once
#include "elements_defs.h"
namespace nani::canvas::elements
{
	class NANI_CANVAS_API ComputedStyle
	{
	public:
		ComputedStyle();
		~ComputedStyle();
	};

	class NANI_CANVAS_API Styles
	{
	public:
		Styles();
		~Styles();

	public:
		ComputedStyle* Compute(Element* element);

	private:
		std::map<std::u8string, ComputedStyle*> m_mapComputedStyles;
	};
}
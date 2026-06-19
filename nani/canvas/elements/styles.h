#pragma once
#include "elements_defs.h"
namespace nani::canvas::elements
{
	class NANI_CANVAS_API ComputedStyle
	{
	public:
		struct DiffResult
		{
			bool layoutChanged = false;
			bool visualChanged = false;
		};
	public:
		ComputedStyle();
		~ComputedStyle();

	public:
		const DiffResult Diff(const ComputedStyle* other) const;
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
#pragma once
#include "elements_defs.h"
#include <yoga/style/Style.h>
namespace nani::canvas::elements
{
	class NANI_CANVAS_API ComputedStyle
	{
	public:
		ComputedStyle();
		~ComputedStyle();

	public:
		struct DiffResult
		{
			bool layoutChanged = false;
			bool visualChanged = false;
		};
		const DiffResult Diff(const ComputedStyle* other) const;

	public:
		struct LayoutProperties
		{
			facebook::yoga::Style style;
		};

		struct VisualProperties
		{
			struct BorderRadius
			{
				basic::scalar topLeft = 0.0f;
				basic::scalar topRight = 0.0f;
				basic::scalar bottomLeft = 0.0f;
				basic::scalar bottomRight = 0.0f;
			};

			struct Shadow
			{
				basic::Color color;
				basic::scalar offsetX = 0.0f;
				basic::scalar offsetY = 0.0f;
				basic::scalar blur = 0.0f;
				basic::scalar spread = 0.0f;
			};

			basic::Color color;
			basic::Color backgroundColor;
			basic::Color borderColor;
			basic::scalar opacity = 1.0f;
			BorderRadius radius;
			Shadow shadow;
		};
	public:
		LayoutProperties layoutProps;
		VisualProperties visualProps;
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
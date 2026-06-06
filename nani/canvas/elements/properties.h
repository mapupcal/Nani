#pragma once
#include "elements_defs.h"
namespace nani::canvas::elements
{
	struct BoxPrimitive
	{
		basic::int32 left = 0;
		basic::int32 top = 0;
		basic::int32 right = 0;
		basic::int32 bottom = 0;
	};

	struct CornerPrimitive
	{
		basic::int32 topLeft = 0;
		basic::int32 bottomLeft = 0;
		basic::int32 topRight = 0;
		basic::int32 bottomRight = 0;
	};

	struct LayoutProperties
	{
		basic::int32 width = 0;
		basic::int32 height = 0;
		basic::int32 minWidth = 0;
		basic::int32 minHeight = 0;
		basic::int32 maxWidth = -1;
		basic::int32 maxHeight = -1;
		basic::single aspectRatio = 0.0f;

		BoxPrimitive padding;
		BoxPrimitive border;
		BoxPrimitive margin;
		BoxPrimitive position{ .left = -1, .top = -1, .right = -1, .bottom = -1 };
	};

	struct StyleProperties
	{
		basic::Color color = basic::Colors::Black;
		basic::Color backgroundColor = basic::Colors::Transparent;
		basic::Color borderColor = basic::Colors::Transparent;
		CornerPrimitive radius;
	};
}
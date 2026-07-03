#include "computed_style.h"

namespace nani::canvas::internal
{
	ComputedStyle::ComputedStyle()
	{

	}

	ComputedStyle::~ComputedStyle()
	{

	}

	bool operator==(const ComputedStyle::LayoutProperties& lhs, const ComputedStyle::LayoutProperties& rhs)
	{
		return lhs.style == rhs.style;
	}

	bool operator==(const ComputedStyle::VisualProperties::BorderRadius& lhs, const ComputedStyle::VisualProperties::BorderRadius& rhs)
	{
		return basic::IsScalarEqual(lhs.topLeft, rhs.topLeft) &&
			basic::IsScalarEqual(lhs.topRight, rhs.topRight) &&
			basic::IsScalarEqual(lhs.bottomLeft, rhs.bottomLeft) &&
			basic::IsScalarEqual(lhs.bottomRight, rhs.bottomRight);
	}

	bool operator==(const ComputedStyle::VisualProperties::Shadow& lhs, const ComputedStyle::VisualProperties::Shadow& rhs)
	{
		return (lhs.color == rhs.color) && 
			basic::IsScalarEqual(lhs.offsetX, rhs.offsetX) &&
			basic::IsScalarEqual(lhs.offsetY, rhs.offsetY) &&
			basic::IsScalarEqual(lhs.blur, rhs.blur) &&
			basic::IsScalarEqual(lhs.spread, rhs.spread);
	}

	bool operator==(const ComputedStyle::VisualProperties::TransformOrigin& lhs, const ComputedStyle::VisualProperties::TransformOrigin& rhs)
	{
		return (lhs.x == rhs.x) && (lhs.y == rhs.y);
	}

	bool operator==(const ComputedStyle::VisualProperties& lhs, const ComputedStyle::VisualProperties& rhs)
	{
		return (lhs.color == rhs.color) && 
			(lhs.borderColor == rhs.borderColor) &&
			(lhs.backgroundColor == rhs.backgroundColor) &&
			basic::IsScalarEqual(lhs.opacity, rhs.opacity) &&
			lhs.transform == rhs.transform &&
			lhs.transformOrigin == rhs.transformOrigin &&
			(lhs.radius == rhs.radius) &&
			(lhs.shadow == rhs.shadow);
	}

	const ComputedStyle::DiffResult ComputedStyle::Diff(const ComputedStyle* other) const
	{
		if (!other)
			return { .layoutChanged = true, .visualChanged = true };
		if(this == other)
			return { .layoutChanged = false, .visualChanged = false };
		return { .layoutChanged = !(layoutProps == other->layoutProps), .visualChanged = !(visualProps == other->visualProps) };
	}
}
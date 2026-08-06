#pragma once

#include "defs.h"
#include "yoga_defs.h"

#include "../basic/marginsf.h"
#include "../basic/rectf.h"

namespace nani::canvas::internal::yoga_utils
{
	void SetNodeStyle(YGNodeRef node, const facebook::yoga::Style& style);
	const basic::MarginsF GetNodeMargins(YGNodeRef node);
	const basic::MarginsF GetNodeBorders(YGNodeRef node);
	const basic::MarginsF GetNodePaddings(YGNodeRef node);
	const basic::RectF GetNodeBorderRect(YGNodeRef node);
	const basic::RectF GetNodeContentRect(YGNodeRef node);
	// Local paint-space content box: border box moved to (0,0), minus borders/paddings.
	const basic::RectF LocalContentRect(const basic::RectF& layoutRect, YGNodeRef node);
	float ResolveMeasuredSize(float desired, YGMeasureMode mode, float constraint);
	const basic::PointF GetPointInRect(const basic::RectF& rect, const facebook::yoga::StyleLength& x, const facebook::yoga::StyleLength& y);
}

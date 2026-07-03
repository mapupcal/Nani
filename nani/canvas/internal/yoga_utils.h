#pragma once
#include "defs.h"
#include <yoga/Yoga.h>
#include <yoga/style/Style.h>
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
	const basic::PointF GetPointInRect(const basic::RectF& rect, const facebook::yoga::StyleLength& x, const facebook::yoga::StyleLength& y);
}

#include "yoga_utils.h"
#include <yoga/node/Node.h>

using namespace nani::canvas::basic;

namespace nani::canvas::internal::yoga_utils
{
	using namespace facebook::yoga;
	void SetNodeStyle(YGNodeRef nodeRef, const Style& style)
	{
		Node* node = static_cast<Node*>(nodeRef);
		node->setStyle(style);
	}

	const basic::MarginsF GetNodeMargins(YGNodeRef node)
	{
		MarginsF margins;
		margins.left = YGNodeLayoutGetMargin(node, YGEdgeLeft);
		margins.top = YGNodeLayoutGetMargin(node, YGEdgeTop);
		margins.right = YGNodeLayoutGetMargin(node, YGEdgeRight);
		margins.bottom = YGNodeLayoutGetMargin(node, YGEdgeBottom);
		return margins;
	}

	const basic::MarginsF GetNodeBorders(YGNodeRef node)
	{
		MarginsF margins;
		margins.left = YGNodeLayoutGetBorder(node, YGEdgeLeft);
		margins.top = YGNodeLayoutGetBorder(node, YGEdgeTop);
		margins.right = YGNodeLayoutGetBorder(node, YGEdgeRight);
		margins.bottom = YGNodeLayoutGetBorder(node, YGEdgeBottom);
		return margins;
	}

	const basic::MarginsF GetNodePaddings(YGNodeRef node)
	{
		MarginsF margins;
		margins.left = YGNodeLayoutGetPadding(node, YGEdgeLeft);
		margins.top = YGNodeLayoutGetPadding(node, YGEdgeTop);
		margins.right = YGNodeLayoutGetPadding(node, YGEdgeRight);
		margins.bottom = YGNodeLayoutGetPadding(node, YGEdgeBottom);
		return margins;
	}

	const basic::RectF GetNodeBorderRect(YGNodeRef node)
	{
		RectF rect;
		rect.left = YGNodeLayoutGetLeft(node);
		rect.top = YGNodeLayoutGetTop(node);
		rect.right = rect.left + YGNodeLayoutGetWidth(node);
		rect.bottom = rect.top + YGNodeLayoutGetHeight(node);
		return rect;
	}

	const basic::RectF GetNodeContentRect(YGNodeRef node)
	{
		return GetNodeBorderRect(node) - (GetNodeBorders(node) + GetNodePaddings(node));
	}

	const basic::PointF GetPointInRect(const basic::RectF& rect, const facebook::yoga::StyleLength& x, const facebook::yoga::StyleLength& y)
	{
		PointF point = rect.TopLeft();

		if (x.unit() == Unit::Percent)
			point.x += rect.Width() * x.value().unwrap() * 0.01f;
		else if (x.unit() == Unit::Point)
			point.x += x.value().unwrap();

		if (y.unit() == Unit::Percent)
			point.y += rect.Height() * y.value().unwrap() * 0.01f;
		else if (y.unit() == Unit::Point)
			point.y += y.value().unwrap();
		
		return point;
	}
}


#include "env_p.h"
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
		rect.right = YGNodeLayoutGetRight(node);
		rect.bottom = YGNodeLayoutGetBottom(node);
		return rect;
	}

	const basic::RectF GetNodeContentRect(YGNodeRef node)
	{
		return GetNodeBorderRect(node) - (GetNodeBorders(node) + GetNodePaddings(node));
	}
}


#include "scroll_area_visual.h"
#include "../elements/scroll_area_element.h"
#include "../events/event.h"

using namespace nani::canvas::basic;
using namespace nani::canvas::elements;
using namespace nani::canvas::events;

namespace nani::canvas::visuals
{
	ScrollAreaVisual::ScrollAreaVisual(visuals::View* view, ScrollAreaElement* element, Visual* parent)
		: Visual(view, element, parent)
	{
	}

	ScrollAreaElement* ScrollAreaVisual::ScrollArea() const
	{
		return static_cast<ScrollAreaElement*>(Element());
	}

	void ScrollAreaVisual::SyncScrollMetrics() const
	{
		auto* scrollArea = ScrollArea();
		if (!scrollArea)
			return;

		scalar contentWidth = 0.0f;
		scalar contentHeight = 0.0f;
		for (const auto& child : Visuals())
		{
			const RectF childRect = child->LayoutRect();
			contentWidth = std::max(contentWidth, childRect.right);
			contentHeight = std::max(contentHeight, childRect.bottom);
		}

		scrollArea->UpdateScrollMetrics(
			SizeF(contentWidth, contentHeight),
			ContentRect().Size());
	}

	PointF ScrollAreaVisual::ContentScrollOffset() const
	{
		SyncScrollMetrics();
		auto* scrollArea = ScrollArea();
		if (!scrollArea)
			return PointF();
		return scrollArea->ScrollOffset();
	}

	bool ScrollAreaVisual::Filter(EventTarget* target, Event* e)
	{
		if (target == ScrollArea() && e->type == Type::ElementScrollChanged)
		{
			Repaint();
			return false;
		}

		return Visual::Filter(target, e);
	}
}

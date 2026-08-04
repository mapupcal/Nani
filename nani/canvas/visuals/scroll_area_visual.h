#pragma once
#include "visuals_defs.h"
#include "visual.h"

namespace nani::canvas::elements
{
	class ScrollAreaElement;
}

namespace nani::canvas::visuals
{
	class NANI_CANVAS_API ScrollAreaVisual : public Visual
	{
	public:
		ScrollAreaVisual(visuals::View* view, elements::ScrollAreaElement* element, Visual* parent);
		~ScrollAreaVisual() override = default;

	public:
		bool Filter(events::EventTarget* target, events::Event* e) override;

	protected:
		basic::PointF ContentScrollOffset() const override;

	private:
		void SyncScrollMetrics() const;
		elements::ScrollAreaElement* ScrollArea() const;
	};
}

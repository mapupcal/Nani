#pragma once
#include "visuals_defs.h"
#include "visual.h"

namespace nani::canvas::elements
{
	class TextElement;
}

namespace nani::canvas::visuals
{
	class NANI_CANVAS_API TextVisual : public Visual
	{
	public:
		TextVisual(visuals::View* view, elements::TextElement* element, Visual* parent);
		~TextVisual() override;

	public:
		void BuildVisuals() override;
		bool HitTestOverride(const basic::PointF& localPos) override;
		void PaintOverride(SkCanvas* canvas, const basic::RectF& dirtyRect) override;
		bool Filter(events::EventTarget* target, events::Event* e) override;

	private:
		struct LayoutCache;
		void InvalidateLayoutCache() const;
		bool EnsurePaintLayout() const;

		mutable std::unique_ptr<LayoutCache> m_layoutCache;
	};
}

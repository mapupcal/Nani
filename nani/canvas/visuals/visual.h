#pragma once
#include "visuals_defs.h"
namespace nani::canvas::visuals
{
	class NANI_CANVAS_API Visual : public events::EventFilter
	{
	public:
		Visual(visuals::View* view, elements::Element* element, Visual* parent);
		virtual ~Visual();

	public:
		Visual* Parent();
		elements::Element* Element() const;
		std::vector<std::shared_ptr<Visual>> Visuals() const;
		visuals::View* View() const;

		void BuildVisuals();
		void Update();
		void Reflow();
		void Repaint();
		void CalculateLayout(const basic::SizeF& size);

		const basic::RectF LayoutRect() const;
		const basic::TransformF Transform() const;
		const basic::PointF TransformOrigin() const;

		bool HitTest(const basic::PointF& localPos, Visual** ppHitVisual, basic::PointF& hitLocalPos);
		virtual bool HitTestOverride(const basic::PointF& localPos);
		void Paint(SkCanvas* canvas, const basic::RectF& dirtyRect);
		virtual void PaintOverride(SkCanvas* canvas, const basic::RectF& dirtyRect);

	public:
		bool Filter(events::EventTarget* target, events::Event* e) override;

	private:
		void BuildComputedStyle();
		void SyncLayoutProperties();
		const basic::PolygonF VisualGeometry();

	private:
		elements::Element* m_pElement = nullptr;
		visuals::View* m_pView = nullptr;
		Visual* m_pParent = nullptr;
		std::vector<std::shared_ptr<Visual>> m_visuals;
		YGNodeRef m_yogaNode = nullptr;
		std::shared_ptr<internal::ComputedStyle> m_spComputedStyle;
	};
}
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

		virtual bool HitTest(const basic::PointF& pos, Visual** ppHitVisual);
		virtual void Paint(SkCanvas* canvas);

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
		std::shared_ptr<elements::ComputedStyle> m_spComputedStyle;
	};
}
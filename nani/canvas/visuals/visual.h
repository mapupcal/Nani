#pragma once
#include "visuals_defs.h"
struct YGNode;
namespace nani::canvas::elements
{
	struct ComputedStyle;
}
namespace nani::canvas::visuals
{
	using YGNodeRef = YGNode*;
	class NANI_CANVAS_API Visual : public events::EventFilter
	{
	public:
		Visual(elements::Element* element, Visual* parent);
		virtual ~Visual();

	public:
		Visual* Parent();
		elements::Element* Element() const;
		std::vector<std::shared_ptr<Visual>> Visuals() const;

		void BuildVisuals();
		void Update();
		void Reflow();
		void Repaint();

		const basic::MarginsF LayoutMarggins() const;
		const basic::MarginsF LayoutBorder() const;
		const basic::MarginsF LayoutPadding() const;
		const basic::RectF LayoutBorderRect() const;
		const basic::RectF LayoutContentRect() const;

		virtual bool HitTest(const basic::PointF& pos, Visual** ppHitVisual);

	private:
		virtual bool Filter(events::EventTarget* target, events::Event* e) override;

	private:
		void BuildComputedStyle();
		void SyncLayoutProperties();

	private:
		elements::Element* m_pElement = nullptr;
		Visual* m_pParent = nullptr;
		std::vector<std::shared_ptr<Visual>> m_visuals;
		YGNodeRef m_yogaNode = nullptr;
		elements::ComputedStyle* m_pComputedStyle = nullptr;
	};
}
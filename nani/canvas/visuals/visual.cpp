#include "visual.h"
#include "view.h"
#include "../elements/element.h"
#include "../elements/element_visibility.h"
#include "../styles.h"
#include "../internal/computed_style.h"
#include "../internal/yoga_utils.h"
#include "../internal/skia_utils.h"
#include <core/SkCanvas.h>

using namespace nani::canvas::elements;
using namespace nani::canvas::events;
using namespace nani::canvas::basic;
using namespace nani::canvas::internal;

namespace nani::canvas::visuals
{
	Visual::Visual(visuals::View* view, elements::Element* element, Visual* parent)
		: m_pView(view)
		, m_pElement(element)
		, m_pParent(parent)
	{
		if (m_pElement)
			m_pElement->RegisterEventFilter(this);
	}

	Visual::~Visual()
	{
		if (m_pElement)
			m_pElement->UnRegisterEventFilter(this);

		if (m_yogaNode)
		{
			if (Parent() && Parent()->m_yogaNode)
			{
				auto parentYogaNode = Parent()->m_yogaNode;
				YGNodeRemoveChild(parentYogaNode, m_yogaNode);
			}
			YGNodeFree(m_yogaNode);
		}
	}

	Visual* Visual::Parent()
	{
		return m_pParent;
	}

	elements::Element* Visual::Element() const
	{
		return m_pElement;
	}

	std::vector<std::shared_ptr<Visual>> Visual::Visuals() const
	{
		return m_visuals;
	}

	visuals::View* Visual::View() const
	{
		return m_pView;
	}

	void Visual::BuildVisuals()
	{
		if (!m_pElement)
			return;
		if (m_yogaNode)
			return;

		m_yogaNode = YGNodeNew();
		BuildComputedStyle();
		SyncLayoutProperties();

		if (Parent() && Parent()->m_yogaNode)
		{
			auto parentYogaNode = Parent()->m_yogaNode;
			YGNodeInsertChild(parentYogaNode, m_yogaNode, YGNodeGetChildCount(parentYogaNode));
		}
		auto elements = m_pElement->Children();
		for (elements::Element* element : elements)
		{
			auto visual = element->CreateVisual(View(), this);
			visual->BuildVisuals();
			m_visuals.push_back(visual);
		}

		Reflow();
	}

	void Visual::Update()
	{
		Styles* styles = Element()->GetStyles();
		if (!styles)
			return;

		auto newComputedStyle = styles->Compute(Element());
		if (!newComputedStyle)
			return;

		auto diff = newComputedStyle->Diff(m_spComputedStyle.get());
		m_spComputedStyle = newComputedStyle;

		if (diff.layoutChanged)
		{
			SyncLayoutProperties();
			Reflow();
		}

		if (diff.visualChanged)
		{
			Repaint();
		}
	}

	void Visual::Reflow()
	{
		if (!m_yogaNode)
			return;

		LayoutRequestEvent lre(this);
		View()->FireEvent(&lre);
	}

	void Visual::Repaint()
	{
		PaintRequestEvent pre(this, VisualGeometry().BoundingBox());
		View()->FireEvent(&pre);
	}

	void Visual::CalculateLayout(const basic::SizeF& size)
	{
		if (!m_yogaNode)
			return;
		if (!m_spComputedStyle)
			return;

		auto style = m_spComputedStyle->layoutProps.style;
		YGNodeCalculateLayout(m_yogaNode, size.width, size.height, (YGDirection)style.direction());
	}

	const basic::RectF Visual::LayoutRect() const
	{
		return internal::yoga_utils::GetNodeBorderRect(m_yogaNode);
	}

	const basic::TransformF Visual::Transform() const
	{
		if (!m_spComputedStyle)
			return TransformF();
		return m_spComputedStyle->visualProps.transform;;
	}

	bool Visual::HitTest(const basic::PointF& localPos, Visual** ppHitVisual, basic::PointF& hitLocalPos)
	{
		if (!m_yogaNode)
			return false;
		if (!m_spComputedStyle)
			return false;

		if (Element()->Visibility()->IsHidden())
		{
			// the entire visuals tree is not visible.
			return false;
		}

		auto _HitTestChildVisual = [&]() -> bool
		{
			for (auto child : std::views::reverse(m_visuals))
			{
				PointF childLocalPos = localPos - child->LayoutRect().TopLeft();
				childLocalPos = child->Transform().Reversed().ApplyTo(childLocalPos);
				if (child->HitTest(childLocalPos, ppHitVisual, hitLocalPos))
					return true;
			}
			return false;
		};

		if (Element()->Visibility()->IsCollapsed())
		{
			//current visual not visible, but children visuals may be visible.
			return _HitTestChildVisual();
		}

		RectF localRect = LayoutRect();
		localRect.MoveTo(PointF(0, 0));
		bool bSelfHit = localRect.IsContains(localPos) && HitTestOverride(localPos);

		bool bOverFlowVisible = m_spComputedStyle->layoutProps.style.overflow() != facebook::yoga::Overflow::Hidden;
		if (!bSelfHit && !bOverFlowVisible)
		{
			//overflow children visuals will be clipped, no hittesting needed.
			return false;
		}

		if (_HitTestChildVisual())
			return true;

		if (bSelfHit)
		{
			*ppHitVisual = this;
			hitLocalPos = localPos;
		}

		return bSelfHit;
	}

	bool Visual::HitTestOverride(const basic::PointF& localPos)
	{
		return true;
	}

	void Visual::Paint(SkCanvas* canvas, const basic::RectF& dirtyRect)
	{
		if (Element()->Visibility()->IsHidden())
			return;

		canvas->save();
		bool bCollapsed = Element()->Visibility()->IsCollapsed();
		if (!bCollapsed)
		{
			canvas->concat(internal::skia_utils::ToSkMatrix(Transform()));
			PaintOverride(canvas, dirtyRect);
		}

		for (auto visual : std::views::reverse(m_visuals))
		{
			canvas->save();
			canvas->translate(visual->LayoutRect().X(), visual->LayoutRect().Y());
			visual->Paint(canvas, dirtyRect);
			canvas->restore();
		}

		canvas->restore();
	}

	void Visual::PaintOverride(SkCanvas* canvas, const basic::RectF& dirtyRect)
	{
		if (!m_spComputedStyle)
			return;

		RectF rect = LayoutRect();
		rect.MoveTo(PointF(0.0f, 0.0f));

		SkRRect borderRect = internal::skia_utils::ToSkRRect(rect, m_spComputedStyle->visualProps.radius);
		SkColor borderColor = internal::skia_utils::ToSkColor(m_spComputedStyle->visualProps.borderColor);

		SkPaint borderPaint;
		borderPaint.setColor(borderColor);
		borderPaint.setAntiAlias(true);
		borderPaint.setStyle(SkPaint::kFill_Style);
		canvas->drawRRect(borderRect, borderPaint);

		rect = rect - internal::yoga_utils::GetNodeBorders(m_yogaNode);
		SkRRect innerRect = internal::skia_utils::ToSkRRect(rect, m_spComputedStyle->visualProps.radius);
		SkColor bacgroudColor = internal::skia_utils::ToSkColor(m_spComputedStyle->visualProps.backgroundColor);
		SkPaint backgroundPaint;
		backgroundPaint.setColor(bacgroudColor);
		backgroundPaint.setAntiAlias(true);
		backgroundPaint.setStyle(SkPaint::kFill_Style);
		canvas->drawRRect(innerRect, backgroundPaint);
	}

	bool Visual::Filter(events::EventTarget* target, events::Event* e)
	{
		if (target == m_pElement)
		{
			if (e->type == Type::ElementAdd)
			{
				ElementModifyEvent* event = static_cast<ElementModifyEvent*>(e);
				auto visual = event->element->CreateVisual(View(), this);
				visual->BuildVisuals();
				m_visuals.push_back(visual);
				Reflow();
			}
			else if (e->type == Type::ElementRemove)
			{
				ElementModifyEvent* event = static_cast<ElementModifyEvent*>(e);
				auto iter = std::find_if(m_visuals.cbegin(), m_visuals.cend(),
					[=](const std::shared_ptr<Visual>& visual) {
						return visual->Element() == event->element;
					});

				if (iter != m_visuals.cend())
				{
					m_visuals.erase(iter);
					Reflow();
				}
			}
			else if (e->type == Type::ElementStatesChanged)
			{
				ElementStatesChangedEvent* event = static_cast<ElementStatesChangedEvent*>(e);
				Update();
			}
			else if (e->type == Type::ElementVisibilityChanged)
			{
				ElementVisibilityChangedEvent* event = static_cast<ElementVisibilityChangedEvent*>(e);
				Update();
			}
		}
		return false;
	}

	void Visual::BuildComputedStyle()
	{
		if (m_spComputedStyle)
			return;
		if (!Element())
			return;

		auto styles = Element()->GetStyles();
		if (!styles)
			return;

		m_spComputedStyle = styles->Compute(Element());
	}

	void Visual::SyncLayoutProperties()
	{
		using namespace facebook::yoga;
		const Style& style = m_spComputedStyle->layoutProps.style;
		YGNodeRef node = m_yogaNode;
		internal::yoga_utils::SetNodeStyle(node, style);
	}

	const PolygonF Visual::VisualGeometry()
	{
		RectF lbr = LayoutRect();
		PolygonF polygon = PolygonF(lbr);
		//TODO:: add transform.
		return polygon;
	}
}
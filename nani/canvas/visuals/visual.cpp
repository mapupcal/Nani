#include "visual.h"
#include "view.h"

#include "../elements/element.h"
#include "../elements/element_visibility.h"
#include "../styles.h"

#include "../internal/computed_style.h"
#include "../internal/skia_defs.h"
#include "../internal/skia_utils.h"
#include "../internal/yoga_utils.h"

using namespace nani::canvas::elements;
using namespace nani::canvas::events;
using namespace nani::canvas::basic;
using namespace nani::canvas::internal;

namespace nani::canvas::visuals
{
	namespace
	{
		constexpr scalar kBlurSigmaScale = 0.5f;
		constexpr scalar kBlurBoundsScale = 3.0f;

		bool HasVisibleShadow(const ComputedStyle::VisualProperties::Shadow& shadow)
		{
			if (shadow.color.a == 0)
				return false;
			return !IsScalarEqual(shadow.offsetX, 0.0f) ||
				!IsScalarEqual(shadow.offsetY, 0.0f) ||
				!IsScalarEqual(shadow.blur, 0.0f) ||
				!IsScalarEqual(shadow.spread, 0.0f);
		}

		bool HasVisibleBorder(const MarginsF& borders, const Color& borderColor)
		{
			if (borderColor.a == 0)
				return false;
			return borders.left > 0.0f || borders.top > 0.0f ||
				borders.right > 0.0f || borders.bottom > 0.0f;
		}

		MarginsF ShadowExpandMargins(const ComputedStyle::VisualProperties::Shadow& shadow)
		{
			if (!HasVisibleShadow(shadow))
				return MarginsF();

			const scalar blurExtent = shadow.blur * kBlurBoundsScale;
			const scalar spread = std::max(0.0f, shadow.spread);
			return MarginsF(
				spread + blurExtent + std::max(0.0f, -shadow.offsetX),
				spread + blurExtent + std::max(0.0f, -shadow.offsetY),
				spread + blurExtent + std::max(0.0f, shadow.offsetX),
				spread + blurExtent + std::max(0.0f, shadow.offsetY));
		}

		RectF LocalLayoutBounds(const RectF& layoutRect)
		{
			RectF local = layoutRect;
			local.MoveTo(PointF(0.0f, 0.0f));
			return local;
		}

		bool DirtyIntersects(const RectF& dirtyRect, const RectF& bounds)
		{
			if (!dirtyRect.IsValid() || !bounds.IsValid())
				return true;
			const RectF hit = dirtyRect.Intersected(bounds);
			return hit.Width() > 0.0f && hit.Height() > 0.0f;
		}

		void DrawShadow(
			SkCanvas* canvas,
			const RectF& localRect,
			const ComputedStyle::VisualProperties::BorderRadius& radius,
			const ComputedStyle::VisualProperties::Shadow& shadow)
		{
			if (!HasVisibleShadow(shadow))
				return;

			const scalar spread = shadow.spread;
			RectF shadowRect = localRect + MarginsF(spread, spread, spread, spread);
			auto shadowRadius = skia_utils::OutsetBorderRadius(radius, std::max(0.0f, spread));
			SkRRect rrect = skia_utils::ToSkRRect(shadowRect, shadowRadius);

			SkPaint paint;
			paint.setAntiAlias(true);
			paint.setStyle(SkPaint::kFill_Style);
			paint.setColor(skia_utils::ToSkColor(shadow.color));
			if (shadow.blur > 0.0f)
			{
				const scalar sigma = std::max(shadow.blur * kBlurSigmaScale, 0.01f);
				paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma));
			}

			canvas->save();
			canvas->translate(shadow.offsetX, shadow.offsetY);
			canvas->drawRRect(rrect, paint);
			canvas->restore();
		}
	}

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
		TransformF transform = m_spComputedStyle->visualProps.transform;
		PointF origin = TransformOrigin();
		return TransformF::Translation(-origin.x, -origin.y)
			.Then(transform)
			.Then(TransformF::Translation(origin.x, origin.y));
	}

	const basic::PointF Visual::TransformOrigin() const
	{
		if (!m_spComputedStyle)
			return PointF();
		auto rect = LayoutRect();
		rect.MoveTo(PointF(0, 0));
		auto transformOrigin = m_spComputedStyle->visualProps.transformOrigin;
		return internal::yoga_utils::GetPointInRect(rect, transformOrigin.x, transformOrigin.y);
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

		const PointF scrollOffset = ContentScrollOffset();
		auto _HitTestChildVisual = [&]() -> bool
		{
			for (auto child : std::views::reverse(m_visuals))
			{
				const PointF childOrigin = child->LayoutRect().TopLeft() - scrollOffset;
				PointF childLocalPos = localPos - childOrigin;
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

		RectF localRect = LocalLayoutBounds(LayoutRect());
		const bool bInsideShape = skia_utils::ContainsPoint(
			localRect,
			m_spComputedStyle->visualProps.radius,
			localPos);
		const bool bSelfHit = bInsideShape && HitTestOverride(localPos);

		const auto overflow = m_spComputedStyle->layoutProps.style.overflow();
		const bool bOverFlowVisible =
			overflow != facebook::yoga::Overflow::Hidden &&
			overflow != facebook::yoga::Overflow::Scroll;
		if (!bInsideShape && !bOverFlowVisible)
		{
			// overflow children are clipped to the rounded border box.
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
		if (!m_spComputedStyle)
			return;

		const scalar opacity = m_spComputedStyle->visualProps.opacity;
		if (opacity <= 0.0f)
			return;

		const RectF localPaintBounds = LocalLayoutBounds(LayoutRect());
		PolygonF paintPolygon(localPaintBounds);
		paintPolygon = Transform().ApplyTo(paintPolygon);
		RectF cullBounds = paintPolygon.BoundingBox();
		if (HasVisibleShadow(m_spComputedStyle->visualProps.shadow))
			cullBounds = cullBounds + ShadowExpandMargins(m_spComputedStyle->visualProps.shadow);

		if (!DirtyIntersects(dirtyRect, cullBounds))
			return;

		canvas->save();

		const bool useOpacityLayer = opacity < 1.0f;
		if (useOpacityLayer)
		{
			SkPaint layerPaint;
			layerPaint.setAlphaf(opacity);
			canvas->saveLayer(nullptr, &layerPaint);
		}

		bool bCollapsed = Element()->Visibility()->IsCollapsed();
		if (!bCollapsed)
		{
			canvas->concat(internal::skia_utils::ToSkMatrix(Transform()));
			PaintOverride(canvas, dirtyRect);

			const auto overflow = m_spComputedStyle->layoutProps.style.overflow();
			const bool clipOverflow =
				overflow == facebook::yoga::Overflow::Hidden ||
				overflow == facebook::yoga::Overflow::Scroll;
			if (clipOverflow)
			{
				// Clip to the padding box so scrolled/overflow children do not
				// paint over the border ring.
				const MarginsF borders = yoga_utils::GetNodeBorders(m_yogaNode);
				const RectF clipBounds = localPaintBounds - borders;
				if (clipBounds.Width() > 0.0f && clipBounds.Height() > 0.0f)
				{
					const auto clipRadius = skia_utils::InsetBorderRadius(
						m_spComputedStyle->visualProps.radius,
						borders);
					canvas->clipRRect(
						skia_utils::ToSkRRect(clipBounds, clipRadius),
						true);
				}
			}
		}

		const PointF scrollOffset = ContentScrollOffset();
		for (auto visual : m_visuals)
		{
			const PointF childOrigin = visual->LayoutRect().TopLeft() - scrollOffset;
			RectF childDirty(
				dirtyRect.left - childOrigin.x,
				dirtyRect.top - childOrigin.y,
				dirtyRect.right - childOrigin.x,
				dirtyRect.bottom - childOrigin.y);

			canvas->save();
			canvas->translate(childOrigin.x, childOrigin.y);
			visual->Paint(canvas, childDirty);
			canvas->restore();
		}

		if (useOpacityLayer)
			canvas->restore();

		canvas->restore();
	}

	void Visual::PaintOverride(SkCanvas* canvas, const basic::RectF& dirtyRect)
	{
		if (!m_spComputedStyle)
			return;

		RectF rect = LocalLayoutBounds(LayoutRect());
		const auto& visualProps = m_spComputedStyle->visualProps;
		const MarginsF borders = yoga_utils::GetNodeBorders(m_yogaNode);

		DrawShadow(canvas, rect, visualProps.radius, visualProps.shadow);

		if (HasVisibleBorder(borders, visualProps.borderColor))
		{
			SkRRect borderRect = skia_utils::ToSkRRect(rect, visualProps.radius);
			SkPaint borderPaint;
			borderPaint.setColor(skia_utils::ToSkColor(visualProps.borderColor));
			borderPaint.setAntiAlias(true);
			borderPaint.setStyle(SkPaint::kFill_Style);
			canvas->drawRRect(borderRect, borderPaint);
		}

		rect = rect - borders;
		if (rect.Width() <= 0.0f || rect.Height() <= 0.0f)
			return;

		const auto innerRadius = skia_utils::InsetBorderRadius(visualProps.radius, borders);
		SkRRect innerRect = skia_utils::ToSkRRect(rect, innerRadius);
		SkPaint backgroundPaint;
		backgroundPaint.setColor(skia_utils::ToSkColor(visualProps.backgroundColor));
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
				SyncLayoutProperties();
				Reflow();
				Repaint();
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
		Style style = m_spComputedStyle->layoutProps.style;

		// Sync visibility to yoga node.
		// Default is Display::Flex, which means the visual is visible.
		if (Element()->Visibility()->IsCollapsed())
			style.setDisplay(Display::Contents);
		else if (Element()->Visibility()->IsHidden())
			style.setDisplay(Display::None);

		YGNodeRef node = m_yogaNode;
		internal::yoga_utils::SetNodeStyle(node, style);
	}

	const PolygonF Visual::VisualGeometry()
	{
		RectF local = LocalLayoutBounds(LayoutRect());
		PolygonF polygon(local);
		polygon = Transform().ApplyTo(polygon);

		RectF bounds = polygon.BoundingBox();
		if (m_spComputedStyle && HasVisibleShadow(m_spComputedStyle->visualProps.shadow))
			bounds = bounds + ShadowExpandMargins(m_spComputedStyle->visualProps.shadow);

		// Map from this visual's parent space into root-local coordinates.
		PointF origin = LayoutRect().TopLeft();
		for (Visual* ancestor = Parent(); ancestor; ancestor = ancestor->Parent())
			origin += ancestor->LayoutRect().TopLeft();

		bounds.MoveTo(PointF(bounds.left + origin.x, bounds.top + origin.y));
		return PolygonF(bounds);
	}

	bool Visual::IsWindowDrag() const
	{
		const ComputedStyle* style = m_spComputedStyle.get();
		return style && style->visualProps.windowDrag;
	}

	bool Visual::HasWindowDragDescendant() const
	{
		if (IsWindowDrag())
			return true;
		for (const auto& child : m_visuals)
		{
			if (child && child->HasWindowDragDescendant())
				return true;
		}
		return false;
	}

	const RectF Visual::ContentRect() const
	{
		if (!m_yogaNode)
			return RectF();
		return yoga_utils::GetNodeContentRect(m_yogaNode);
	}

	PointF Visual::ContentScrollOffset() const
	{
		return PointF();
	}

	const ComputedStyle* Visual::GetComputedStyle() const
	{
		return m_spComputedStyle.get();
	}

	YGNodeRef Visual::YogaNode() const
	{
		return m_yogaNode;
	}
}

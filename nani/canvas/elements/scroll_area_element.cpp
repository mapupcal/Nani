#include "scroll_area_element.h"
#include "../visuals/scroll_area_visual.h"
#include "../events/event.h"

using namespace nani::canvas::basic;
using namespace nani::canvas::visuals;
using namespace nani::canvas::events;

namespace nani::canvas::elements
{
	namespace
	{
		constexpr scalar kWheelScale = 40.0f;

		scalar ClampScalar(scalar value, scalar minValue, scalar maxValue)
		{
			if (value < minValue)
				return minValue;
			if (value > maxValue)
				return maxValue;
			return value;
		}
	}

	ScrollAreaElement::ScrollAreaElement(Element* parent)
		: Element(parent)
	{
		SetStyleClass(u8"ScrollableColumn");
	}

	void ScrollAreaElement::SetScrollOffset(const PointF& offset)
	{
		const PointF clamped = ClampedOffset(offset);
		if (m_scrollOffset == clamped)
			return;

		m_scrollOffset = clamped;
		ElementScrollChangedEvent event(this);
		FireEvent(&event);
	}

	const PointF& ScrollAreaElement::ScrollOffset() const
	{
		return m_scrollOffset;
	}

	void ScrollAreaElement::ScrollBy(scalar dx, scalar dy)
	{
		SetScrollOffset(PointF(m_scrollOffset.x + dx, m_scrollOffset.y + dy));
	}

	void ScrollAreaElement::UpdateScrollMetrics(const SizeF& contentSize, const SizeF& viewportSize)
	{
		const bool metricsChanged =
			m_contentSize != contentSize || m_viewportSize != viewportSize;
		m_contentSize = contentSize;
		m_viewportSize = viewportSize;
		if (!metricsChanged)
			return;

		const PointF clamped = ClampedOffset(m_scrollOffset);
		if (m_scrollOffset == clamped)
			return;

		m_scrollOffset = clamped;
		ElementScrollChangedEvent event(this);
		FireEvent(&event);
	}

	const SizeF& ScrollAreaElement::ContentSize() const
	{
		return m_contentSize;
	}

	const SizeF& ScrollAreaElement::ViewportSize() const
	{
		return m_viewportSize;
	}

	std::shared_ptr<Visual> ScrollAreaElement::CreateVisual(View* view, Visual* visualParent)
	{
		return std::make_shared<ScrollAreaVisual>(view, this, visualParent);
	}

	void ScrollAreaElement::OnEvent(Event* e)
	{
		if (e->type == Type::Wheel)
		{
			auto* wheel = static_cast<WheelEvent*>(e);
			ScrollBy(
				static_cast<scalar>(-wheel->deltaX * kWheelScale),
				static_cast<scalar>(-wheel->deltaY * kWheelScale));
			return;
		}

		Element::OnEvent(e);
	}

	PointF ScrollAreaElement::ClampedOffset(const PointF& offset) const
	{
		const scalar maxX = std::max(0.0f, m_contentSize.width - m_viewportSize.width);
		const scalar maxY = std::max(0.0f, m_contentSize.height - m_viewportSize.height);
		return PointF(
			ClampScalar(offset.x, 0.0f, maxX),
			ClampScalar(offset.y, 0.0f, maxY));
	}
}

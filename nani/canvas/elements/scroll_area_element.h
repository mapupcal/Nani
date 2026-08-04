#pragma once
#include "elements_defs.h"
#include "element.h"

namespace nani::canvas::elements
{
	class NANI_CANVAS_API ScrollAreaElement : public Element
	{
	public:
		explicit ScrollAreaElement(Element* parent);
		ScrollAreaElement(const ScrollAreaElement&) = delete;
		~ScrollAreaElement() override = default;

	public:
		void SetScrollOffset(const basic::PointF& offset);
		const basic::PointF& ScrollOffset() const;
		void ScrollBy(basic::scalar dx, basic::scalar dy);

		void UpdateScrollMetrics(const basic::SizeF& contentSize, const basic::SizeF& viewportSize);
		const basic::SizeF& ContentSize() const;
		const basic::SizeF& ViewportSize() const;

		std::shared_ptr<visuals::Visual> CreateVisual(
			visuals::View* view,
			visuals::Visual* visualParent) override;

	protected:
		void OnEvent(events::Event* e) override;

	private:
		basic::PointF ClampedOffset(const basic::PointF& offset) const;

	private:
		basic::PointF m_scrollOffset;
		basic::SizeF m_contentSize;
		basic::SizeF m_viewportSize;
	};
}

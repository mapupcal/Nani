#include "view.h"
#include "visual.h"

#include "elements/element.h"
#include "elements/element_states.h"
#include "elements/scroll_area_element.h"
#include "elements/input_text_element.h"

#include "events/event.h"
#include "window.h"

#include "internal/skia_defs.h"

using namespace nani::canvas::basic;
using namespace nani::canvas::events;

namespace nani::canvas::visuals
{
	View::View(canvas::Window* window)
		: m_pWindow(window)
	{

	}

	View::~View()
	{

	}

	canvas::Window* View::Window() const
	{
		return m_pWindow;
	}

	visuals::Visual* View::Visual()
	{
		return m_spVisual.get();
	}

	void View::BuildVisuals()
	{
		if (m_spVisual)
		{
			MarkDirty();
			return;
		}

		m_spVisual = Window()->RootElement()->CreateVisual(this, nullptr);
		m_spVisual->BuildVisuals();
		MarkDirty();
	}

	void View::MarkDirty()
	{
		m_bLayoutDirty = true;
		m_bPaintDirty = true;
		const SizeF size = Window()->ClientRect().Size();
		m_dirtyRect = RectF(PointF(0.0f, 0.0f), size);
	}

	void View::Flush()
	{
		if (!m_spVisual)
			return;

		if(m_bLayoutDirty)
		{
			SizeF size = Window()->ClientRect().Size();

			m_spVisual->CalculateLayout(size);

			m_bLayoutDirty = false;
			Window()->SyncWindowDrag();
		}

		if (m_bPaintDirty)
		{
			SkCanvas* canvas = Window()->GetCanvas();
			if (!canvas)
				return;

			canvas->save();
			const RectF clientRect = Window()->ClientRect();
			canvas->translate(clientRect.X(), clientRect.Y());
			// WindowPrivate::Repaint clears the entire surface each frame, so the
			// paint pass must cover the full client area in root-local space.
			const RectF paintRect(PointF(0.0f, 0.0f), clientRect.Size());
			m_spVisual->Paint(canvas, paintRect);
			canvas->restore();

			m_bPaintDirty = false;
			m_dirtyRect = RectF();
		}
	}

	bool View::IsDirty() const
	{
		return m_bLayoutDirty || m_bPaintDirty;
	}

	void View::OnEvent(events::Event* e)
	{
		switch (e->type)
		{
		case events::Type::LayoutRequest:
		{
			OnLayoutRequest(static_cast<events::LayoutRequestEvent*>(e));
			break;
		}
		case events::Type::PaintRequest:
		{
			OnPaintRequest(static_cast<events::PaintRequestEvent*>(e));
			break;
		}
		case events::Type::ImeCaretRect:
		{
			OnImeCaretRect(static_cast<events::ImeCaretRectEvent*>(e));
			break;
		}
		}
	}

	void View::OnImeCaretRect(events::ImeCaretRectEvent* e)
	{
		if (!e || !Window())
			return;
		Window()->SetImeCaretRect(e->clientRect);
	}

	void View::OnLayoutRequest(events::LayoutRequestEvent* e)
	{
		m_bLayoutDirty = true;
	}

	void View::OnPaintRequest(events::PaintRequestEvent* e)
	{
		m_bPaintDirty = true;
		m_dirtyRect = m_dirtyRect | static_cast<events::PaintRequestEvent*>(e)->dirtyRect;
	}

	void View::BubbleMouseEvent(visuals::Visual* hitVisual, PointF hitLocalPos, MouseEvent* e)
	{
		if (!hitVisual || !e)
			return;

		PointF localPos = hitLocalPos;
		for (visuals::Visual* visual = hitVisual; visual; )
		{
			e->pos = localPos;
			if (elements::Element* element = visual->Element())
				element->FireEvent(e);
			if (e->IsAccepted())
				break;

			visuals::Visual* parent = visual->Parent();
			if (!parent)
				break;
			localPos = visual->MapToParentLocal(localPos);
			visual = parent;
		}
	}

	void View::OnMouseMove(events::MouseMoveEvent* e)
	{
		PointF hitLocalPos;
		visuals::Visual* hitVisual = HitTestVisual(e->pos, hitLocalPos);
		HoverElement(hitVisual ? hitVisual->Element() : nullptr);
		if (!hitVisual)
			return;

		MouseMoveEvent me(hitLocalPos, e->globalPos);
		BubbleMouseEvent(hitVisual, hitLocalPos, &me);
	}

	void View::OnMousePress(events::MousePressEvent* e)
	{
		PointF hitLocalPos;
		visuals::Visual* hitVisual = HitTestVisual(e->pos, hitLocalPos);
		elements::Element* hitElement = hitVisual ? hitVisual->Element() : nullptr;
		HoverElement(hitElement);
		SetFocus(FindFocusable(hitElement));
		if (!hitVisual)
			return;

		MousePressEvent me(e->button, hitLocalPos, e->globalPos, e->modifier);
		BubbleMouseEvent(hitVisual, hitLocalPos, &me);
	}

	void View::OnMouseRelease(events::MouseReleaseEvent* e)
	{
		PointF hitLocalPos;
		visuals::Visual* hitVisual = HitTestVisual(e->pos, hitLocalPos);
		HoverElement(hitVisual ? hitVisual->Element() : nullptr);
		if (!hitVisual)
			return;

		MouseReleaseEvent me(e->button, hitLocalPos, e->globalPos, e->modifier);
		BubbleMouseEvent(hitVisual, hitLocalPos, &me);
	}

	void View::OnWheel(events::WheelEvent* e)
	{
		// Hit-test without updating hover (unlike mouse move/press).
		PointF hitLocalPos;
		visuals::Visual* hitVisual = HitTestVisual(e->pos, hitLocalPos);
		if (!hitVisual)
			return;

		WheelEvent we(hitLocalPos, e->globalPos, e->deltaX, e->deltaY);
		BubbleMouseEvent(hitVisual, hitLocalPos, &we);
	}

	void View::OnKeyPress(events::KeyPressEvent* e)
	{
		RouteToFocus(e);
	}

	void View::OnKeyRelease(events::KeyReleaseEvent* e)
	{
		RouteToFocus(e);
	}

	void View::OnChar(events::CharEvent* e)
	{
		RouteToFocus(e);
	}

	void View::OnImeComposition(events::Event* e)
	{
		RouteToFocus(e);
	}

	void View::SetFocus(elements::Element* element)
	{
		if (m_spFocusElement == element)
			return;

		if (m_spFocusElement)
		{
			if (auto* input = dynamic_cast<elements::InputTextElement*>(m_spFocusElement.get()))
				input->EndComposition();
			m_spFocusElement->States()->SetFocused(false);
		}

		m_spFocusElement = element;

		if (m_spFocusElement)
			m_spFocusElement->States()->SetFocused(true);
	}

	elements::Element* View::FindFocusable(elements::Element* candidate) const
	{
		for (elements::Element* element = candidate; element; element = element->Parent())
		{
			if (dynamic_cast<elements::InputTextElement*>(element))
				return element;
		}
		return nullptr;
	}

	void View::RouteToFocus(events::Event* e)
	{
		if (!m_spFocusElement || !e)
			return;
		m_spFocusElement->FireEvent(e);
	}

	PointF View::ToRootLocal(const PointF& windowPos) const
	{
		PointF pos = windowPos;
		pos -= Window()->ClientRect().TopLeft();
		if (m_spVisual)
			pos = m_spVisual->Transform().Reversed().ApplyTo(pos);
		return pos;
	}

	visuals::Visual* View::HitTestVisual(const PointF& windowPos, PointF& hitLocalPos)
	{
		if (!m_spVisual)
			return nullptr;

		visuals::Visual* hitVisual = nullptr;
		if (!m_spVisual->HitTest(ToRootLocal(windowPos), &hitVisual, hitLocalPos))
			return nullptr;
		return hitVisual;
	}

	elements::Element* View::HitTest(const PointF& windowPos, PointF& hitLocalPos)
	{
		visuals::Visual* hitVisual = HitTestVisual(windowPos, hitLocalPos);
		return hitVisual ? hitVisual->Element() : nullptr;
	}

	bool View::IsWindowDragAt(const PointF& windowPos)
	{
		PointF hitLocalPos;
		visuals::Visual* hit = HitTestVisual(windowPos, hitLocalPos);
		return hit && hit->IsWindowDrag();
	}

	void View::UpdateHoverAt(const PointF& windowPos)
	{
		PointF hitLocalPos;
		HoverElement(HitTest(windowPos, hitLocalPos));
	}

	void View::ClearHover()
	{
		HoverElement(nullptr);
	}

	elements::Element* View::HitTest(events::MouseEvent* e, PointF& hitLocalPos)
	{
		return HoverElement(HitTest(e->pos, hitLocalPos));
	}

	namespace
	{
		// Leaf -> root.
		std::vector<elements::Element*> HoverAncestorChain(elements::Element* leaf)
		{
			std::vector<elements::Element*> chain;
			for (elements::Element* element = leaf; element; element = element->Parent())
				chain.push_back(element);
			return chain;
		}
	}

	elements::Element* View::HoverElement(elements::Element* candidate)
	{
		if (m_spHoverElement == candidate)
			return candidate;

		const auto oldChain = HoverAncestorChain(m_spHoverElement.get());
		const auto newChain = HoverAncestorChain(candidate);

		// Shared ancestors from the root stay hovered across sibling moves.
		size_t oldUnique = oldChain.size();
		size_t newUnique = newChain.size();
		while (oldUnique > 0 && newUnique > 0 &&
			oldChain[oldUnique - 1] == newChain[newUnique - 1])
		{
			--oldUnique;
			--newUnique;
		}

		for (size_t i = 0; i < oldUnique; ++i)
		{
			Event leave(Type::Leave);
			oldChain[i]->FireEvent(&leave);
		}

		m_spHoverElement = candidate;

		for (size_t i = newUnique; i > 0; --i)
		{
			Event enter(Type::Enter);
			newChain[i - 1]->FireEvent(&enter);
		}

		return m_spHoverElement;
	}
}
#include "view.h"
#include "visual.h"
#include "elements/element.h"
#include "elements/element_states.h"
#include "events/event.h"
#include <core/SkCanvas.h>
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
		m_dirtyRect = Window()->ClientRect();
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
		}

		if (m_bPaintDirty)
		{
			SkCanvas* canvas = Window()->GetCanvas();
			if (!canvas)
				return;

			canvas->save();
			RectF rect = Window()->ClientRect();
			canvas->translate(rect.X(), rect.Y());
			m_spVisual->Paint(canvas, m_dirtyRect);
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
		}
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

	void View::OnMouseMove(events::MouseMoveEvent* e)
	{
		if (PointF hitLocalPos; elements::Element* hitElement = HitTest(e, hitLocalPos))
		{
			MouseMoveEvent me(hitLocalPos, e->globalPos);
			hitElement->FireEvent(&me);
		}
	}

	void View::OnMousePress(events::MousePressEvent* e)
	{
		if (PointF hitLocalPos; elements::Element* hitElement = HitTest(e, hitLocalPos))
		{
			MousePressEvent me(e->button, hitLocalPos, e->globalPos, e->modifier);
			hitElement->FireEvent(&me);
		}
	}

	void View::OnMouseRelease(events::MouseReleaseEvent* e)
	{
		if (PointF hitLocalPos; elements::Element* hitElement = HitTest(e, hitLocalPos))
		{
			MouseReleaseEvent me(e->button, hitLocalPos, e->globalPos, e->modifier);
			hitElement->FireEvent(&me);
		}
	}

	void View::OnWheel(events::WheelEvent* e)
	{

	}

	void View::OnKeyPress(events::KeyPressEvent* e)
	{

	}

	void View::OnKeyRelease(events::KeyReleaseEvent* e)
	{

	}

	elements::Element* View::HitTest(events::MouseEvent* e, PointF& hitLocalPos)
	{
		if (!m_spVisual)
			return nullptr;

		PointF pos = e->pos;
		RectF clientRect = Window()->ClientRect();
		pos -= clientRect.TopLeft();
		pos = m_spVisual->Transform().Reversed().ApplyTo(pos);

		visuals::Visual* hitVisual = nullptr;
		m_spVisual->HitTest(pos, &hitVisual, hitLocalPos);
		return HoverElement(hitVisual ? hitVisual->Element() : nullptr);
	}

	elements::Element* View::HoverElement(elements::Element* candidate)
	{
		if (m_spHoverElement == candidate)
			return candidate;

		if (m_spHoverElement)
		{
			Event le(Type::Leave);
			m_spHoverElement->FireEvent(&le);
		}

		m_spHoverElement = candidate;

		if (m_spHoverElement)
		{
			Event ee(Type::Enter);
			m_spHoverElement->FireEvent(&ee);
		}

		return m_spHoverElement;
	}
}
#include "event.h"
namespace nani::canvas::events
{
	Event::Event(Type type_)
		: type(type_)
	{

	}

	MoveEvent::MoveEvent(const basic::PointF& oldPos_, const basic::PointF& newPos_)
		: Event(Type::Move)
		, oldPos(oldPos_)
		, newPos(newPos_)
	{

	}

	ResizeEvent::ResizeEvent(const basic::SizeF& oldSize_, const basic::SizeF& newSize_)
		: Event(Type::Resize)
		, oldSize(oldSize_)
		, newSize(newSize_)
	{

	}

	PaintEvent::PaintEvent(const basic::RectF& dirtyRect_)
		: Event(Type::Paint)
		, dirtyRect(dirtyRect_)
	{

	}


	MouseEvent::MouseEvent(Type type_, const basic::PointF& pos_, const basic::PointF& globalPos_)
		: Event(type_)
		, pos(pos_)
		, globalPos(globalPos_)
	{

	}

	MouseMoveEvent::MouseMoveEvent(const basic::PointF& pos_, const basic::PointF& globalPos_)
		: MouseEvent(Type::MouseMove, pos_, globalPos_)
	{

	}

	MouseButtonEvent::MouseButtonEvent(Type type_, MouseButton button_, const basic::PointF& pos_, const basic::PointF& globalPos_, Modifier modifier_)
		: MouseEvent(type_, pos_, globalPos_)
		, button(button_)
		, modifier(modifier_)
	{

	}

	MousePressEvent::MousePressEvent(MouseButton button_, const basic::PointF& pos_, const basic::PointF& globalPos_, Modifier modifier_)
		: MouseButtonEvent(Type::MousePress, button_, pos_, globalPos_, modifier_)
	{

	}

	MouseReleaseEvent::MouseReleaseEvent(MouseButton button_, const basic::PointF& pos_, const basic::PointF& globalPos_, Modifier modifier_)
		: MouseButtonEvent(Type::MouseRelease, button_, pos_, globalPos_, modifier_)
	{

	}

	WheelEvent::WheelEvent(double deltaX_, double deltaY_)
		: Event(Type::Wheel)
		, deltaX(deltaX_)
		, deltaY(deltaY_)
	{

	}

	KeyEvent::KeyEvent(Type type_, Key key_, Modifier modifier_, int scancode_)
		: Event(type_)
		, key(key_)
		, modifier(modifier_)
		, scancode(scancode_)
	{

	}

	KeyPressEvent::KeyPressEvent(Key key_, Modifier modifier_, int scancode_)
		: KeyEvent(Type::KeyPress, key_, modifier_, scancode_)
	{

	}

	KeyReleaseEvent::KeyReleaseEvent(Key key_, Modifier modifier_, int scancode_)
		: KeyEvent(Type::KeyRelease, key_, modifier_, scancode_)
	{

	}
}


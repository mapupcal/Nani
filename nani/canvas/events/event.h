#pragma once
#include "event_defs.h"

namespace nani::canvas::events
{
	class NANI_CANVAS_API Event
	{
	public:
		Event(Type type_);
		~Event() = default;

		const Type type = Type::Unknown;
	};

	class NANI_CANVAS_API MoveEvent : public Event
	{
	public:
		MoveEvent(const basic::PointF& oldPos_, const basic::PointF& newPos_);

		const basic::PointF oldPos;
		const basic::PointF newPos;
	};

	class NANI_CANVAS_API ResizeEvent : public Event
	{
	public:
		ResizeEvent(const basic::SizeF& oldSize_, const basic::SizeF& newSize_);

		const basic::SizeF oldSize;
		const basic::SizeF newSize;
	};

	class NANI_CANVAS_API PaintEvent : public Event
	{
	public:
		PaintEvent(const basic::RectF& dirtyRect_);
		~PaintEvent() = default;

		const basic::RectF dirtyRect;
	};

	class NANI_CANVAS_API MouseEvent : public Event
	{
	public:
		MouseEvent(Type type_, const basic::PointF& pos_, const basic::PointF& globalPos_);
		~MouseEvent() = default;

		basic::PointF pos;
		basic::PointF globalPos;
	};

	class NANI_CANVAS_API MouseMoveEvent : public MouseEvent
	{
	public:
		MouseMoveEvent(const basic::PointF& pos_, const basic::PointF& globalPos_);
		~MouseMoveEvent() = default;
	};

	inline Modifier operator|(Modifier lhs, Modifier rhs)
	{
		return static_cast<Modifier>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
	}

	inline Modifier operator&(Modifier lhs, Modifier rhs)
	{
		return static_cast<Modifier>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
	}

	class NANI_CANVAS_API MouseButtonEvent : public MouseEvent
	{
	public:
		MouseButtonEvent(Type type_, MouseButton button_, const basic::PointF& pos_, const basic::PointF& globalPos_, Modifier modifier_);
		~MouseButtonEvent() = default;

		const MouseButton button = MouseButton::Unknown;
		const Modifier modifier = Modifier::None;
	};

	class NANI_CANVAS_API MousePressEvent : public MouseButtonEvent
	{
	public:
		MousePressEvent(MouseButton button_, const basic::PointF& pos_, const basic::PointF& globalPos_, Modifier modifier_ = Modifier::None);
		~MousePressEvent() = default;
	};

	class NANI_CANVAS_API MouseReleaseEvent : public MouseButtonEvent
	{
	public:
		MouseReleaseEvent(MouseButton button_, const basic::PointF& pos_, const basic::PointF& globalPos_, Modifier modifier_ = Modifier::None);
		~MouseReleaseEvent() = default;
	};

	class NANI_CANVAS_API WheelEvent : public Event
	{
	public:
		WheelEvent(double deltaX_, double deltaY_);
		~WheelEvent() = default;

		const double deltaX = 0.0f;
		const double deltaY = 0.0f;
	};

	class NANI_CANVAS_API KeyEvent : public Event
	{
	public:
		KeyEvent(Type type_, Key key_, Modifier modifier_, int scancode_);
		~KeyEvent() = default;

	public:
		const Key key = Key::Unknown;
		const Modifier modifier = Modifier::None;
		const int scancode = 0;
	};

	class NANI_CANVAS_API KeyPressEvent : public KeyEvent
	{
	public:
		KeyPressEvent(Key key_, Modifier modifier_ = Modifier::None, int scancode_ = 0);
		~KeyPressEvent() = default;
	};

	class NANI_CANVAS_API KeyReleaseEvent : public KeyEvent
	{
	public:
		KeyReleaseEvent(Key key_, Modifier modifier_ = Modifier::None, int scancode_ = 0);
		~KeyReleaseEvent() = default;
	};

	class NANI_CANVAS_API ElementModifyEvent : public Event
	{
	public:
		ElementModifyEvent(Type type, elements::Element* element_);
		~ElementModifyEvent() = default;

		elements::Element* const element = nullptr;
	};

	class NANI_CANVAS_API ElementStatesChangedEvent : public Event
	{
	public:
		ElementStatesChangedEvent(elements::Element* element_);
		~ElementStatesChangedEvent() = default;

		elements::Element* const element = nullptr;
	};
}

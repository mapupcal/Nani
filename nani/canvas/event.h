#pragma once
#include "defs.h"
#include "basic/pointf.h"
#include "basic/sizef.h"
#include "basic/rectf.h"

namespace nani::canvas
{
	class NANI_API Event
	{
	public:
		struct IFilter
		{
			virtual bool FilterEvent(Event* event) = 0;
		};

		enum class Type
		{
			Unknown,

			AboutToQuit,
			Quit,

			MouseMove,
			MousePress,
			MouseRelease,
			Wheel,

			KeyPress,
			KeyRelese,

			Show,
			Hide,
			Close,
			Move,
			Resize,
			FocusIn,
			FocusOut,
			Paint,
			Enter,
			Leave,
		};

	public:
		Event(Type type_);
		~Event() = default;

		const Type type = Type::Unknown;
	};

	class NANI_API MoveEvent : public Event
	{
	public:
		MoveEvent(const basic::PointF& oldPos_, const basic::PointF& newPos_);

		const basic::PointF oldPos;
		const basic::PointF newPos;
	};

	class NANI_API ResizeEvent : public Event
	{
	public:
		ResizeEvent(const basic::SizeF& oldSize_, const basic::SizeF& newSize_);

		const basic::SizeF oldSize;
		const basic::SizeF newSize;
	};

	class NANI_API PaintEvent : public Event
	{
	public:
		PaintEvent(const basic::RectF& dirtyRect_);
		~PaintEvent() = default;

		const basic::RectF dirtyRect;
	};

	class NANI_API MouseEvent : public Event
	{
	public:
		MouseEvent(Type type_, const basic::PointF& pos_, const basic::PointF& globalPos_);
		~MouseEvent() = default;

		basic::PointF pos;
		basic::PointF globalPos;
	};

	class NANI_API MouseMoveEvent : public MouseEvent
	{
	public:
		MouseMoveEvent(const basic::PointF& pos_, const basic::PointF& globalPos_);
		~MouseMoveEvent() = default;
	};

	enum class MouseButton : unsigned int
	{
		Unknown,
		Left,
		Middle,
		Right
	};

	enum class Modifier : unsigned int
	{
		None		= 0,
		Shift		= 1,
		Alt			= 1 << 1,
		Ctrl		= 1 << 2,
		CapsLock	= 1 << 3,
		NumLock		= 1 << 4,
		Super		= 1 << 5
	};

	inline Modifier operator|(Modifier lhs, Modifier rhs)
	{
		return static_cast<Modifier>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
	}

	inline Modifier operator&(Modifier lhs, Modifier rhs)
	{
		return static_cast<Modifier>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
	}

	class NANI_API MouseButtonEvent : public MouseEvent
	{
	public:
		MouseButtonEvent(Type type_, MouseButton button_, const basic::PointF& pos_, const basic::PointF& globalPos_, Modifier modifier_);
		~MouseButtonEvent() = default;

		const MouseButton button = MouseButton::Unknown;
		const Modifier modifier = Modifier::None;
	};

	class NANI_API MousePressEvent : public MouseButtonEvent
	{
	public:
		MousePressEvent(MouseButton button_, const basic::PointF& pos_, const basic::PointF& globalPos_, Modifier modifier_ = Modifier::None);
		~MousePressEvent() = default;
	};

	class NANI_API MouseReleaseEvent : public MouseButtonEvent
	{
	public:
		MouseReleaseEvent(MouseButton button_, const basic::PointF& pos_, const basic::PointF& globalPos_, Modifier modifier_ = Modifier::None);
		~MouseReleaseEvent() = default;
	};

	class NANI_API WheelEvent : public Event
	{
	public:
		WheelEvent(double deltaX_, double deltaY_);
		~WheelEvent() = default;

		const double deltaX = 0.0f;
		const double deltaY = 0.0f;
	};
}

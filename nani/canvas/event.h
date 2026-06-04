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
	
	enum class Key : int
	{
		Unknown = -1,

		Space = 32,
		Apostrophe = 39,      /* ' */
		Comma = 44,           /* , */
		Minus = 45,           /* - */
		Period = 46,          /* . */
		Slash = 47,           /* / */
		Key_0 = 48,
		Key_1 = 49,
		Key_2 = 50,
		Key_3 = 51,
		Key_4 = 52,
		Key_5 = 53,
		Key_6 = 54,
		Key_7 = 55,
		Key_8 = 56,
		Key_9 = 57,
		Semicolon = 59,       /* ; */
		Equal = 61,           /* = */
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,
		LeftBracket = 91,    /* [ */
		Backslash = 92,      /* \ */
		RightBracket = 93,   /* ] */
		GraveAccent = 96,    /* ` */
		World1 = 161,        /* non - US #1 */
		World2 = 162,        /* non - US #2 */

		// Function keys
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		// Keypad keys
		KeyPad_0 = 320,
		KeyPad_1 = 321,
		KeyPad_2 = 322,
		KeyPad_3 = 323,
		KeyPad_4 = 324,
		KeyPad_5 = 325,
		KeyPad_6 = 326,
		KeyPad_7 = 327,
		KeyPad_8 = 328,
		KeyPad_9 = 329,
		KeyPad_Decimal = 330,
		KeyPad_Divide = 331,
		KeyPad_Multiply = 332,
		KeyPad_Subtract = 333,
		KeyPad_Add = 334,
		KeyPad_Enter = 335,
		KeyPad_Equal = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348,

		Last = Menu
	};

	class NANI_API KeyEvent : public Event
	{
	public:
		KeyEvent(Type type_, Key key_, Modifier modifier_);
		~KeyEvent() = default;

	public:
		const Key key = Key::Unknown;
		const Modifier modifier = Modifier::None;
	};

	class NANI_API KeyPressEvent : public KeyEvent
	{
	public:
		KeyPressEvent(Key key_, Modifier modifier_);
		~KeyPressEvent() = default;
	};

	class NANI_API KeyReleaseEvent : public KeyEvent
	{
		KeyReleaseEvent(Key key_, Modifier modifier_);
		~KeyReleaseEvent() = default;
	};
}

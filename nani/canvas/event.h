#pragma once
#include "defs.h"
#include "basic/pointf.h"
#include "basic/sizef.h"

namespace nani::canvas
{
	class Event
	{
	public:
		enum class Type
		{
			Unknown,

			//Applicaton
			AboutToQuit,
			Quit,

			//IO
			MouseMove,
			MousePress,
			MouseRelease,

			KeyPress,
			KeyRelese,

			Show,
			Hide,
			Close,
			Move,
			Resize,
			FocusIn,
			FocusOut
		};

	public:
		Event(Type type_);
		~Event();

		const Type type = Type::Unknown;
	};

	class MoveEvent : public Event
	{
	public:
		MoveEvent(const basic::PointF& oldPos, const basic::PointF& newPos);
		~MoveEvent();

	public:
		const basic::PointF OldPosition() const;
		const basic::PointF NewPosiiton() const;

	private:
		basic::PointF m_oldPos;
		basic::PointF m_newPos;
	};

	class ResizeEvent : public Event
	{
	public:
		ResizeEvent(const basic::SizeF& oldSize, const basic::SizeF& newSize);
		~ResizeEvent();

	public:
		const basic::SizeF OldSize() const;
		const basic::SizeF NewSize() const;

	private:
		basic::SizeF m_oldSize;
		basic::SizeF m_newSize;
	};
}

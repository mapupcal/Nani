#include "event.h"
namespace nani::canvas
{
	Event::Event(Type type_)
		: type(type_)
	{

	}

	Event::~Event()
	{

	}

	MoveEvent::MoveEvent(const basic::PointF& oldPos, const basic::PointF& newPos)
		: Event(Type::Move)
		, m_oldPos(oldPos)
		, m_newPos(newPos)
	{

	}

	MoveEvent::~MoveEvent()
	{

	}

	const basic::PointF MoveEvent::OldPosition() const
	{
		return m_oldPos;
	}

	const basic::PointF MoveEvent::NewPosiiton() const
	{
		return m_newPos;
	}

	ResizeEvent::ResizeEvent(const basic::SizeF& oldSize, const basic::SizeF& newSize)
		: Event(Type::Resize)
		, m_oldSize(oldSize)
		, m_newSize(newSize)
	{

	}

	ResizeEvent::~ResizeEvent()
	{

	}

	const basic::SizeF ResizeEvent::OldSize() const
	{
		return m_oldSize;
	}

	const basic::SizeF ResizeEvent::NewSize() const
	{
		return m_newSize;
	}

	PaintEvent::PaintEvent(const basic::RectF& dirtyRect)
		: Event(Event::Type::Paint)
		, m_dirtyRect(dirtyRect)
	{

	}

	PaintEvent::~PaintEvent()
	{

	}

	const basic::RectF PaintEvent::DirtyRect() const
	{
		return m_dirtyRect;
	}
}


#include "cursor.h"
#ifdef NANI_OS_WIN
#include <Windows.h>
#endif
using namespace nani::canvas::basic;

namespace nani::canvas
{
	Cursor::Cursor()
	{

	}

	Cursor::~Cursor()
	{

	}

	const PointF Cursor::Pos()
	{
#ifdef NANI_OS_WIN
		POINT pos;
		::GetCursorPos(&pos);
		return PointF(pos.x, pos.y);
#else
		NANI_ASSERT(false);
		NANI_MESSAGE("Not Implement!")
#endif
	}

	void Cursor::SetPos(const basic::PointF& pos)
	{
#ifdef NANI_OS_WIN
		::SetCursorPos((int)pos.x, (int)pos.y);
#else
		NANI_ASSERT(false);
		NANI_MESSAGE("Not Implement!")
#endif
	}
}
#include "cursor.h"
#include "internal/platform.h"
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
		return internal::Platform::GetCursorPos();
	}

	void Cursor::SetPos(const basic::PointF& pos)
	{
		internal::Platform::SetCursorPos(pos);
	}
}
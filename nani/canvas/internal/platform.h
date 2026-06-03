#pragma once
#include "defs.h"
#include "basic/pointf.h"

namespace nani::canvas::internal
{
	struct Platform
	{
		static const basic::PointF GetCursorPos();
		static void SetCursorPos(const basic::PointF& pos);
	};
}
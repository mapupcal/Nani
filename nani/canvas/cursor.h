#pragma once
#include "defs.h"
#include "basic/pointf.h"
namespace nani::canvas
{
	class NANI_API Cursor
	{
	public:
		Cursor();
		~Cursor();

	public:
		static const basic::PointF Pos();
		static void SetPos(const basic::PointF& pos);
	};
}
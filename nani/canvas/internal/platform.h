#pragma once
#include "defs.h"
#include "basic/pointf.h"
#include "basic/color.h"

struct GLFWwindow;
namespace nani::canvas::internal
{
	struct Platform
	{
		static const basic::PointF GetCursorPos();
		static void SetCursorPos(const basic::PointF& pos);
		static void MakeToolWindow(GLFWwindow* window, bool bTool);
		static void MakeTruncatedPassThroughWindow(GLFWwindow* window, bool bPassThrough, const basic::Color& truncatedColor);
	};
}
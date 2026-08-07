#pragma once
#include "defs.h"
#include "basic/pointf.h"
#include "basic/rectf.h"

struct GLFWwindow;
namespace nani::canvas::internal
{
	struct Platform
	{
		// Call before glfwInit / any HWND. Idempotent; safe from static init.
		static void EnableProcessDpiAwareness();

		static const basic::PointF GetCursorPos();
		static void SetCursorPos(const basic::PointF& pos);
		static void MakeToolWindow(GLFWwindow* window, bool bTool);
		static void MakeResizableWindow(GLFWwindow* window, bool bResizable);
		static void MakeTruncatedPassThroughWindow(GLFWwindow* window, bool bPassThrough);
		static void SyncCustomWndProc(GLFWwindow* window);
		static void EnsureImeHook(GLFWwindow* window);
		static void SetImeCompositionRect(GLFWwindow* window, const basic::RectF& clientCaretRect);
	};
}
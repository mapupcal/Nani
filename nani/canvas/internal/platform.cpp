#include "platform.h"
#if defined(NANI_OS_WIN)
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

using namespace nani::canvas::basic;
namespace nani::canvas::internal
{
	const PointF Platform::GetCursorPos()
	{
#ifdef NANI_OS_WIN
		POINT pos;
		::GetCursorPos(&pos);
		return PointF(pos.x, pos.y);
#else
		NANI_ASSERT(false);
		NANI_MESSAGE("Not Implement!");
		return PointF();
#endif
	}

	void Platform::SetCursorPos(const PointF& pos)
	{
#ifdef NANI_OS_WIN
		::SetCursorPos((int)pos.x, (int)pos.y);
#else
		NANI_ASSERT(false);
		NANI_MESSAGE("Not Implement!")
#endif
	}

	void Platform::MakeToolWindow(GLFWwindow* window, bool bTool)
	{
#ifdef NANI_OS_WIN
		HWND hwnd = glfwGetWin32Window(window);
		LONG_PTR exStyle = ::GetWindowLongPtr(hwnd, GWL_EXSTYLE);
		if (bTool)
		{
			exStyle |= WS_EX_TOOLWINDOW;
			exStyle &= ~WS_EX_APPWINDOW;
		}
		else
		{
			exStyle &= ~WS_EX_TOOLWINDOW;
			exStyle |= WS_EX_APPWINDOW;
		}
		::SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
#else
		NANI_ASSERT(false);
		NANI_MESSAGE("Not Implement!")
#endif
	}
}
#include "platform.h"
#include "window_p.h"
#if defined(NANI_OS_WIN)
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <map>

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

#ifdef NANI_OS_WIN
	namespace
	{
		static std::map<HWND, WindowPrivate*> g_hwnd2ResizableWindowPrivates;
		LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		using _WndProcType = decltype(CustomWndProc);

		LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			WindowPrivate* pImpl = g_hwnd2ResizableWindowPrivates.at(hwnd);
			switch (msg)
			{
			case WM_NCCALCSIZE:
			{
				if (wParam == TRUE)
					return 0;
				break;
			}
			case WM_NCHITTEST:
			{
				POINT pt = { LOWORD(lParam), HIWORD(lParam) };
				::ScreenToClient(hwnd, &pt);
				int width = pImpl->size.width;
				int height = pImpl->size.height;
				int borderWidth = pImpl->borderWidth;
				int radius = pImpl->radius;

				if (pt.x < radius && pt.y < radius)
				{
					int dx = pt.x - radius;
					int dy = pt.y - radius;
					if (dx * dx + dy * dy > radius * radius)
						return HTNOWHERE;
					else
						return HTTOPLEFT;
				}

				if (pt.x > width - radius && pt.y < radius)
				{
					int dx = pt.x - (width - radius);
					int dy = pt.y - radius;
					if (dx * dx + dy * dy > radius * radius)
						return HTNOWHERE;
					else
						return HTTOPRIGHT;
				}

				if (pt.x < radius && pt.y > height - radius)
				{
					int dx = pt.x - radius;
					int dy = pt.y - (height - radius);
					if (dx * dx + dy * dy > radius * radius)
						return HTNOWHERE;
					else
						return HTBOTTOMLEFT;
				}

				if (pt.x > width - radius && pt.y > height - radius)
				{
					int dx = pt.x - (width - radius);
					int dy = pt.y - (height - radius);
					if (dx * dx + dy * dy > radius * radius)
						return HTNOWHERE;
					else
						return HTBOTTOMRIGHT;
				}

				if (pt.y < borderWidth)
				{
					if (pt.x < borderWidth) return HTTOPLEFT;
					if (pt.x > width - borderWidth) return HTTOPRIGHT;
					return HTTOP;
				}

				if (pt.y > height - borderWidth)
				{
					if (pt.x < borderWidth)
						return HTBOTTOMLEFT;
					if (pt.x > width - borderWidth)
						return HTBOTTOMRIGHT;
					return HTBOTTOM;
				}

				if (pt.x < borderWidth) 
					return HTLEFT;
				if (pt.x > width - borderWidth)
					return HTRIGHT;

				return HTCLIENT;
			}
			case WM_DESTROY:
				g_hwnd2ResizableWindowPrivates.erase(hwnd);
				::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)(_WndProcType*)(pImpl->_originalWndProc));
				break;
			}
			return ::CallWindowProc((_WndProcType*)(pImpl->_originalWndProc), hwnd, msg, wParam, lParam);
		}
	}
#endif
	void Platform::MakeResizableWindow(GLFWwindow* window, bool bResizable)
	{
#ifdef NANI_OS_WIN
		HWND hwnd = glfwGetWin32Window(window);
		WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
		if (bResizable)
		{
			if (!pImpl->_originalWndProc)
			{
				SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_THICKFRAME);
				g_hwnd2ResizableWindowPrivates.insert({ hwnd, pImpl });
				pImpl->_originalWndProc = (void*)::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)CustomWndProc);
			}
		}
		else
		{
			if (pImpl->_originalWndProc)
			{
				SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_THICKFRAME);
				::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pImpl->_originalWndProc);
				pImpl->_originalWndProc = nullptr;
				g_hwnd2ResizableWindowPrivates.erase(hwnd);
			}
		}
#else
		NANI_ASSERT(false);
		NANI_MESSAGE("Not Implement!")
#endif
	}

	void Platform::MakeTruncatedPassThroughWindow(GLFWwindow* window, bool bPassThrough)
	{
#ifdef NANI_OS_WIN
		HWND hwnd = glfwGetWin32Window(window);
		if (bPassThrough)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			Color truncatedColor = pImpl->truncatedColor;
			LONG style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
			style |= WS_EX_LAYERED;
			::SetWindowLong(hwnd, GWL_EXSTYLE, style);
			::SetLayeredWindowAttributes(hwnd, RGB(truncatedColor.r, truncatedColor.g, truncatedColor.b), 0, LWA_COLORKEY);
		}
		else
		{
			LONG style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
			style &= ~WS_EX_LAYERED;
			::SetWindowLong(hwnd, GWL_EXSTYLE, style);
		}
#else
		NANI_ASSERT(false);
		NANI_MESSAGE("Not Implement!")
#endif
	}
}
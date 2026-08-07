#include "platform.h"
#include "window_p.h"
#include "../visuals/view.h"
#include "../window.h"
#include "../events/event.h"

#if defined(NANI_OS_WIN)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <windowsx.h>
#include <imm.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <cmath>
#include <map>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

using namespace nani::canvas::basic;
namespace nani::canvas::internal
{
	void Platform::EnableProcessDpiAwareness()
	{
#ifdef NANI_OS_WIN
		// Must run before any HWND / glfwInit so the process is Per-Monitor V2 aware.
		using SetProcessDpiAwarenessContextFn = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
		HMODULE user32 = ::GetModuleHandleW(L"user32.dll");
		if (user32)
		{
			auto setCtx = reinterpret_cast<SetProcessDpiAwarenessContextFn>(
				::GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
			if (setCtx && setCtx(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
				return;
		}

		// Older Windows: fall back to per-monitor v1 via Shcore (value = 2).
		using SetProcessDpiAwarenessFn = HRESULT(WINAPI*)(int);
		if (HMODULE shcore = ::LoadLibraryW(L"Shcore.dll"))
		{
			auto setDpiAwareness = reinterpret_cast<SetProcessDpiAwarenessFn>(
				::GetProcAddress(shcore, "SetProcessDpiAwareness"));
			if (setDpiAwareness)
				setDpiAwareness(2);
			::FreeLibrary(shcore);
		}
#else
		// Non-Windows: nothing to do; content scale comes from GLFW.
#endif
	}

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
		if (!window)
			return;
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
		static std::map<HWND, WindowPrivate*> g_hwnd2WindowPrivates;
		LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		using _WndProcType = decltype(CustomWndProc);

		void EnsureCustomWndProc(HWND hwnd, WindowPrivate* pImpl)
		{
			if (!pImpl || pImpl->_originalWndProc)
				return;
			g_hwnd2WindowPrivates.insert({ hwnd, pImpl });
			pImpl->_originalWndProc = (void*)::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)CustomWndProc);
		}

		void ReleaseCustomWndProcIfUnused(HWND hwnd, WindowPrivate* pImpl)
		{
			if (!pImpl || !pImpl->_originalWndProc)
				return;
			if (pImpl->resizableEnabled || pImpl->windowDragEnabled || pImpl->imeHookEnabled)
				return;

			::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pImpl->_originalWndProc);
			pImpl->_originalWndProc = nullptr;
			g_hwnd2WindowPrivates.erase(hwnd);
		}

		std::u8string WideToUtf8(std::wstring_view wide)
		{
			if (wide.empty())
				return {};

			const int bytes = ::WideCharToMultiByte(
				CP_UTF8,
				0,
				wide.data(),
				static_cast<int>(wide.size()),
				nullptr,
				0,
				nullptr,
				nullptr);
			if (bytes <= 0)
				return {};

			std::string utf8(static_cast<size_t>(bytes), '\0');
			::WideCharToMultiByte(
				CP_UTF8,
				0,
				wide.data(),
				static_cast<int>(wide.size()),
				utf8.data(),
				bytes,
				nullptr,
				nullptr);
			return std::u8string(utf8.begin(), utf8.end());
		}

		std::u8string ReadImeString(HIMC himc, DWORD index)
		{
			if (!himc)
				return {};

			const LONG bytes = ::ImmGetCompositionStringW(himc, index, nullptr, 0);
			if (bytes <= 0)
				return {};

			std::wstring buffer(static_cast<size_t>(bytes) / sizeof(wchar_t), L'\0');
			::ImmGetCompositionStringW(himc, index, buffer.data(), static_cast<DWORD>(bytes));
			return WideToUtf8(buffer);
		}

		void FireImeCompositionUpdate(WindowPrivate* pImpl, HIMC himc)
		{
			if (!pImpl || !pImpl->window || !himc)
				return;

			events::ImeCompositionUpdateEvent event(ReadImeString(himc, GCS_COMPSTR));
			pImpl->window->FireEvent(&event);
		}

		// Returns true when RESULTSTR was committed by the app (caller should
		// return 0 so DefWindowProc/GLFW do not also synthesize WM_CHARs).
		bool FireImeCompositionCommit(WindowPrivate* pImpl, HIMC himc)
		{
			if (!pImpl || !pImpl->window || !himc)
				return false;

			const std::u8string text = ReadImeString(himc, GCS_RESULTSTR);
			if (text.empty())
				return false;

			events::ImeCompositionCommitEvent event(text);
			pImpl->window->FireEvent(&event);
			return true;
		}

		LRESULT HitTestResizable(WindowPrivate* pImpl, POINT pt)
		{
			const PointF scale = pImpl->LogicalToPlatformScale();

			const int width = static_cast<int>(pImpl->platformWindowSize.width > 0.0f
				? pImpl->platformWindowSize.width
				: pImpl->size.width);
			const int height = static_cast<int>(pImpl->platformWindowSize.height > 0.0f
				? pImpl->platformWindowSize.height
				: pImpl->size.height);
			const int borderWidth = static_cast<int>(std::lround(pImpl->borderWidth * scale.x));
			const int radius = static_cast<int>(std::lround(pImpl->radius * ((scale.x + scale.y) * 0.5f)));

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

		LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			auto iter = g_hwnd2WindowPrivates.find(hwnd);
			if (iter == g_hwnd2WindowPrivates.end())
				return ::DefWindowProc(hwnd, msg, wParam, lParam);

			WindowPrivate* pImpl = iter->second;
			if (!pImpl || !pImpl->_originalWndProc)
				return ::DefWindowProc(hwnd, msg, wParam, lParam);

			switch (msg)
			{
			case WM_IME_STARTCOMPOSITION:
			{
				if (pImpl->window)
				{
					events::ImeCompositionStartEvent event;
					pImpl->window->FireEvent(&event);
				}
				// Anchor before Imm places the candidate list. Do not handle
				// WM_IME_NOTIFY->ImmSet* (that re-enters and freezes the UI).
				// Clear valid so SetImeCompositionRect re-applies even if unchanged.
				if (pImpl->imeCaretRectValid)
				{
					const basic::RectF rect = pImpl->imeCaretRect;
					pImpl->imeCaretRectValid = false;
					Platform::SetImeCompositionRect(pImpl->glfwWindow, rect);
				}
				// Preedit is drawn by InputTextVisual; do not create the default
				// Imm composition window (would show a duplicate string).
				return 0;
			}
			case WM_IME_COMPOSITION:
			{
				const bool hasCompStr = (lParam & GCS_COMPSTR) != 0;
				const bool hasResultStr = (lParam & GCS_RESULTSTR) != 0;
				bool committedResult = false;
				if (pImpl->window && (hasCompStr || hasResultStr))
				{
					HIMC himc = ::ImmGetContext(hwnd);
					if (himc)
					{
						// Commit finalized text ourselves. Win+. emoji panel and
						// many IMEs put the payload in GCS_RESULTSTR; only firing
						// End clears preedit and drops the characters when no
						// subsequent WM_CHAR arrives.
						if (hasResultStr)
							committedResult = FireImeCompositionCommit(pImpl, himc);
						if (hasCompStr && !committedResult)
							FireImeCompositionUpdate(pImpl, himc);
						::ImmReleaseContext(hwnd, himc);
					}
				}
				if (pImpl->window && hasResultStr && !committedResult)
				{
					events::ImeCompositionEndEvent event;
					pImpl->window->FireEvent(&event);
				}
				// Suppress default Imm painting / duplicate WM_CHAR after we
				// already inserted RESULTSTR (or when only updating preedit).
				if ((hasCompStr && !hasResultStr) || committedResult)
					return 0;
				break;
			}
			case WM_IME_ENDCOMPOSITION:
			{
				if (pImpl->window)
				{
					events::ImeCompositionEndEvent event;
					pImpl->window->FireEvent(&event);
				}
				break;
			}
			case WM_NCCALCSIZE:
			{
				if (wParam == TRUE)
					return 0;
				break;
			}
			case WM_NCHITTEST:
			{
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(hwnd, &pt);

				const PointF localPt = pImpl->ScreenToLogical(pt.x, pt.y);
				visuals::View* view = pImpl->window ? pImpl->window->GetView() : nullptr;

				if (pImpl->resizableEnabled)
				{
					LRESULT borderHit = HitTestResizable(pImpl, pt);
					if (borderHit != HTCLIENT)
					{
						if (view)
							view->ClearHover();
						return borderHit;
					}
				}

				if (pImpl->windowDragEnabled && view && view->IsWindowDragAt(localPt))
				{
					view->UpdateHoverAt(localPt);
					return HTCAPTION;
				}

				return HTCLIENT;
			}
			case WM_DESTROY:
			{
				auto originalWndProc = pImpl->_originalWndProc;
				g_hwnd2WindowPrivates.erase(hwnd);
				::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)(_WndProcType*)(originalWndProc));
				pImpl->_originalWndProc = nullptr;
				pImpl->resizableEnabled = false;
				pImpl->windowDragEnabled = false;
				pImpl->imeHookEnabled = false;
				return ::CallWindowProc((_WndProcType*)(originalWndProc), hwnd, msg, wParam, lParam);
			}
			}
			return ::CallWindowProc((_WndProcType*)(pImpl->_originalWndProc), hwnd, msg, wParam, lParam);
		}

	}
#endif

	void Platform::SyncCustomWndProc(GLFWwindow* window)
	{
#ifdef NANI_OS_WIN
		if (!window)
			return;
		HWND hwnd = glfwGetWin32Window(window);
		WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
		if (!pImpl)
			return;

		if (pImpl->resizableEnabled || pImpl->windowDragEnabled || pImpl->imeHookEnabled)
			EnsureCustomWndProc(hwnd, pImpl);
		else
			ReleaseCustomWndProcIfUnused(hwnd, pImpl);
#else
		(void)window;
#endif
	}

	void Platform::EnsureImeHook(GLFWwindow* window)
	{
#ifdef NANI_OS_WIN
		if (!window)
			return;
		WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
		if (!pImpl)
			return;

		pImpl->imeHookEnabled = true;
		SyncCustomWndProc(window);
#else
		(void)window;
#endif
	}

	void Platform::SetImeCompositionRect(GLFWwindow* window, const basic::RectF& clientCaretRect)
	{
#ifdef NANI_OS_WIN
		if (!window)
			return;

		WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
		if (!pImpl || pImpl->imeUpdatingForms)
			return;

		// ImmSet* can synthesize IMN_* notifications; ignore same-rect updates and
		// re-entrant calls so we never loop (that previously froze on focus).
		constexpr float kEps = 0.5f;
		if (pImpl->imeCaretRectValid &&
			std::abs(pImpl->imeCaretRect.left - clientCaretRect.left) < kEps &&
			std::abs(pImpl->imeCaretRect.top - clientCaretRect.top) < kEps &&
			std::abs(pImpl->imeCaretRect.right - clientCaretRect.right) < kEps &&
			std::abs(pImpl->imeCaretRect.bottom - clientCaretRect.bottom) < kEps)
		{
			return;
		}

		pImpl->imeCaretRect = clientCaretRect;
		pImpl->imeCaretRectValid = true;
		pImpl->imeUpdatingForms = true;

		HWND hwnd = glfwGetWin32Window(window);
		HIMC himc = ::ImmGetContext(hwnd);
		if (!himc)
		{
			pImpl->imeUpdatingForms = false;
			return;
		}

		// Caret rect is in logical client coordinates; Imm wants platform client pixels.
		const RectF platformRect = pImpl->LogicalToPlatformRect(clientCaretRect);
		const LONG left = static_cast<LONG>(std::lround(platformRect.left));
		const LONG top = static_cast<LONG>(std::lround(platformRect.top));
		const LONG right = clientCaretRect.right > clientCaretRect.left
			? static_cast<LONG>(std::lround(platformRect.right))
			: left + 1;
		const LONG bottom = clientCaretRect.bottom > clientCaretRect.top
			? static_cast<LONG>(std::lround(platformRect.bottom))
			: top + 1;

		// Qt-style anchor: composition point at preedit/caret top-left; candidate
		// excludes that rect so Imm parks the list against its bottom edge.
		// STARTCOMPOSITION returns 0, so Imm still must not paint a second preedit.
		COMPOSITIONFORM composition = {};
		composition.dwStyle = CFS_FORCE_POSITION;
		composition.ptCurrentPos.x = left;
		composition.ptCurrentPos.y = top;
		::ImmSetCompositionWindow(himc, &composition);

		CANDIDATEFORM candidate = {};
		candidate.dwIndex = 0;
		candidate.dwStyle = CFS_EXCLUDE;
		candidate.ptCurrentPos.x = left;
		candidate.ptCurrentPos.y = bottom;
		candidate.rcArea = { left, top, right, bottom };
		::ImmSetCandidateWindow(himc, &candidate);

		::ImmReleaseContext(hwnd, himc);
		pImpl->imeUpdatingForms = false;
#else
		(void)window;
		(void)clientCaretRect;
#endif
	}

	void Platform::MakeResizableWindow(GLFWwindow* window, bool bResizable)
	{
#ifdef NANI_OS_WIN
		if (!window)
			return;
		HWND hwnd = glfwGetWin32Window(window);
		WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
		if (!pImpl)
			return;

		pImpl->resizableEnabled = bResizable;
		LONG style = ::GetWindowLong(hwnd, GWL_STYLE);
		if (bResizable)
			style |= WS_THICKFRAME;
		else
			style &= ~WS_THICKFRAME;
		::SetWindowLong(hwnd, GWL_STYLE, style);

		SyncCustomWndProc(window);
#else
		NANI_ASSERT(false);
		NANI_MESSAGE("Not Implement!")
#endif
	}

	void Platform::MakeTruncatedPassThroughWindow(GLFWwindow* window, bool bPassThrough)
	{
#ifdef NANI_OS_WIN
		if (!window)
			return;
		HWND hwnd = glfwGetWin32Window(window);
		if (bPassThrough)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (!pImpl)
				return;
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

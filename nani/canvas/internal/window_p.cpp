#pragma once
#include "window_p.h"
#include "window.h"
#include "env_p.h"
#include "platform.h"
#include "skia_defs.h"
#include "skia_utils.h"
#include "events/event.h"
#include "../visuals/view.h"
#include "../visuals/visual.h"

#include <algorithm>
#include <cmath>

#include <include/gpu/ganesh/gl/GrGLInterface.h>
#include <include/gpu/ganesh/GrContextOptions.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/gpu/ganesh/gl/GrGLBackendSurface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/ganesh/GrBackendSurface.h>

using namespace nani::canvas::events;
using namespace nani::canvas::basic;

namespace nani::canvas::internal
{
	namespace
	{
		void FramebufferScaleForWindow(
			int winW, int winH, int fbW, int fbH, float& scaleX, float& scaleY)
		{
			scaleX = (winW > 0) ? (static_cast<float>(fbW) / static_cast<float>(winW)) : 1.0f;
			scaleY = (winH > 0) ? (static_cast<float>(fbH) / static_cast<float>(winH)) : 1.0f;
			if (scaleX <= 0.0f)
				scaleX = 1.0f;
			if (scaleY <= 0.0f)
				scaleY = 1.0f;
		}

		SizeF LogicalSizeFromPlatform(
			const SizeF& platform, const SizeF& framebuffer, float devicePixelRatio)
		{
			float fbRatioX = 1.0f;
			float fbRatioY = 1.0f;
			FramebufferScaleForWindow(
				static_cast<int>(platform.width),
				static_cast<int>(platform.height),
				static_cast<int>(framebuffer.width),
				static_cast<int>(framebuffer.height),
				fbRatioX,
				fbRatioY);

			const float dpr = (devicePixelRatio > 0.0f) ? devicePixelRatio : 1.0f;
			return SizeF(
				platform.width * fbRatioX / dpr,
				platform.height * fbRatioY / dpr);
		}

		struct PlatformSurfaceSnapshot
		{
			SizeF platform;
			SizeF framebuffer;
			float devicePixelRatio = 1.0f;
			SizeF logical;
		};

		bool CanHandlePlatformSurfaceEvent(const WindowPrivate& windowPrivate)
		{
			return windowPrivate.glfwWindow && !windowPrivate.syncingPlatformSize;
		}

		bool QueryPlatformSurfaceSnapshot(
			const WindowPrivate& windowPrivate, PlatformSurfaceSnapshot& out)
		{
			if (!windowPrivate.glfwWindow)
				return false;

			int winW = 0;
			int winH = 0;
			glfwGetWindowSize(windowPrivate.glfwWindow, &winW, &winH);
			if (winW <= 0 || winH <= 0)
				return false;

			float xScale = 1.0f;
			float yScale = 1.0f;
			glfwGetWindowContentScale(windowPrivate.glfwWindow, &xScale, &yScale);

			out.platform = SizeF(static_cast<float>(winW), static_cast<float>(winH));
			out.devicePixelRatio = (xScale > 0.0f) ? xScale : 1.0f;

			int fbW = 0;
			int fbH = 0;
			glfwGetFramebufferSize(windowPrivate.glfwWindow, &fbW, &fbH);
			if (fbW > 0 && fbH > 0)
			{
				out.framebuffer = SizeF(static_cast<float>(fbW), static_cast<float>(fbH));
				out.logical = LogicalSizeFromPlatform(
					out.platform, out.framebuffer, out.devicePixelRatio);
			}
			else
			{
				out.framebuffer = SizeF();
				out.logical = windowPrivate.size;
			}

			return true;
		}

		void NotifyLogicalSizeChanged(WindowPrivate& windowPrivate, const SizeF& oldLogicalSize)
		{
			if (oldLogicalSize != windowPrivate.size)
			{
				ResizeEvent event(oldLogicalSize, windowPrivate.size);
				windowPrivate.window->FireEvent(&event);
				return;
			}

			if (visuals::View* view = windowPrivate.window->GetView())
				view->MarkDirty();
		}

		void _OnGLFWWindowSizeChanged(GLFWwindow* window, int width, int height)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
				pImpl->OnGLFWWindowSizeChanged(width, height);
		}

		void _OnGLFWFramebufferSizeChanged(GLFWwindow* window, int width, int height)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
				pImpl->OnGLFWFramebufferSizeChanged(width, height);
		}

		void _OnGLFWWindowContentScaleChanged(GLFWwindow* window, float xScale, float yScale)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
				pImpl->OnGLFWWindowContentScaleChanged(xScale, yScale);
		}

		void _OnGLFWWindowPositionChanged(GLFWwindow* window, int xpos, int ypos)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
				pImpl->OnGLFWWindowPositionChanged(xpos, ypos);
		}

		void _OnGLFWWindowClose(GLFWwindow* window)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
				pImpl->OnGLFWWindowClose();
		}

		void _OnGLFWWindowFocusChanged(GLFWwindow* window, int focused)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
				pImpl->OnGLFWWindowFocusChanged(focused == GLFW_TRUE);
		}

		void _OnGLFWWindowMousePos(GLFWwindow* window, double xpos, double ypos)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
				pImpl->OnGLFWWindowMouseMove(xpos, ypos);
		}

		void _OnGLFWWindowMouseEnter(GLFWwindow* window, int entered)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
				pImpl->OnGLFWWindowMouseEnter(entered == GLFW_TRUE);
		}

		void _OnGLFWWindowMouseButton(GLFWwindow* window, int btn, int action, int mods)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
			{
				double xPos = 0.0f;
				double yPos = 0.0f;
				glfwGetCursorPos(window, &xPos, &yPos);

				MouseButton button = MouseButton::Unknown;
				if (btn == GLFW_MOUSE_BUTTON_LEFT)
					button = MouseButton::Left;
				else if (btn == GLFW_MOUSE_BUTTON_MIDDLE)
					button = MouseButton::Middle;
				else if(btn == GLFW_MOUSE_BUTTON_RIGHT)
					button = MouseButton::Right;
				else
				{
					NANI_ASSERT(false);
					NANI_MESSAGE("Not support mouse button type");
				}

				Modifier modifier = Modifier::None;
				if (mods & GLFW_MOD_SHIFT)
					modifier = modifier | Modifier::Shift;
				if (mods & GLFW_MOD_ALT)
					modifier = modifier | Modifier::Alt;
				if (mods & GLFW_MOD_CONTROL)
					modifier = modifier | Modifier::Ctrl;
				if (mods & GLFW_MOD_CAPS_LOCK)
					modifier = modifier | Modifier::CapsLock;
				if (mods & GLFW_MOD_NUM_LOCK)
					modifier = modifier | Modifier::NumLock;
				if (mods & GLFW_MOD_SUPER)
					modifier = modifier | Modifier::Super;

				pImpl->OnGLFWWindowMouseButton(xPos, yPos, button, action == GLFW_PRESS, modifier);
			}
		}

		void _OnGLFWWindowWheelScroll(GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
				pImpl->OnGLFWWindowWheelScroll(xOffset, yOffset);
		}

		void _OnGLFWWindowKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
			{
				events::Key key_ = static_cast<events::Key>(key);
				Modifier modifier = Modifier::None;
				if (mods & GLFW_MOD_SHIFT)
					modifier = modifier | Modifier::Shift;
				if (mods & GLFW_MOD_ALT)
					modifier = modifier | Modifier::Alt;
				if (mods & GLFW_MOD_CONTROL)
					modifier = modifier | Modifier::Ctrl;
				if (mods & GLFW_MOD_CAPS_LOCK)
					modifier = modifier | Modifier::CapsLock;
				if (mods & GLFW_MOD_NUM_LOCK)
					modifier = modifier | Modifier::NumLock;
				if (mods & GLFW_MOD_SUPER)
					modifier = modifier | Modifier::Super;
				// GLFW_REPEAT must be treated as press so held keys keep firing
				// KeyPress (caret move / backspace / delete).
				const bool bPress = (action == GLFW_PRESS || action == GLFW_REPEAT);
				pImpl->OnGLFWWindowKeyEvent(key_, scancode, bPress, modifier);
			}
		}

		void _OnGLFWWindowChar(GLFWwindow* window, unsigned int codepoint)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
				pImpl->OnGLFWWindowChar(codepoint);
		}

		void _SetWindowHints(GLFWwindow* window, Window::Hint hints)
		{
			glfwSetWindowAttrib(window, GLFW_FLOATING, !!(hints & Window::Top));
			internal::Platform::MakeResizableWindow(window, !!(hints & Window::Resizable));
			internal::Platform::MakeTruncatedPassThroughWindow(window, !!(hints & Window::TruncatedPassThrough));
			internal::Platform::MakeToolWindow(window, !!(hints & Window::Tool));
		}
	}

	WindowPrivate::WindowPrivate(Window* window_)
		: window(window_)
	{

	}

	WindowPrivate::~WindowPrivate()
	{
		Close();
	}

	bool WindowPrivate::IsVisible() const
	{
		if (!glfwWindow)
			return false;
		return glfwGetWindowAttrib(glfwWindow, GLFW_VISIBLE) == GLFW_TRUE;
	}

	void WindowPrivate::Show()
	{
		if (!glfwWindow)
		{
			InitializeGLFWWindow();
			InitializeSkiaContext();

			Event event(Type::Show);
			window->FireEvent(&event);
			Repaint();
			return;
		}
		if (!glfwWindow)
			return;

		if (IsVisible())
			return;

		glfwShowWindow(glfwWindow);

		Event event(Type::Show);
		window->FireEvent(&event);
		Repaint();
	}

	void WindowPrivate::Hide()
	{
		if (!glfwWindow)
			return;

		glfwHideWindow(glfwWindow);

		Event event(Type::Hide);
		window->FireEvent(&event);
	}

	void WindowPrivate::Close()
	{
		if (glfwWindow != nullptr)
		{
			EnvPrivate::Instance()->UnRegisterWindow(glfwWindow);
			glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
			skiaGlContext.reset();
			skiaSurface.reset();
			glfwDestroyWindow(glfwWindow);
		}

		glfwWindow = nullptr;
	}

	void WindowPrivate::Move(const basic::PointF& pos)
	{
		if (!glfwWindow)
			return;
		if (pos == this->pos)
			return;

		glfwSetWindowPos(glfwWindow, pos.x, pos.y);
	}

	void WindowPrivate::Resize(const basic::SizeF& size_)
	{
		if (size_ == this->size && glfwWindow)
			return;

		const SizeF oldSize = size;
		size = size_;
		if (!glfwWindow)
		{
			framebufferSize = size;
			return;
		}

		SyncDpiSurface(true);
		if (oldSize != size)
		{
			ResizeEvent event(oldSize, size);
			window->FireEvent(&event);
		}
		Repaint();
	}

	void WindowPrivate::SetRadius(basic::single fRadius)
	{
		radius = fRadius;
		Repaint();
	}

	void WindowPrivate::SetBorderWidth(basic::single fWidth)
	{
		borderWidth = fWidth;
		Repaint();
	}

	void WindowPrivate::SetBorderColor(const basic::Color & color)
	{
		borderColor = color;
		Repaint();
	}

	void WindowPrivate::SetBackgroundColor(const basic::Color& color)
	{
		backgroundColor = color;
		Repaint();
	}

	void WindowPrivate::SetTitle(const std::string_view & title_)
	{
		title = title_;
		if(glfwWindow)
			glfwSetWindowTitle(glfwWindow, title.c_str());
	}

	void WindowPrivate::SetHints(unsigned int hints_)
	{
		hints = hints_;
		if (glfwWindow)
			_SetWindowHints(glfwWindow,(Window::Hint)hints);
		Repaint();
	}

	void WindowPrivate::SetTruncatedColor(const basic::Color& color)
	{
		truncatedColor = color;
		SetHints(hints);
	}

	void WindowPrivate::OnGLFWWindowSizeChanged(int /*width*/, int /*height*/)
	{
		if (!CanHandlePlatformSurfaceEvent(*this))
			return;

		// Repaint from the framebuffer callback once the physical surface matches.
		surfaceResizeFallbackPending = true;
	}

	void WindowPrivate::OnGLFWFramebufferSizeChanged(int width, int height)
	{
		if (!CanHandlePlatformSurfaceEvent(*this))
			return;
		if (width <= 0 || height <= 0)
			return;

		surfaceResizeFallbackPending = false;
		CommitSurfaceResize();
	}

	void WindowPrivate::CommitSurfaceResize()
	{
		if (!CanHandlePlatformSurfaceEvent(*this))
			return;

		PlatformSurfaceSnapshot snapshot;
		if (!QueryPlatformSurfaceSnapshot(*this, snapshot) || !snapshot.framebuffer.IsValid())
			return;

		if (snapshot.platform == platformWindowSize &&
			snapshot.framebuffer == framebufferSize &&
			snapshot.logical == size)
		{
			return;
		}

		devicePixelRatio = snapshot.devicePixelRatio;
		platformWindowSize = snapshot.platform;
		framebufferSize = snapshot.framebuffer;

		const SizeF oldSize = size;
		size = snapshot.logical;

		BindSkiaSurface();
		NotifyLogicalSizeChanged(*this, oldSize);
		Repaint();
	}

	void WindowPrivate::OnGLFWWindowContentScaleChanged(float xScale, float /*yScale*/)
	{
		if (!glfwWindow)
			return;

		devicePixelRatio = (xScale > 0.0f) ? xScale : 1.0f;
		// Keep logical size; grow/shrink the platform window so FB covers logical*dpr.
		SyncDpiSurface(true);
		if (visuals::View* view = window->GetView())
			view->MarkDirty();
		Repaint();
	}

	void WindowPrivate::OnGLFWWindowPositionChanged(int xPos, int yPos)
	{
		PointF oldPos = pos;
		pos = PointF(xPos, yPos);

		MoveEvent event(oldPos, pos);
		window->FireEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowFocusChanged(bool bFocus)
	{
		Event event(bFocus ? Type::FocusIn : Type::FocusOut);
		window->FireEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowClose()
	{
		EnvPrivate::Instance()->UnRegisterWindow(glfwWindow);
		glfwWindow = nullptr;
		skiaGlContext.reset();
		skiaSurface.reset();

		Event event(Type::Close);
		window->FireEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowMouseEnter(bool bEnter)
	{
		if (!bEnter)
		{
			if (visuals::View* view = window->GetView())
				view->ClearHover();
		}
		Type type = bEnter ? Type::Enter : Type::Leave;
		Event event(type);
		window->FireEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowMouseMove(double xPos, double yPos)
	{
		PointF pos_ = ScreenToLogical(xPos, yPos);
		PointF globalPos = pos_ + pos;
		MouseMoveEvent event(pos_, globalPos);
		window->FireEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowMouseButton(double xPos, double yPos, MouseButton button, bool bPress, Modifier modifier)
	{
		PointF pos_ = ScreenToLogical(xPos, yPos);
		PointF globalPos = pos_ + pos;
		if (bPress)
		{
			MousePressEvent event(button, pos_, globalPos, modifier);
			window->FireEvent(&event);
		}
		else
		{
			MouseReleaseEvent event(button, pos_, globalPos, modifier);
			window->FireEvent(&event);
		}
	}

	void WindowPrivate::OnGLFWWindowWheelScroll(double xDelta, double yDelta)
	{
		double xPos = 0.0;
		double yPos = 0.0;
		if (glfwWindow)
			glfwGetCursorPos(glfwWindow, &xPos, &yPos);

		PointF pos_ = ScreenToLogical(xPos, yPos);
		PointF globalPos = pos_ + pos;
		WheelEvent event(pos_, globalPos, xDelta, yDelta);
		window->FireEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowKeyEvent(events::Key key, int scancode, bool bPress, events::Modifier modifier)
	{
		if (bPress)
		{
			KeyPressEvent event(key, modifier, scancode);
			window->FireEvent(&event);
		}
		else
		{
			KeyReleaseEvent event(key, modifier, scancode);
			window->FireEvent(&event);
		}
	}

	void WindowPrivate::OnGLFWWindowChar(unsigned int codepoint)
	{
		CharEvent event(static_cast<char32_t>(codepoint));
		window->FireEvent(&event);
	}

	void WindowPrivate::SyncWindowDrag()
	{
		bool enabled = false;
		if (visuals::View* view = window ? window->GetView() : nullptr)
		{
			if (visuals::Visual* root = view->Visual())
				enabled = root->HasWindowDragDescendant();
		}

		if (windowDragEnabled == enabled)
			return;

		windowDragEnabled = enabled;
		if (glfwWindow)
			Platform::SyncCustomWndProc(glfwWindow);
	}

	void WindowPrivate::onTick()
	{
		if (surfaceResizeFallbackPending)
		{
			surfaceResizeFallbackPending = false;
			CommitSurfaceResize();
			return;
		}

		if (window->GetView()->IsDirty())
			Repaint();
	}

	void nani::canvas::internal::WindowPrivate::Repaint()
	{
		if (!glfwWindow || !skiaSurface || !skiaGlContext)
			return;

		glfwMakeContextCurrent(glfwWindow);
		// Glyph/text draws bind GL textures. After making the context current (or any
		// external GL use), resync Skia's cached GL state so atlas sampling stays valid.
		skiaGlContext->resetContext();

		SkCanvas* canvas = skiaSurface->getCanvas();
		if (!canvas)
			return;

		// Clear the full physical framebuffer, then draw in logical coordinates.
		canvas->clear(SK_ColorTRANSPARENT);

		const float dpr = (devicePixelRatio > 0.0f) ? devicePixelRatio : 1.0f;
		canvas->save();
		canvas->scale(dpr, dpr);

		SkRect rect = SkRect::MakeXYWH(0, 0, size.width, size.height);
		SkRect fillRect = rect;
		fillRect.inset(borderWidth / 2, borderWidth / 2);
		SkPaint fillPaint;
		fillPaint.setAntiAlias(true);
		fillPaint.setStyle(SkPaint::kStrokeAndFill_Style);
		fillPaint.setColor(skia_utils::ToSkColor(backgroundColor));
		canvas->drawRRect(SkRRect::MakeRectXY(fillRect, radius, radius), fillPaint);

		if (borderWidth > 0)
		{
			SkPaint strokePaint;
			SkRect strokeRect = rect;
			strokeRect.inset(borderWidth / 2, borderWidth / 2);
			strokePaint.setAntiAlias(true);
			strokePaint.setStyle(SkPaint::kStroke_Style);
			strokePaint.setStrokeWidth(borderWidth);
			strokePaint.setColor(skia_utils::ToSkColor(borderColor));
			canvas->drawRRect(SkRRect::MakeRectXY(strokeRect, radius, radius), strokePaint);
		}

		canvas->save();
		canvas->clipRRect(SkRRect::MakeRectXY(fillRect, radius, radius));

		PaintEvent event(RectF(PointF(0.0f, 0.0f), size));
		window->FireEvent(&event);

		canvas->restore();
		canvas->restore();

		skiaGlContext->flushAndSubmit();
		glfwSwapBuffers(glfwWindow);
	}

	bool WindowPrivate::Initialize()
	{
		InitializeGLFWWindow();
		InitializeSkiaContext();
		if (!glfwWindow || !skiaGlContext || !skiaSurface)
		{
			NANI_ASSERT(false);
			NANI_MESSAGE("Failed to initialize window.");
			return false;
		}
		return true;
	}

	void WindowPrivate::InitializeGLFWWindow()
	{
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#if defined(GLFW_SCALE_FRAMEBUFFER)
		glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_TRUE);
#endif

		
		// Create at logical size first; SyncDpiSurface(true) expands the platform
		// window on systems where FB == window size (Windows).
		glfwWindow = glfwCreateWindow(
			std::max(1, static_cast<int>(std::lround(size.width))),
			std::max(1, static_cast<int>(std::lround(size.height))),
			title.c_str(),
			nullptr,
			nullptr);
		if (!glfwWindow)
			return;

		glfwSetWindowUserPointer(glfwWindow, this);
		glfwSetWindowPos(glfwWindow, pos.x, pos.y);
		_SetWindowHints(glfwWindow, (Window::Hint)hints);

		EnvPrivate::Instance()->RegisterWindow(glfwWindow);
		glfwMakeContextCurrent(glfwWindow);
		glfwSwapInterval(1);

		glfwSetWindowSizeCallback(glfwWindow, _OnGLFWWindowSizeChanged);
		glfwSetFramebufferSizeCallback(glfwWindow, _OnGLFWFramebufferSizeChanged);
		glfwSetWindowContentScaleCallback(glfwWindow, _OnGLFWWindowContentScaleChanged);
		glfwSetWindowPosCallback(glfwWindow, _OnGLFWWindowPositionChanged);
		glfwSetWindowCloseCallback(glfwWindow, _OnGLFWWindowClose);
		glfwSetWindowFocusCallback(glfwWindow, _OnGLFWWindowFocusChanged);
		glfwSetCursorPosCallback(glfwWindow, _OnGLFWWindowMousePos);
		glfwSetMouseButtonCallback(glfwWindow, _OnGLFWWindowMouseButton);
		glfwSetCursorEnterCallback(glfwWindow, _OnGLFWWindowMouseEnter);
		glfwSetScrollCallback(glfwWindow, _OnGLFWWindowWheelScroll);
		glfwSetKeyCallback(glfwWindow, _OnGLFWWindowKeyEvent);
		glfwSetCharCallback(glfwWindow, _OnGLFWWindowChar);

		SyncDpiSurface(true);
		Platform::EnsureImeHook(glfwWindow);
	}

	void WindowPrivate::InitializeSkiaContext()
	{
		sk_sp<const GrGLInterface> skiaGlInterface = GrGLMakeNativeInterface();
		if (!skiaGlInterface)
			return;

		skiaGlContext = GrDirectContexts::MakeGL(skiaGlInterface);
		if (!skiaGlContext)
			return;

		SyncDpiSurface(false);
	}

	void WindowPrivate::RefreshDpiState()
	{
		if (!glfwWindow)
			return;

		PlatformSurfaceSnapshot snapshot;
		if (!QueryPlatformSurfaceSnapshot(*this, snapshot))
			return;

		devicePixelRatio = snapshot.devicePixelRatio;
		platformWindowSize = snapshot.platform;

		if (snapshot.framebuffer.IsValid())
			framebufferSize = snapshot.framebuffer;
		else if (platformWindowSize.width > 0.0f)
			framebufferSize = platformWindowSize;
		else
			framebufferSize = SizeF(size.width * devicePixelRatio, size.height * devicePixelRatio);
	}

	void WindowPrivate::SyncDpiSurface(bool syncPlatformWindow)
	{
		if (!glfwWindow)
			return;

		RefreshDpiState();

		if (syncPlatformWindow)
			SyncPlatformWindowToLogicalDpi();

		BindSkiaSurface();
	}

	void WindowPrivate::SyncPlatformWindowToLogicalDpi()
	{
		if (!glfwWindow)
			return;

		const int winW = static_cast<int>(platformWindowSize.width);
		const int winH = static_cast<int>(platformWindowSize.height);
		const int fbW = static_cast<int>(framebufferSize.width);
		const int fbH = static_cast<int>(framebufferSize.height);

		float fbRatioX = 1.0f;
		float fbRatioY = 1.0f;
		FramebufferScaleForWindow(winW, winH, fbW, fbH, fbRatioX, fbRatioY);

		// Desired physical FB = logical * dpr.
		// Windows fbRatio≈1 → grow platform window; macOS fbRatio≈dpr → keep logical.
		const int desiredWinW = std::max(
			1, static_cast<int>(std::lround(size.width * devicePixelRatio / fbRatioX)));
		const int desiredWinH = std::max(
			1, static_cast<int>(std::lround(size.height * devicePixelRatio / fbRatioY)));

		if (winW != desiredWinW || winH != desiredWinH)
		{
			syncingPlatformSize = true;
			glfwSetWindowSize(glfwWindow, desiredWinW, desiredWinH);
			syncingPlatformSize = false;
			RefreshDpiState();
		}
	}

	void WindowPrivate::BindSkiaSurface()
	{
		if (!glfwWindow)
			return;

		const int fbw = static_cast<int>(framebufferSize.width);
		const int fbh = static_cast<int>(framebufferSize.height);
		if (fbw <= 0 || fbh <= 0)
			return;

		glfwMakeContextCurrent(glfwWindow);
		glViewport(0, 0, fbw, fbh);
		ResetSkiaSurface(false);
	}

	basic::PointF WindowPrivate::LogicalToPlatformScale() const
	{
		float scaleX = 1.0f;
		float scaleY = 1.0f;
		if (size.width > 0.0f && platformWindowSize.width > 0.0f)
			scaleX = platformWindowSize.width / size.width;
		if (size.height > 0.0f && platformWindowSize.height > 0.0f)
			scaleY = platformWindowSize.height / size.height;
		return PointF(scaleX, scaleY);
	}

	basic::RectF WindowPrivate::LogicalToPlatformRect(const RectF& logical) const
	{
		const PointF scale = LogicalToPlatformScale();
		return RectF(
			logical.left * scale.x,
			logical.top * scale.y,
			logical.right * scale.x,
			logical.bottom * scale.y);
	}

	basic::PointF WindowPrivate::ScreenToLogical(double xPos, double yPos) const
	{
		const PointF scale = LogicalToPlatformScale();
		if (scale.x <= 0.0f || scale.y <= 0.0f)
			return PointF(static_cast<float>(xPos), static_cast<float>(yPos));

		return PointF(
			static_cast<float>(xPos) / scale.x,
			static_cast<float>(yPos) / scale.y);
	}

	void WindowPrivate::ResetSkiaSurface(bool verifyFramebufferSize)
	{
		if (!skiaGlContext)
			return;

		skiaSurface.reset();

		// Describe the real default framebuffer. Mismatched sample/stencil counts
		// make Skia's backend RT invalid for texture-backed ops (glyph atlases).
		int sampleCnt = 0;
		int stencilBits = 8;
		int fbw = static_cast<int>(framebufferSize.width);
		int fbh = static_cast<int>(framebufferSize.height);
		if (glfwWindow)
		{
			sampleCnt = glfwGetWindowAttrib(glfwWindow, GLFW_SAMPLES);
			stencilBits = glfwGetWindowAttrib(glfwWindow, GLFW_STENCIL_BITS);
			if (sampleCnt < 0)
				sampleCnt = 0;
			if (stencilBits < 0)
				stencilBits = 0;

			if (verifyFramebufferSize)
			{
				int glfwFbw = 0;
				int glfwFbh = 0;
				glfwGetFramebufferSize(glfwWindow, &glfwFbw, &glfwFbh);
				if (glfwFbw > 0 && glfwFbh > 0)
				{
					fbw = glfwFbw;
					fbh = glfwFbh;
					framebufferSize = SizeF(static_cast<float>(fbw), static_cast<float>(fbh));
				}
			}
		}

		if (fbw <= 0 || fbh <= 0)
		{
			fbw = static_cast<int>(size.width);
			fbh = static_cast<int>(size.height);
		}
		if (fbw <= 0 || fbh <= 0)
			return;

		GrGLFramebufferInfo fbi =
		{
			.fFBOID = 0,
			.fFormat = GL_RGBA8
		};

		GrBackendRenderTarget target = GrBackendRenderTargets::MakeGL(
			fbw,
			fbh,
			sampleCnt,
			stencilBits,
			fbi);
		skiaSurface = SkSurfaces::WrapBackendRenderTarget(
			skiaGlContext.get(),
			target,
			kBottomLeft_GrSurfaceOrigin,
			kRGBA_8888_SkColorType,
			nullptr,
			nullptr
		);
	}

	SkCanvas* WindowPrivate::GetCanvas()
	{
		return skiaSurface ? skiaSurface->getCanvas() : nullptr;
	}
}

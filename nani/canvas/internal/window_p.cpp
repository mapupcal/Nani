#pragma once
#include "window_p.h"
#include "window.h"
#include "env_p.h"
#include "platform.h"
#include "events/event.h"
#include <include/gpu/ganesh/gl/GrGLInterface.h>
#include <include/gpu/ganesh/GrContextOptions.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/gpu/ganesh/gl/GrGLBackendSurface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/ganesh/GrBackendSurface.h>
#include <core/SkCanvas.h>
#include <core/SkColorSpace.h>
#include <include/core/SkRRect.h>
#include <include/core/SkRect.h>
#include <ranges>

using namespace nani::canvas::events;
using namespace nani::canvas::basic;

namespace nani::canvas::internal
{
	namespace
	{
		void _OnGLFWWindowSizeChanged(GLFWwindow* window, int width, int height)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(window));
			if (pImpl)
				pImpl->OnGLFWWindowSizeChanged(width, height);
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
				pImpl->OnGLFWWindowKeyEvent(key_, scancode, action == GLFW_PRESS, modifier);
			}
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

	void WindowPrivate::Resize(const basic::SizeF& size)
	{
		if (!glfwWindow)
			return;
		if (size == this->size)
			return;

		glfwSetWindowSize(glfwWindow, size.width, size.height);
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

	void WindowPrivate::OnGLFWWindowSizeChanged(int width, int height)
	{
		if (!glfwWindow)
			return;
		if (width <= 0 || height <= 0)
			return;

		SizeF oldSize = size;
		size = SizeF(width, height);

		glfwMakeContextCurrent(glfwWindow);
		glViewport(0, 0, width, height);
		ResetSkiaSurface();

		ResizeEvent event(oldSize, size);
		window->FireEvent(&event);
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
		Type type = bEnter ? Type::Enter : Type::Leave;
		Event event(type);
		window->FireEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowMouseMove(double xPos, double yPos)
	{
		PointF pos_(xPos, yPos);
		PointF globalPos = pos_ + pos;
		MouseMoveEvent event(pos_, globalPos);
		window->FireEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowMouseButton(double xPos, double yPos, MouseButton button, bool bPress, Modifier modifier)
	{
		PointF pos_(xPos, yPos);
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
		WheelEvent event(xDelta, yDelta);
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

	void WindowPrivate::onTick()
	{
		window->Update();
	}

	void nani::canvas::internal::WindowPrivate::Repaint()
	{
		if (!glfwWindow || !skiaSurface || !skiaGlContext)
			return;

		glfwMakeContextCurrent(glfwWindow);
		SkCanvas* canvas = skiaSurface->getCanvas();
		if (!canvas)
			return;

		canvas->clear(SK_ColorTRANSPARENT);

		SkRect rect = SkRect::MakeXYWH(0, 0, size.width, size.height);

		SkRect fillRect = rect;
		fillRect.inset(borderWidth / 2, borderWidth / 2);
		SkPaint fillPaint;
		fillPaint.setAntiAlias(true);
		fillPaint.setStyle(SkPaint::kStrokeAndFill_Style);
		fillPaint.setColor(SkColorSetARGB(backgroundColor.a, backgroundColor.r, backgroundColor.g, backgroundColor.b));
		canvas->drawRRect(SkRRect::MakeRectXY(fillRect, radius, radius), fillPaint);

		if (borderWidth > 0)
		{
			SkPaint strokePaint;
			SkRect strokeRect = rect;
			strokeRect.inset(borderWidth / 2, borderWidth / 2);
			strokePaint.setAntiAlias(true);
			strokePaint.setStyle(SkPaint::kStroke_Style);
			strokePaint.setStrokeWidth(borderWidth);
			strokePaint.setColor(SkColorSetARGB(borderColor.a, borderColor.r, borderColor.g, borderColor.b));
			canvas->drawRRect(SkRRect::MakeRectXY(strokeRect, radius, radius), strokePaint);
		}

		PaintEvent event(RectF(PointF(0.0f, 0.0f), size));
		window->FireEvent(&event);
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

		
		glfwWindow = glfwCreateWindow(size.width, size.height, title.c_str(), nullptr, nullptr);
		if (!glfwWindow)
			return;

		glfwSetWindowUserPointer(glfwWindow, this);
		glfwSetWindowPos(glfwWindow, pos.x, pos.y);
		_SetWindowHints(glfwWindow, (Window::Hint)hints);

		EnvPrivate::Instance()->RegisterWindow(glfwWindow);
		glfwMakeContextCurrent(glfwWindow);
		glfwSwapInterval(1);

		glfwSetWindowSizeCallback(glfwWindow, _OnGLFWWindowSizeChanged);
		glfwSetWindowPosCallback(glfwWindow, _OnGLFWWindowPositionChanged);
		glfwSetWindowCloseCallback(glfwWindow, _OnGLFWWindowClose);
		glfwSetWindowFocusCallback(glfwWindow, _OnGLFWWindowFocusChanged);
		glfwSetCursorPosCallback(glfwWindow, _OnGLFWWindowMousePos);
		glfwSetMouseButtonCallback(glfwWindow, _OnGLFWWindowMouseButton);
		glfwSetCursorEnterCallback(glfwWindow, _OnGLFWWindowMouseEnter);
		glfwSetScrollCallback(glfwWindow, _OnGLFWWindowWheelScroll);
		glfwSetKeyCallback(glfwWindow, _OnGLFWWindowKeyEvent);

	}

	void WindowPrivate::InitializeSkiaContext()
	{
		sk_sp<const GrGLInterface> skiaGlInterface = GrGLMakeNativeInterface();
		if (!skiaGlInterface)
			return;

		skiaGlContext = GrDirectContexts::MakeGL(skiaGlInterface);
		if (!skiaGlContext)
			return;

		ResetSkiaSurface();
	}

	void WindowPrivate::ResetSkiaSurface()
	{
		if (!skiaGlContext)
			return;

		skiaSurface.reset();

		GrGLFramebufferInfo fbi =
		{
			.fFBOID = 0,
			.fFormat = GL_RGBA8
		};

		GrBackendRenderTarget target = GrBackendRenderTargets::MakeGL(size.width, size.height, 0, 8, fbi);
		skiaSurface = SkSurfaces::WrapBackendRenderTarget(
			skiaGlContext.get(),
			target,
			kBottomLeft_GrSurfaceOrigin,
			kRGBA_8888_SkColorType,
			nullptr,
			nullptr
		);

		Repaint();
	}

	SkCanvas* WindowPrivate::GetCanvas()
	{
		return skiaSurface ? skiaSurface->getCanvas() : nullptr;
	}
}

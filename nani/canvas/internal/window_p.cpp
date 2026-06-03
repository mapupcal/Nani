#pragma once
#include "window_p.h"
#include "window.h"
#include "env_p.h"
#include <include/gpu/ganesh/gl/GrGLInterface.h>
#include <include/gpu/ganesh/GrContextOptions.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/gpu/ganesh/gl/GrGLBackendSurface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/ganesh/GrBackendSurface.h>
#include <core/SkCanvas.h>
#include <core/SkColorSpace.h>
#include <ranges>

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
				if (btn == GLFW_MOUSE_BUTTON_MIDDLE)
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
		}

		if (IsVisible())
			return;

		glfwShowWindow(glfwWindow);

		Event event(Event::Type::Show);
		window->RaiseEvent(&event);
		Paint(RectF(0, 0, size));
	}

	void WindowPrivate::Hide()
	{
		if (!glfwWindow)
			return;

		glfwHideWindow(glfwWindow);

		Event event(Event::Type::Hide);
		window->RaiseEvent(&event);
	}

	void WindowPrivate::Close()
	{
		if (glfwWindow != nullptr)
		{
			EnvPrivate::Instance()->UnRegisterWindow(glfwWindow);
			glfwWindowShouldClose(glfwWindow);
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

	void WindowPrivate::OnGLFWWindowSizeChanged(int width, int height)
	{
		if (width <= 0 || height <= 0)
			return;

		SizeF oldSize = size;
		size = SizeF(width, height);

		glfwMakeContextCurrent(glfwWindow);
		glViewport(0, 0, width, height);
		ResetSkiaSurface();

		ResizeEvent event(oldSize, size);
		window->RaiseEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowPositionChanged(int xPos, int yPos)
	{
		PointF oldPos = pos;
		pos = PointF(xPos, yPos);

		MoveEvent event(oldPos, pos);
		window->RaiseEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowFocusChanged(bool bFocus)
	{
		Event event(bFocus ? Event::Type::FocusIn : Event::Type::FocusOut);
		window->RaiseEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowClose()
	{
		EnvPrivate::Instance()->UnRegisterWindow(glfwWindow);
		glfwWindow = nullptr;
		skiaGlContext.reset();
		skiaSurface.reset();

		Event event(Event::Type::Close);
		window->RaiseEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowMouseEnter(bool bEnter)
	{
		Event::Type type = bEnter ? Event::Type::Enter : Event::Type::Leave;
		Event event(type);
		window->RaiseEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowMouseMove(double xPos, double yPos)
	{
		PointF pos_(xPos, yPos);
		PointF globalPos = pos_ + pos;
		MouseMoveEvent event(pos_, globalPos);
		window->RaiseEvent(&event);
	}

	void WindowPrivate::OnGLFWWindowMouseButton(double xPos, double yPos, MouseButton button, bool bPress, Modifier modifier)
	{
		PointF pos_(xPos, yPos);
		PointF globalPos = pos_ + pos;
		if (bPress)
		{
			MousePressEvent event(button, pos_, globalPos, modifier);
			window->RaiseEvent(&event);
		}
		else
		{
			MouseReleaseEvent event(button, pos_, globalPos, modifier);
			window->RaiseEvent(&event);
		}
	}

	void WindowPrivate::OnGLFWWindowWheelScroll(double xDelta, double yDelta)
	{
		WheelEvent event(xDelta, yDelta);
		window->RaiseEvent(&event);
	}

	void WindowPrivate::Paint(const RectF& dirtyRect)
	{
		if (!skiaSurface)
			return;

		SkCanvas* canvas = skiaSurface->getCanvas();
		if (!canvas)
			return;

		canvas->clear(SK_ColorTRANSPARENT); //TODO:Dirty Rect Update.
		PaintEvent event(dirtyRect);
		window->RaiseEvent(&event);
		skiaGlContext->flushAndSubmit();
		glfwSwapBuffers(glfwWindow);
	}

	void WindowPrivate::RegisterEventFilter(Event::IFilter* filter)
	{
		auto iter = std::find(eventFilters.cbegin(), eventFilters.cend(), filter);
		if (iter != eventFilters.cend())
		{
			NANI_MESSAGE("filter already registered.");
			return;
		}
		eventFilters.push_back(filter);
	}

	void WindowPrivate::UnRegisterEventFilter(Event::IFilter * filter)
	{
		auto iter = std::find(eventFilters.cbegin(), eventFilters.cend(), filter);
		if (iter == eventFilters.cend())
		{
			NANI_MESSAGE("filter not registered.");
			return;
		}
		eventFilters.erase(iter);
	}

	bool WindowPrivate::FilterEvent(Event * event)
	{
		for (Event::IFilter* filter : eventFilters | std::views::reverse)
		{
			if (filter->FilterEvent(event))
				return true;
		}
		return false;
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

		glfwWindow = glfwCreateWindow(size.width, size.height, "Nani", nullptr, nullptr);
		if (!glfwWindow)
			return;

		EnvPrivate::Instance()->RegisterWindow(glfwWindow);
		glfwMakeContextCurrent(glfwWindow);
		glfwSwapInterval(1);

		glfwSetWindowUserPointer(glfwWindow, this);
		glfwSetWindowSizeCallback(glfwWindow, _OnGLFWWindowSizeChanged);
		glfwSetWindowPosCallback(glfwWindow, _OnGLFWWindowPositionChanged);
		glfwSetWindowCloseCallback(glfwWindow, _OnGLFWWindowClose);
		glfwSetCursorPosCallback(glfwWindow, _OnGLFWWindowMousePos);
		glfwSetMouseButtonCallback(glfwWindow, _OnGLFWWindowMouseButton);
		glfwSetCursorEnterCallback(glfwWindow, _OnGLFWWindowMouseEnter);
		glfwSetScrollCallback(glfwWindow, _OnGLFWWindowWheelScroll);

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

		Paint(RectF(0, 0, size));
	}
}

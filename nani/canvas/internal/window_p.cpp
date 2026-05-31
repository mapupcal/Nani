#pragma once
#include "window_p.h"
#include "window.h"
#include "event.h"
#include "env_p.h"
#include <include/gpu/ganesh/gl/GrGLInterface.h>
#include <include/gpu/ganesh/GrContextOptions.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/gpu/ganesh/gl/GrGLBackendSurface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/ganesh/GrBackendSurface.h>
#include <core/SkCanvas.h>
#include <core/SkColorSpace.h>

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

	void WindowPrivate::OnGLFWWindowPositionChanged(int xpos, int ypos)
	{
		PointF oldPos = pos;
		pos = PointF(xpos, ypos);

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

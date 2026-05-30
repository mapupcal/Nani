#pragma once
#include <GLFW/glfw3.h>
#include <include/gpu/ganesh/gl/GrGLDirectContext.h>
#include <core/SkSurface.h>
#include "basic/pointf.h"
#include "basic/sizef.h"
#include "basic/rectf.h"

namespace nani::canvas
{
	class Window;
}

namespace nani::canvas::internal
{
	class WindowPrivate
	{
	public:
		WindowPrivate(Window* window_);
		~WindowPrivate();

	public:
		bool IsVisible() const;

		void Show();
		void Hide();
		void Close();
		void Move(const basic::PointF& pos);
		void Resize(const basic::SizeF& size);

		bool Initialize();
		void InitializeGLFWWindow();
		void InitializeSkiaContext();
		void ResetSkiaSurface();

	public:
		void OnGLFWWindowSizeChanged(int width, int height);
		void OnGLFWWindowPositionChanged(int xpos, int ypos);
		void OnGLFWWindowFocusChanged(bool bFocus);
		void OnGLFWWindowClose();

	public:
		basic::PointF pos;
		basic::SizeF size;
		Window* window = nullptr;
		GLFWwindow* glfwWindow = nullptr;
		sk_sp<GrDirectContext> skiaGlContext;
		sk_sp<SkSurface> skiaSurface;
	};
}

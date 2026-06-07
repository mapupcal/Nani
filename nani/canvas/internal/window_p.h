#pragma once
#include <GLFW/glfw3.h>
#include <include/gpu/ganesh/gl/GrGLDirectContext.h>
#include <core/SkSurface.h>
#include "basic/pointf.h"
#include "basic/sizef.h"
#include "basic/rectf.h"
#include "basic/color.h"
#include "events/event_defs.h"
#include <vector>

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

		void SetRadius(basic::single fRadius);
		void SetBorderWidth(basic::single fWidth);
		void SetBorderColor(const basic::Color& color);
		void SetBackgroundColor(const basic::Color& color);
		void SetTitle(const std::string_view& title_);

		bool Initialize();
		void InitializeGLFWWindow();
		void InitializeSkiaContext();
		void ResetSkiaSurface();

		void Paint(const basic::RectF& dirtyRect);

	public:
		void OnGLFWWindowSizeChanged(int width, int height);
		void OnGLFWWindowPositionChanged(int xPos, int yPos);
		void OnGLFWWindowFocusChanged(bool bFocus);
		void OnGLFWWindowClose();
		void OnGLFWWindowMouseEnter(bool bEnter);
		void OnGLFWWindowMouseMove(double xPos, double yPos);
		void OnGLFWWindowMouseButton(double xPos, double yPos, events::MouseButton button, bool bPress, events::Modifier modifier);
		void OnGLFWWindowWheelScroll(double xDelta, double yDelta);
		void OnGLFWWindowKeyEvent(events::Key key, int scancode, bool bPress, events::Modifier modifier);

	private:


	public:
		basic::PointF pos;
		basic::SizeF size;
		basic::single radius = 0.0f;
		basic::single borderWidth = 0.0f;
		basic::Color borderColor = basic::Color(basic::Colors::Transparent);
		basic::Color backgroundColor = basic::Color(basic::Colors::Transparent);
		std::string title;
		Window* window = nullptr;
		GLFWwindow* glfwWindow = nullptr;
		sk_sp<GrDirectContext> skiaGlContext;
		sk_sp<SkSurface> skiaSurface;
	};
}

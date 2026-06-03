#pragma once
#include <GLFW/glfw3.h>
#include <include/gpu/ganesh/gl/GrGLDirectContext.h>
#include <core/SkSurface.h>
#include "basic/pointf.h"
#include "basic/sizef.h"
#include "basic/rectf.h"
#include "event.h"
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

		bool Initialize();
		void InitializeGLFWWindow();
		void InitializeSkiaContext();
		void ResetSkiaSurface();

		void Paint(const basic::RectF& dirtyRect);

		void RegisterEventFilter(Event::IFilter* filter);
		void UnRegisterEventFilter(Event::IFilter* filter);
		bool FilterEvent(Event* event);

	public:
		void OnGLFWWindowSizeChanged(int width, int height);
		void OnGLFWWindowPositionChanged(int xPos, int yPos);
		void OnGLFWWindowFocusChanged(bool bFocus);
		void OnGLFWWindowClose();
		void OnGLFWWindowMouseEnter(bool bEnter);
		void OnGLFWWindowMouseMove(double xPos, double yPos);
		void OnGLFWWindowMouseButton(double xPos, double yPos, MouseButton button, bool bPress, Modifier modifier);
		void OnGLFWWindowWheelScroll(double xDelta, double yDelta);

	public:
		basic::PointF pos;
		basic::SizeF size;
		Window* window = nullptr;
		GLFWwindow* glfwWindow = nullptr;
		sk_sp<GrDirectContext> skiaGlContext;
		sk_sp<SkSurface> skiaSurface;
		std::vector<Event::IFilter*> eventFilters;
	};
}

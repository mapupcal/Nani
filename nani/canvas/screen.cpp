#include "screen.h"
#include <GLFW/glfw3.h>
#include <ranges>

using namespace nani::canvas::basic;

namespace nani::canvas::internal
{
	struct ScreenData
	{
		GLFWmonitor* monitor = nullptr;
	};
}

namespace nani::canvas
{
	const RectF Screen::Rect() const
	{
		if (!m_pData->monitor)
			return RectF();
		const GLFWvidmode* mode = glfwGetVideoMode(m_pData->monitor);
		if (!mode)
			return RectF();
		return RectF(0, 0, mode->width, mode->height);
	}

	const RectF Screen::WorkAreaRect() const
	{
		if (!m_pData->monitor)
			return RectF();
		int x = 0, y = 0, w = 0, h = 0;
		glfwGetMonitorWorkarea(m_pData->monitor, &x, &y, &w, &h);
		return RectF(x, y, w, h);
	}

	namespace
	{
		PointF GetGeometryPos(GLFWmonitor* monitor)
		{
			if (!monitor)
				return PointF();
			int x = 0, y = 0;
			glfwGetMonitorPos(monitor, &x, &y);
			return PointF(x, y);
		}
	}
	const RectF Screen::Geometry() const
	{
		RectF rc = Rect();
		return rc.MoveTo(GetGeometryPos(m_pData->monitor));
	}

	const RectF Screen::WorkAreaGeometry() const
	{
		RectF rc = WorkAreaRect();
		return rc.MoveTo(GetGeometryPos(m_pData->monitor));
	}

	const float Screen::DPI() const
	{
		if (!m_pData->monitor)
			return 1.0f;
		float x = 1.0f, y = 1.0f;
		glfwGetMonitorContentScale(m_pData->monitor, &x, &y);
		return x;
	}

	const int Screen::Width() const
	{
		return Rect().Width();
	}

	const int Screen::Height() const
	{
		return Rect().Height();
	}

	const Screen* Screen::Primary()
	{
		auto lstScreens = Screens();
		if (lstScreens.empty())
			return nullptr;
		return lstScreens.at(0);
	}

	std::vector<const Screen*> Screen::Screens()
	{
		static std::vector<std::shared_ptr<Screen>> g_allScreens;
		if (g_allScreens.empty())
		{
			auto fnEnqueMonitor = [&](GLFWmonitor* monitor) {
				if (!monitor)
					return;
				std::shared_ptr<Screen> spScreen = std::make_shared<Screen>();
				spScreen->m_pData->monitor = monitor;
				g_allScreens.push_back(spScreen);
			};

			//primary screen always at first.
			GLFWmonitor* primary = glfwGetPrimaryMonitor();
			fnEnqueMonitor(primary);

			int count = 0;
			GLFWmonitor** alls = glfwGetMonitors(&count);
			for (int idx = 0; idx < count; idx++)
			{
				GLFWmonitor* monitor = alls[idx];
				if (primary == monitor)
					continue;
				fnEnqueMonitor(monitor);
			}
		}
		auto view = g_allScreens | std::views::transform(
			[](const std::shared_ptr<Screen> spScreen) -> const Screen* {
				return spScreen.get();
			});
		return std::vector<const Screen*>(view.begin(), view.end());
	}

	Screen::Screen()
		: m_pData(new internal::ScreenData())
	{

	}

	Screen::~Screen()
	{
		delete m_pData;
	}
}
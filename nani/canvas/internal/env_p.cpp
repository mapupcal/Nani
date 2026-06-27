#include "env_p.h"
#include "window_p.h"

namespace nani::canvas::internal
{
	EnvPrivate* EnvPrivate::Instance()
	{
		static EnvPrivate ins;
		return &ins;
	}

	void EnvPrivate::Initialize()
	{
		glfwInit();
	}

	void EnvPrivate::Terminate()
	{
		glfwTerminate();
	}

	void EnvPrivate::RegisterWindow(GLFWwindow* window)
	{
		auto iter = std::find(m_lstWindows.cbegin(), m_lstWindows.cend(), window);
		if (iter != m_lstWindows.cend())
			return;
		m_lstWindows.push_back(window);
	}

	void EnvPrivate::UnRegisterWindow(GLFWwindow* window)
	{
		auto iter = std::find(m_lstWindows.cbegin(), m_lstWindows.cend(), window);
		if (iter == m_lstWindows.cend())
			return;
		m_lstWindows.erase(iter);
	}

	int EnvPrivate::WaitForQuit()
	{
		while (!m_lstWindows.empty())
		{
			glfwWaitEvents();
			Tick();
			glfwPollEvents();
		}

		return 0;
	}

	EnvPrivate::EnvPrivate()
	{

	}

	EnvPrivate::~EnvPrivate()
	{

	}

	void EnvPrivate::Tick()
	{
		for (auto glfwWindow : m_lstWindows)
		{
			WindowPrivate* pImpl = reinterpret_cast<WindowPrivate*>(glfwGetWindowUserPointer(glfwWindow));
			if (pImpl)
				pImpl->onTick();
		}
	}
}


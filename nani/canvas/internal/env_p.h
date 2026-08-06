#pragma once

#include "defs.h"

#include <GLFW/glfw3.h>

namespace nani::canvas
{
	class Timer;
}

namespace nani::canvas::internal
{
	class EnvPrivate
	{
	public:
		static EnvPrivate* Instance();

		void Initialize();
		void Terminate();

		void RegisterWindow(GLFWwindow* window);
		void UnRegisterWindow(GLFWwindow* window);

		void RegisterActiveTimer(Timer* timer);
		void UnregisterActiveTimer(Timer* timer);

		int WaitForQuit();
		void ProcessEvents();

	private:
		EnvPrivate();
		~EnvPrivate();

	private:
		void Tick();
		void ProcessTimers();
		double SecondsUntilNextTimer() const;

	private:
		std::vector<GLFWwindow*> m_lstWindows;
		std::vector<Timer*> m_timers;
	};
}

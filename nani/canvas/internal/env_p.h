#pragma once
#include "defs.h"
#include <GLFW/glfw3.h>

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

		int WaitForQuit();

	private:
		EnvPrivate();
		~EnvPrivate();

	private:
		void Tick();

	private:
		std::vector<GLFWwindow*> m_lstWindows;
	};
}

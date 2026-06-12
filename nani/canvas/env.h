#pragma once
#include "defs.h"

namespace nani::canvas
{
	class NANI_CANVAS_API Env
	{
	public:
		Env(int argc, char** argv);
		~Env();

	public:
		int WaitForQuit();
	};
}

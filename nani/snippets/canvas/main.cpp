#include "../canvas/env.h"
#include "../canvas/window.h"
#include <memory>

using namespace nani::canvas::basic;
using namespace nani::canvas;

int main(int argc, char** argv)
{
	Env env(argc, argv);

	std::shared_ptr<Window> window = std::make_shared<Window>(PointF(0,0), SizeF(600,400));
	window->Show();

	int ret = env.WaitForQuit();
	return ret;
}


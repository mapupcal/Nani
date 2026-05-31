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
	printf("visible : %d \n", window->IsVisible());
	window->Hide();
	printf("visible : %d \n", window->IsVisible());
	
	printf("positon : %f, %f \n", window->Position().x, window->Position().y);
	window->Move({ 100, 100 });
	printf("positon : %f, %f \n", window->Position().x, window->Position().y);
	window->Show();
	int ret = env.WaitForQuit();
	return ret;
}


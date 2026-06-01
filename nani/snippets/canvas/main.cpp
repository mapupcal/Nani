#include "../canvas/env.h"
#include "../canvas/window.h"
#include "../canvas/event.h"
#include <memory>
#include <functional>

using namespace nani::canvas::basic;
using namespace nani::canvas;

struct WindowEventFilterDelegate : public Event::IFilter
{
public:
	WindowEventFilterDelegate(Window* window_)
		: window(window_)
	{
		if (window)
			window->RegisterEventFilter(this);
	}

	~WindowEventFilterDelegate()
	{
		if (window)
			window->UnRegisterEventFilter(this);
	}

	virtual bool FilterEvent(Event* e)
	{
		if (delegate)
			return delegate(e);
		return false;
	}

	std::function<bool(Event* e)> delegate;
private:
	Window* window = nullptr;
};

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

	WindowEventFilterDelegate watcher(window.get());
	watcher.delegate = [=](Event* e) -> bool {
		if (e->type == Event::Type::MousePress)
		{
			MousePressEvent* mpe = static_cast<MousePressEvent*>(e);
			if (mpe->button == MouseButton::Left)
				window->Close();
		}
		return false;
	};

	int ret = env.WaitForQuit();
	return ret;
}


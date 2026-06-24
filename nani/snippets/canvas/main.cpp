#include "canvas/env.h"
#include "canvas/window.h"
#include "canvas/events/event.h"
#include "canvas/events/event_filter.h"
#include "canvas/screen.h"
#include "canvas/elements/element.h"
#include <memory>
#include <functional>

using namespace nani::canvas::basic;
using namespace nani::canvas::events;
using namespace nani::canvas::elements;
using namespace nani::canvas;

struct WindowEventFilterDelegate : public EventFilter
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

	bool Filter(EventTarget* target, Event* e) override
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
	window->SetTitle("Nani Canvas");
	window->SetBackgroundColor(Colors::Magenta);
	window->SetHints(Window::Tool | Window::Top | Window::TruncatedPassThrough | Window::Resizable);
	window->SetBorderWidth(2.0f);
	window->SetRadius(50.0f);
	window->SetBorderColor(Colors::Cyan);
	auto element = new Element(window->RootElement());
	window->Show();
	WindowEventFilterDelegate watcher(window.get());
	watcher.delegate = [=](Event* e) -> bool {
		if (e->type == Type::MousePress)
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


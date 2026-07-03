#include "canvas/env.h"
#include "canvas/window.h"
#include "canvas/events/event.h"
#include "canvas/events/event_filter.h"
#include "canvas/screen.h"
#include "canvas/elements/element.h"
#include "canvas/styles.h"
#include <memory>
#include <functional>

using namespace nani::canvas::basic;
using namespace nani::canvas::events;
using namespace nani::canvas::elements;
using namespace nani::canvas;

struct EventFilterDelegate : public EventFilter
{
public:
	EventFilterDelegate(EventTarget* target)
		: m_target(target)
	{
		if (m_target)
			m_target->RegisterEventFilter(this);
	}

	~EventFilterDelegate()
	{
		if (m_target)
			m_target->UnRegisterEventFilter(this);
	}

	bool Filter(EventTarget*, Event* e) override
	{
		if (delegate)
			return delegate(e);
		return false;
	}

	std::function<bool(Event* e)> delegate;

private:
	EventTarget* m_target = nullptr;
};

int main(int argc, char** argv)
{
	Env env(argc, argv);
	float borderWidth = 5.0f;
	std::shared_ptr<Window> window = std::make_shared<Window>(PointF(0, 0), SizeF(600 + borderWidth, 400 + borderWidth));
	window->SetTitle("Nani Canvas");
	window->SetBackgroundColor(Colors::White);
	window->SetBorderWidth(borderWidth);
	window->SetRadius(8.0f);
	window->SetBorderColor(Colors::Yellow);
	window->SetHints(Window::Tool | Window::Top | Window::TruncatedPassThrough | Window::Resizable);

	auto styles = window->RootElement()->GetStyles();
	styles->LoadFromXML(R"(
		<Styles>
			<Style class="NaniWindow">
				<FlexBox direction="ltr" flexDirection="row" />
				<Colors background="#0000FFFF"/>
			</Style>

			<Style class="Button">
				<Dimension width="50" height="50" minWidth="50" minHeight="50" maxWidth="50" maxHeight="50" />
				<Radius radius="4" />
				<Colors background="#00FFFF80" border="#FF000080"/>
			</Style>

			<Style class="CloseButton" inherit="Button">
				<Colors background="#FF0000FF" border="#FFFFFFFF"/>
				<Transform>
					<Scaling x="2" y="2" />
					<Rotation a="45" />	
				</Transform>
				<Borders value="2" />
			</Style>

			<Style class="TitlePanel">
				<FlexBox direction="ltr" flexDirection="row" />
				<Gaps gap="5" />
				<Borders value="5" />
				<Dimension width="100%" height="60" />
			</Style>

			<Style class="Title">
				<FlexBox flex="1.0" />
				<Dimension height="50"/>
				<Colors background="#00FF00FF" />
			</Style>
		</Styles>
	)");


	Element* panel = new Element(window->RootElement());
	panel->SetStyleClass(u8"TitlePanel");
	EventFilterDelegate panelWatcher(panel);
	panelWatcher.delegate = [](Event* e) -> bool {
		if (e->type == Type::MouseMove)
		{
			auto me = static_cast<MouseMoveEvent*>(e);
			printf("mouse move pos(%f, %f), globalPos(%f, %f) over panel.\n", me->pos.x, me->pos.y, me->globalPos.x, me->globalPos.y);
		}
		return false;
	};

	Element* title = new Element(panel);
	title->SetStyleClass(u8"Title");
	EventFilterDelegate titleWatcher(title);
	titleWatcher.delegate = [](Event* e) -> bool {
		if (e->type == Type::MouseMove)
		{
			auto me = static_cast<MouseMoveEvent*>(e);
			printf("mouse move pos(%f, %f), globalPos(%f, %f) over title.\n", me->pos.x, me->pos.y, me->globalPos.x, me->globalPos.y);
		}
		return false;
	};

	Element* close = new Element(panel);
	close->SetStyleClass(u8"CloseButton");
	EventFilterDelegate closeWatcher(close);
	closeWatcher.delegate = [=](Event* e) -> bool {
		if (e->type == Type::MouseMove)
		{
			auto me = static_cast<MouseMoveEvent*>(e);
			printf("mouse move pos(%f, %f), globalPos(%f, %f) over close button.\n", me->pos.x, me->pos.y, me->globalPos.x, me->globalPos.y);
		}
		if (e->type == Type::MousePress)
			window->Close();
		return false;
	};
	window->Show();
	int ret = env.WaitForQuit();
	return ret;
}


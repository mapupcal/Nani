#include "canvas/env.h"
#include "canvas/window.h"
#include "canvas/events/event.h"
#include "canvas/events/event_filter.h"
#include "canvas/screen.h"
#include "canvas/elements/element.h"
#include "canvas/elements/text_element.h"
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
	window->SetBackgroundColor(Color(0xF4F6F8FF));
	window->SetBorderWidth(borderWidth);
	window->SetRadius(10.0f);
	window->SetBorderColor(Color(0x9AA3AFFF));
	window->SetHints(Window::Tool | Window::Top | Window::TruncatedPassThrough | Window::Resizable);

	auto styles = window->RootElement()->GetStyles();
	styles->LoadFromXML(R"(
		<Styles>
			<Style class="NaniWindow">
				<FlexBox direction="ltr" flexDirection="row" />
				<Colors background="#F4F6F8FF"/>
			</Style>

			<Style class="Button">
				<Dimension width="50" height="50" minWidth="50" minHeight="50" maxWidth="50" maxHeight="50" />
				<Radius radius="8" />
				<Colors background="#CBD5E140" border="#94A3B880"/>
			</Style>

			<Style class="Button" state="hovered">
				<Radius radius="10" />
				<Colors background="#CBD5E180" border="#64748BCC"/>
			</Style>

			<Style class="CloseButton" inherit="Button">
				<Colors background="#E25555FF" border="#F8DADAFF"/>
				<Transform>
					<Scaling x="1.1" y="1.1" />
				</Transform>
				<Borders value="1.5" />
			</Style>

			<Style class="CloseButton" state="hovered">
				<Colors background="#F06A6AFF" border="#FFFFFFFF"/>
				<Radius radius="12" />
			</Style>

			<Style class="TitlePanel">
				<FlexBox direction="ltr" flexDirection="row" alignItems="center" />
				<Gaps gap="8" />
				<Borders value="8" />
				<Dimension width="100%" height="56" />
				<Colors background="#1E293BFF" />
			</Style>

			<Style class="DefaultText">
				<Font family="Segoe UI" size="14" style="normal" weight="normal" />
				<Colors color="#334155FF" />
			</Style>

			<Style class="TitleText" inherit="DefaultText">
				<Font family="Segoe UI" size="18" weight="bold" />
				<FlexBox flex="1.0" />
				<Dimension height="40" />
				<Colors color="#F8FAFCFF" />
				<TextAlignment horizontal="center" vertical="center" />
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

	TextElement* title = new TextElement(panel, u8"Nani Canvas");
	title->SetStyleClass(u8"TitleText");
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


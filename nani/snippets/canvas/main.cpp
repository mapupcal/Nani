#include "canvas/env.h"
#include "canvas/styles.h"
#include "canvas/window.h"

#include "canvas/elements/element.h"
#include "canvas/elements/text_element.h"
#include "canvas/elements/scroll_area_element.h"

#include "canvas/events/event.h"
#include "canvas/events/event_filter.h"

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
	std::shared_ptr<Window> window = std::make_shared<Window>(PointF(0, 0), SizeF(960 + borderWidth, 640 + borderWidth));
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
				<FlexBox direction="ltr" flexDirection="column" />
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
				<FlexBox direction="ltr" flexDirection="row" alignItems="center" shrink="0" />
				<Gaps gap="8" />
				<Borders value="8" />
				<Dimension width="100%" height="56" minHeight="56" />
				<Colors background="#1E293BFF" />
			</Style>

			<Style class="DemoPanel">
				<FlexBox flexDirection="row" flex="1.0" grow="1.0" shrink="1.0" alignItems="stretch" />
				<Gaps gap="16" />
				<Paddings value="20" />
				<Dimension width="100%" />
			</Style>

			<Style class="DemoLeft">
				<FlexBox flexDirection="column" flex="1.0" grow="1.0" shrink="1.0" />
				<Gaps gap="12" />
				<Dimension height="100%" />
			</Style>

			<Style class="DemoRight">
				<FlexBox flexDirection="column" shrink="0" />
				<Gaps gap="8" />
				<Dimension width="280" height="100%" />
			</Style>

			<Style class="EffectsRow">
				<FlexBox flexDirection="row" alignItems="center" shrink="0" />
				<Gaps gap="16" />
				<Dimension width="100%" height="96" />
			</Style>

			<Style class="ShadowCard">
				<Dimension width="72" height="72" />
				<Radius radius="12" />
				<Borders value="2" />
				<Colors background="#FFFFFFEE" border="#94A3B8FF" />
			</Style>

			<Style class="ShadowCard" state="hovered">
				<Colors background="#FFFFFFFF" border="#64748BFF" />
				<Shadow color="#0F172A66" x="4" y="6" b="10" s="1" />
			</Style>

			<Style class="OpacityCard">
				<Dimension width="72" height="72" />
				<Radius radius="12" />
				<Colors opacity="0.45" background="#38BDF8FF" border="#0EA5E9FF" />
				<Borders value="2" />
			</Style>

			<Style class="RadiusCard">
				<Dimension width="72" height="72" />
				<Radius tl="4" tr="20" bl="20" br="4" />
				<Borders value="3" />
				<Colors background="#FDE68AFF" border="#D97706FF" />
			</Style>

			<Style class="DefaultText">
				<Font family="Segoe UI" size="14" style="normal" weight="normal" />
				<Colors color="#334155FF" />
				<TextDecoration line="none" />
			</Style>

			<Style class="TitleText" inherit="DefaultText">
				<Font family="Segoe UI" size="18" weight="bold" />
				<FlexBox flex="1.0" />
				<Dimension height="40" />
				<Colors color="#F8FAFCFF" />
				<TextAlignment horizontal="center" vertical="center" />
			</Style>

			<Style class="TitleText" state="hovered">
				<Colors color="#CBD5E1FF" />
				<TextDecoration line="linethrough" style="solid" color="#F87171FF" />
			</Style>

			<Style class="DemoText" inherit="DefaultText">
				<Font family="Segoe UI" size="16" />
				<Dimension height="28" width="100%" />
				<TextAlignment horizontal="left" vertical="center" />
			</Style>

			<Style class="DemoUnderline" inherit="DemoText" />
			<Style class="DemoUnderline" state="hovered">
				<Colors color="#1D4ED8FF" />
				<TextDecoration line="underline" style="solid" color="#2563EBFF" />
			</Style>

			<Style class="DemoOverline" inherit="DemoText" />
			<Style class="DemoOverline" state="hovered">
				<Colors color="#0F766EFF" />
				<TextDecoration line="overline" style="dotted" color="#14B8A6FF" />
			</Style>

			<Style class="DemoLineThrough" inherit="DemoText" />
			<Style class="DemoLineThrough" state="hovered">
				<Colors color="#B91C1CFF" />
				<TextDecoration line="linethrough" style="solid" color="#EF4444FF" />
			</Style>

			<Style class="DemoDashedUnderline" inherit="DemoText" />
			<Style class="DemoDashedUnderline" state="hovered">
				<Colors color="#7C3AEDFF" />
				<TextDecoration line="underline" style="dashed" color="#8B5CF6FF" />
			</Style>

			<Style class="DemoDoubleUnderline" inherit="DemoText" />
			<Style class="DemoDoubleUnderline" state="hovered">
				<Colors color="#C2410CFF" />
				<TextDecoration line="underline" style="double" color="#F97316FF" />
			</Style>

			<Style class="DemoWavyUnderline" inherit="DemoText" />
			<Style class="DemoWavyUnderline" state="hovered">
				<Colors color="#BE185DFF" />
				<TextDecoration line="underline" style="wavy" color="#EC4899FF" />
			</Style>

			<Style class="DemoCombo" inherit="DemoText" />
			<Style class="DemoCombo" state="hovered">
				<Colors color="#334155FF" />
				<TextDecoration line="underline,overline" style="solid" color="#64748BFF" />
			</Style>

			<Style class="ScrollableColumn">
				<FlexBox flexDirection="column" overflow="scroll" />
			</Style>

			<Style class="ScrollDemo" inherit="ScrollableColumn">
				<Dimension width="100%" height="100%" />
				<FlexBox flex="1.0" grow="1.0" shrink="1.0" flexDirection="column" overflow="scroll" />
				<Paddings value="8" />
				<Gaps gap="6" />
				<Borders value="1" />
				<Radius radius="8" />
				<Colors background="#FFFFFFEE" border="#94A3B8FF" />
			</Style>

			<Style class="ScrollDemoLabel" inherit="DefaultText">
				<Dimension width="100%" height="22" />
				<FlexBox shrink="0" />
				<Colors color="#64748BFF" />
				<TextAlignment horizontal="left" vertical="center" />
			</Style>

			<Style class="ScrollDemoItem">
				<Dimension width="100%" height="32" minHeight="32" />
				<FlexBox shrink="0" grow="0" flexDirection="row" alignItems="center" />
				<Colors background="#E2E8F0FF" border="#E2E8F000" />
				<Borders value="1" />
				<Radius radius="4" />
				<Paddings l="8" r="8" t="0" b="0" />
			</Style>

			<Style class="ScrollDemoItem" state="hovered">
				<Colors background="#BFDBFEFF" border="#3B82F6FF" />
				<Radius radius="6" />
			</Style>

			<Style class="ScrollDemoItemText" inherit="DefaultText">
				<FlexBox flex="1.0" grow="1.0" />
				<Dimension height="100%" />
				<TextAlignment horizontal="left" vertical="center" />
				<Colors color="#334155FF" />
			</Style>

			<Style class="ScrollDemoItemText" state="hovered">
				<Colors color="#1D4ED8FF" />
				<TextDecoration line="underline" style="solid" color="#2563EBFF" />
			</Style>
		</Styles>
	)");

	Element* panel = new Element(window->RootElement());
	panel->SetStyleClass(u8"TitlePanel");

	TextElement* title = new TextElement(panel, u8"Nani Canvas");
	title->SetStyleClass(u8"TitleText");

	Element* close = new Element(panel);
	close->SetStyleClass(u8"CloseButton");
	EventFilterDelegate closeWatcher(close);
	closeWatcher.delegate = [=](Event* e) -> bool {
		if (e->type == Type::MousePress)
			window->Close();
		return false;
	};

	Element* demoPanel = new Element(window->RootElement());
	demoPanel->SetStyleClass(u8"DemoPanel");

	Element* demoLeft = new Element(demoPanel);
	demoLeft->SetStyleClass(u8"DemoLeft");

	Element* effectsRow = new Element(demoLeft);
	effectsRow->SetStyleClass(u8"EffectsRow");
	Element* shadowCard = new Element(effectsRow);
	shadowCard->SetStyleClass(u8"ShadowCard");
	Element* opacityCard = new Element(effectsRow);
	opacityCard->SetStyleClass(u8"OpacityCard");
	Element* radiusCard = new Element(effectsRow);
	radiusCard->SetStyleClass(u8"RadiusCard");

	const struct
	{
		const char8_t* label;
		const char8_t* styleClass;
	} demos[] = {
		{ u8"Hover me - Underline (solid)", u8"DemoUnderline" },
		{ u8"Hover me - Overline (dotted)", u8"DemoOverline" },
		{ u8"Hover me - Line-through (solid)", u8"DemoLineThrough" },
		{ u8"Hover me - Underline (dashed)", u8"DemoDashedUnderline" },
		{ u8"Hover me - Underline (double)", u8"DemoDoubleUnderline" },
		{ u8"Hover me - Underline (wavy)", u8"DemoWavyUnderline" },
		{ u8"Hover me - Underline + Overline", u8"DemoCombo" },
	};

	for (const auto& demo : demos)
	{
		TextElement* text = new TextElement(demoLeft, demo.label);
		text->SetStyleClass(demo.styleClass);
	}

	Element* demoRight = new Element(demoPanel);
	demoRight->SetStyleClass(u8"DemoRight");

	TextElement* scrollLabel = new TextElement(demoRight, u8"ScrollArea — mouse wheel");
	scrollLabel->SetStyleClass(u8"ScrollDemoLabel");

	auto* scrollArea = new ScrollAreaElement(demoRight);
	scrollArea->SetStyleClass(u8"ScrollDemo");
	for (int i = 1; i <= 25; ++i)
	{
		Element* row = new Element(scrollArea);
		row->SetStyleClass(u8"ScrollDemoItem");
		const std::string ascii = "Scroll item " + std::to_string(i);
		const std::u8string label(ascii.begin(), ascii.end());
		TextElement* item = new TextElement(row, label);
		item->SetStyleClass(u8"ScrollDemoItemText");
	}

	window->Show();
	int ret = env.WaitForQuit();
	return ret;
}

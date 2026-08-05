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
using namespace nani::canvas::text;
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

namespace
{
	ScrollAreaElement* CreateCategory(
		Element* parent,
		const std::u8string_view& title)
	{
		Element* panel = new Element(parent);
		panel->SetStyleClass(u8"CategoryPanel");

		TextElement* label = new TextElement(panel, title);
		label->SetStyleClass(u8"CategoryTitle");

		auto* scroll = new ScrollAreaElement(panel);
		scroll->SetStyleClass(u8"CategoryScroll");
		return scroll;
	}
}

int main(int argc, char** argv)
{
	Env env(argc, argv);
	float borderWidth = 5.0f;
	const float windowRadius = 12.0f;
	std::shared_ptr<Window> window = std::make_shared<Window>(PointF(0, 0), SizeF(960 + borderWidth * 2, 640 + borderWidth * 2));
	window->SetTitle("Nani Canvas");
	window->SetBackgroundColor(Color(0xF4F6F8FF));
	window->SetBorderWidth(borderWidth);
	window->SetRadius(windowRadius);
	window->SetBorderColor(Color(0xCBD5E1FF));
	window->SetHints(Window::Tool | Window::Top | Window::TruncatedPassThrough | Window::Resizable);

	auto styles = window->RootElement()->GetStyles();
	styles->LoadFromXML(R"(
		<Styles>
			<Style class="NaniWindow">
				<FlexBox direction="ltr" flexDirection="column" />
				<Radius radius="12" />
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

			<Style class="TitleBarButton" inherit="Button">
				<Colors background="#E25555FF" border="#F8DADAFF"/>
				<Transform>
					<Scaling x="1.1" y="1.1" />
				</Transform>
				<Borders value="1.5" />
			</Style>

			<Style class="TitleBarButton" state="hovered">
				<Colors background="#F06A6AFF" border="#FFFFFFFF"/>
				<Radius radius="12" />
			</Style>

			<Style class="TitleBar" window-drag="true">
				<FlexBox direction="ltr" flexDirection="row" alignItems="center" shrink="0" />
				<Gaps gap="8" />
				<Borders value="8" />
				<Dimension width="100%" height="56" minHeight="56" />
				<Radius tl="12" tr="12" bl="0" br="0" />
				<Colors background="#1E293BFF" />
			</Style>

			<Style class="DefaultText">
				<Font family="Segoe UI" size="14" style="normal" weight="normal" />
				<Colors color="#334155FF" />
				<TextDecoration line="none" />
			</Style>

			<Style class="TitleBarText" inherit="DefaultText" window-drag="true">
				<Font family="Segoe UI" size="18" weight="bold" />
				<FlexBox flex="1.0" />
				<Dimension height="40" />
				<Colors color="#F8FAFCFF" />
				<TextAlignment horizontal="center" vertical="center" />
			</Style>

			<Style class="TitleBarText" state="hovered">
				<Colors color="#CBD5E1FF" />
				<TextDecoration line="linethrough" style="solid" color="#F87171FF" />
			</Style>

			<Style class="DemoBody">
				<FlexBox flexDirection="column" flex="1.0" grow="1.0" shrink="1.0" />
				<Gaps gap="12" />
				<Paddings value="16" />
				<Dimension width="100%" />
			</Style>

			<Style class="CategoryRow">
				<FlexBox flexDirection="row" flex="1.0" grow="1.0" shrink="1.0" alignItems="stretch" />
				<Gaps gap="12" />
				<Dimension width="100%" />
			</Style>

			<Style class="CategoryPanel">
				<FlexBox flexDirection="column" flex="1.0" grow="1.0" shrink="1.0" />
				<Gaps gap="8" />
				<Paddings value="10" />
				<Borders value="1" />
				<Radius radius="10" />
				<Colors background="#FFFFFFEE" border="#94A3B8FF" />
				<Dimension height="100%" />
			</Style>

			<Style class="CategoryTitle" inherit="DefaultText">
				<Font family="Segoe UI" size="13" weight="bold" />
				<Dimension width="100%" height="22" />
				<FlexBox shrink="0" />
				<Colors color="#64748BFF" />
				<TextAlignment horizontal="left" vertical="center" />
			</Style>

			<Style class="ScrollableColumn">
				<FlexBox flexDirection="column" overflow="scroll" />
			</Style>

			<Style class="CategoryScroll" inherit="ScrollableColumn">
				<FlexBox flex="1.0" grow="1.0" shrink="1.0" flexDirection="column" overflow="scroll" />
				<Dimension width="100%" height="100%" />
				<Gaps gap="8" />
				<Paddings value="4" />
			</Style>

			<Style class="EffectsContent">
				<FlexBox flexDirection="column" shrink="0" />
				<Gaps gap="12" />
				<Dimension width="100%" />
			</Style>

			<Style class="EffectsRow">
				<FlexBox flexDirection="row" alignItems="center" shrink="0" />
				<Gaps gap="16" />
				<Dimension width="100%" height="96" />
			</Style>

			<Style class="ShadowCard" window-drag="true">
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

			<Style class="EffectsHint" inherit="DefaultText">
				<Dimension width="100%" height="40" />
				<FlexBox shrink="0" />
				<Colors color="#94A3B8FF" />
				<TextAlignment horizontal="left" vertical="top" />
			</Style>

			<Style class="DemoText" inherit="DefaultText">
				<Font family="Segoe UI" size="16" />
				<Dimension height="28" width="100%" />
				<FlexBox shrink="0" />
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

			<Style class="MultiLineDemo" inherit="DefaultText">
				<Dimension width="100%" />
				<FlexBox shrink="0" />
				<Paddings value="10" />
				<Borders value="1" />
				<Radius radius="8" />
				<Colors color="#0F172AFF" background="#F8FAFCFF" border="#CBD5E1FF" />
				<TextAlignment horizontal="left" vertical="top" />
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

	// ---------------------------------------------------------------------------
	// Element tree:
	//   NaniWindow
	//   ├── TitleBar
	//   │   ├── TitleBarText
	//   │   └── TitleBarButton (close)
	//   └── DemoBody
	//       ├── CategoryRow
	//       │   ├── Visual Effects  → ScrollArea → cards
	//       │   └── Text Decorations → ScrollArea → hover demos
	//       └── CategoryRow
	//           ├── Multi-line Text → ScrollArea → wrap sample
	//           └── Scroll Items    → ScrollArea → item rows
	// ---------------------------------------------------------------------------

	Element* titleBar = new Element(window->RootElement());
	titleBar->SetStyleClass(u8"TitleBar");

	TextElement* title = new TextElement(titleBar, u8"Nani Canvas");
	title->SetStyleClass(u8"TitleBarText");

	Element* close = new Element(titleBar);
	close->SetStyleClass(u8"TitleBarButton");
	EventFilterDelegate closeWatcher(close);
	closeWatcher.delegate = [=](Event* e) -> bool {
		if (e->type == Type::MousePress)
			window->Close();
		return false;
	};

	Element* demoBody = new Element(window->RootElement());
	demoBody->SetStyleClass(u8"DemoBody");

	Element* topRow = new Element(demoBody);
	topRow->SetStyleClass(u8"CategoryRow");
	Element* bottomRow = new Element(demoBody);
	bottomRow->SetStyleClass(u8"CategoryRow");

	// --- Visual effects ---
	ScrollAreaElement* effectsScroll = CreateCategory(topRow, u8"Visual Effects");
	Element* effectsContent = new Element(effectsScroll);
	effectsContent->SetStyleClass(u8"EffectsContent");

	Element* effectsRow = new Element(effectsContent);
	effectsRow->SetStyleClass(u8"EffectsRow");
	Element* shadowCard = new Element(effectsRow);
	shadowCard->SetStyleClass(u8"ShadowCard");
	Element* opacityCard = new Element(effectsRow);
	opacityCard->SetStyleClass(u8"OpacityCard");
	Element* radiusCard = new Element(effectsRow);
	radiusCard->SetStyleClass(u8"RadiusCard");

	TextElement* effectsHint = new TextElement(
		effectsContent,
		u8"Hover the white card for shadow. Drag it to move the window.");
	effectsHint->SetStyleClass(u8"EffectsHint");

	// --- Text decorations ---
	ScrollAreaElement* decorationScroll = CreateCategory(topRow, u8"Text Decorations");
	const struct
	{
		const char8_t* label;
		const char8_t* styleClass;
	} decorationDemos[] = {
		{ u8"Hover me - Underline (solid)", u8"DemoUnderline" },
		{ u8"Hover me - Overline (dotted)", u8"DemoOverline" },
		{ u8"Hover me - Line-through (solid)", u8"DemoLineThrough" },
		{ u8"Hover me - Underline (dashed)", u8"DemoDashedUnderline" },
		{ u8"Hover me - Underline (double)", u8"DemoDoubleUnderline" },
		{ u8"Hover me - Underline (wavy)", u8"DemoWavyUnderline" },
		{ u8"Hover me - Underline + Overline", u8"DemoCombo" },
	};
	for (const auto& demo : decorationDemos)
	{
		TextElement* text = new TextElement(decorationScroll, demo.label);
		text->SetStyleClass(demo.styleClass);
	}

	// --- Multi-line text ---
	ScrollAreaElement* multiLineScroll = CreateCategory(bottomRow, u8"Multi-line Text");
	TextElement* multiLine = new TextElement(
		multiLineScroll,
		u8"Line one via hard break.\nLine two via hard break.\n"
		u8"Then a longer paragraph soft-wraps inside the scroll viewport: "
		u8"alpha bravo charlie delta echo foxtrot golf hotel india juliet "
		u8"kilo lima mike november oscar papa quebec romeo sierra tango.");
	multiLine->SetStyleClass(u8"MultiLineDemo");
	multiLine->SetWrapMode(TextWrapMode::Wrap);
	multiLine->SetElideMode(TextElideMode::None);

	// --- Scroll item list ---
	ScrollAreaElement* itemScroll = CreateCategory(bottomRow, u8"ScrollArea Items");
	for (int i = 1; i <= 25; ++i)
	{
		Element* row = new Element(itemScroll);
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

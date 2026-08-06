#include "canvas/env.h"
#include "canvas/styles.h"
#include "canvas/window.h"

#include "canvas/elements/element.h"
#include "canvas/elements/input_text_element.h"
#include "canvas/elements/scroll_area_element.h"
#include "canvas/elements/text_element.h"

#include "canvas/visuals/view.h"
#include "canvas/visuals/visual.h"

#include "canvas/events/event.h"
#include "canvas/events/event_filter.h"

#include "runtime/directory.h"

using namespace nani::canvas;
using namespace nani::canvas::basic;
using namespace nani::canvas::elements;
using namespace nani::canvas::events;
using namespace nani::canvas::text;
using namespace nani::canvas::visuals;
using namespace nani::runtime;

namespace
{
	constexpr float kBorderWidth = 5.0f;
	constexpr float kWindowRadius = 12.0f;
	constexpr float kClientWidth = 640.0f;
	constexpr float kClientHeight = 560.0f;

	constexpr char8_t kLightStylesRelPath[] = u8"assets/canvas/styles.xml";
	constexpr char8_t kDarkStylesRelPath[] = u8"assets/canvas/styles_dark.xml";

	class EventFilterDelegate : public EventFilter
	{
	public:
		explicit EventFilterDelegate(EventTarget* target)
			: m_target(target)
		{
			if (m_target)
				m_target->RegisterEventFilter(this);
		}

		EventFilterDelegate(const EventFilterDelegate&) = delete;
		EventFilterDelegate& operator=(const EventFilterDelegate&) = delete;

		~EventFilterDelegate() override
		{
			if (m_target)
				m_target->UnRegisterEventFilter(this);
		}

		void SetDelegate(std::function<bool(Event*)> delegate)
		{
			m_delegate = std::move(delegate);
		}

		bool Filter(EventTarget*, Event* e) override
		{
			if (m_delegate)
				return m_delegate(e);
			return false;
		}

	private:
		EventTarget* m_target = nullptr;
		std::function<bool(Event*)> m_delegate;
	};

	using FilterList = std::vector<std::unique_ptr<EventFilterDelegate>>;

	std::u8string ToU8Path(const char* path)
	{
		if (!path || path[0] == '\0')
			return {};
		return std::u8string(reinterpret_cast<const char8_t*>(path));
	}

	void RefreshVisualStyles(Visual* visual)
	{
		if (!visual)
			return;

		visual->Update();
		for (const auto& child : visual->Visuals())
			RefreshVisualStyles(child.get());
	}

	Element* CreateStyledElement(Element* parent, const std::u8string_view& styleClass)
	{
		Element* element = new Element(parent);
		element->SetStyleClass(styleClass);
		return element;
	}

	TextElement* CreateStyledText(
		Element* parent,
		const std::u8string_view& text,
		const std::u8string_view& styleClass)
	{
		TextElement* element = new TextElement(parent, text);
		element->SetStyleClass(styleClass);
		return element;
	}

	InputTextElement* CreateStyledInputText(
		Element* parent,
		const std::u8string_view& text,
		const std::u8string_view& styleClass)
	{
		auto* element = new InputTextElement(parent, text);
		element->SetStyleClass(styleClass);
		return element;
	}

	ScrollAreaElement* CreateCategory(
		Element* parent,
		const std::u8string_view& title)
	{
		Element* panel = CreateStyledElement(parent, u8"CategoryPanel");
		CreateStyledText(panel, title, u8"CategoryTitle");

		auto* scroll = new ScrollAreaElement(panel);
		scroll->SetStyleClass(u8"CategoryScroll");
		return scroll;
	}

	void BindMousePress(FilterList& filters, Element* element, std::function<void()> onPress)
	{
		auto watcher = std::make_unique<EventFilterDelegate>(element);
		watcher->SetDelegate([onPress = std::move(onPress)](Event* e) -> bool {
			if (e->type == Type::MousePress)
				onPress();
			return false;
		});
		filters.push_back(std::move(watcher));
	}

	void ApplyTheme(
		Window* window,
		Styles* styles,
		bool dark,
		const std::u8string& lightStylesPath,
		const std::u8string& darkStylesPath)
	{
		if (!window || !styles)
			return;

		styles->LoadFromFile(dark ? darkStylesPath : lightStylesPath);
		if (View* view = window->GetView())
		{
			RefreshVisualStyles(view->Visual());
			view->MarkDirty();
		}
	}

	void BuildTitleBar(
		Element* root,
		Window* window,
		Styles* styles,
		bool& darkTheme,
		const std::u8string& lightStylesPath,
		const std::u8string& darkStylesPath,
		FilterList& filters)
	{
		Element* titleBar = CreateStyledElement(root, u8"TitleBar");
		CreateStyledText(titleBar, u8"Nani Canvas", u8"TitleBarText");

		Element* actions = CreateStyledElement(titleBar, u8"TitleBarActions");
		Element* themeToggle = CreateStyledElement(actions, u8"ThemeBarButton");
		Element* close = CreateStyledElement(actions, u8"TitleBarButton");

		BindMousePress(filters, themeToggle, [&, window, styles, lightStylesPath, darkStylesPath]() {
			darkTheme = !darkTheme;
			ApplyTheme(window, styles, darkTheme, lightStylesPath, darkStylesPath);
		});
		BindMousePress(filters, close, [window]() {
			if (window)
				window->Close();
		});
	}

	void BuildInputTextCategory(Element* row)
	{
		ScrollAreaElement* scroll = CreateCategory(row, u8"Input Text");
		Element* content = CreateStyledElement(scroll, u8"InputDemoContent");

		CreateStyledText(content, u8"Outlined", u8"InputDemoLabel");
		CreateStyledInputText(content, u8"Type here (IME supported)", u8"DemoInputOutlined");

		CreateStyledText(content, u8"Filled / Rounded", u8"InputDemoLabel");
		CreateStyledInputText(content, u8"Search or compose…", u8"DemoInputFilled");

		CreateStyledText(
			content,
			u8"Click a field to focus, then type. Switch fields to end IME composition.",
			u8"InputDemoHint");
	}

	void BuildEffectsCategory(Element* row)
	{
		ScrollAreaElement* scroll = CreateCategory(row, u8"Visual Effects");
		Element* content = CreateStyledElement(scroll, u8"EffectsContent");
		Element* effectsRow = CreateStyledElement(content, u8"EffectsRow");

		CreateStyledElement(effectsRow, u8"ShadowCard");
		CreateStyledElement(effectsRow, u8"OpacityCard");
		CreateStyledElement(effectsRow, u8"RadiusCard");
		CreateStyledText(
			content,
			u8"Hover the white card for shadow. Drag it to move the window.",
			u8"EffectsHint");
	}

	void BuildDecorationCategory(Element* row)
	{
		ScrollAreaElement* scroll = CreateCategory(row, u8"Text Decorations");
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
			CreateStyledText(scroll, demo.label, demo.styleClass);
	}

	void BuildMultiLineCategory(Element* row)
	{
		ScrollAreaElement* scroll = CreateCategory(row, u8"Multi-line Text");
		TextElement* multiLine = CreateStyledText(
			scroll,
			u8"Line one via hard break.\nLine two via hard break.\n"
			u8"Then a longer paragraph soft-wraps inside the scroll viewport: "
			u8"alpha bravo charlie delta echo foxtrot golf hotel india juliet "
			u8"kilo lima mike november oscar papa quebec romeo sierra tango.",
			u8"MultiLineDemo");
		multiLine->SetWrapMode(TextWrapMode::Wrap);
		multiLine->SetElideMode(TextElideMode::None);
	}

	void BuildScrollItemCategory(Element* row)
	{
		ScrollAreaElement* scroll = CreateCategory(row, u8"ScrollArea Items");
		for (int i = 1; i <= 25; ++i)
		{
			Element* itemRow = CreateStyledElement(scroll, u8"ScrollDemoItem");
			const std::string ascii = "Scroll item " + std::to_string(i);
			CreateStyledText(
				itemRow,
				std::u8string(ascii.begin(), ascii.end()),
				u8"ScrollDemoItemText");
		}
	}

	void BuildDemoBody(Element* root)
	{
		Element* demoBody = CreateStyledElement(root, u8"DemoBody");
		Element* inputRow = CreateStyledElement(demoBody, u8"CategoryRow");
		Element* topRow = CreateStyledElement(demoBody, u8"CategoryRow");
		Element* bottomRow = CreateStyledElement(demoBody, u8"CategoryRow");

		BuildInputTextCategory(inputRow);
		BuildEffectsCategory(topRow);
		BuildDecorationCategory(topRow);
		BuildMultiLineCategory(bottomRow);
		BuildScrollItemCategory(bottomRow);
	}
}

int main(int argc, char** argv)
{
	Env env(argc, argv);

	std::shared_ptr<Window> window = std::make_shared<Window>(
		PointF(0.0f, 0.0f),
		SizeF(kClientWidth + kBorderWidth * 2.0f, kClientHeight + kBorderWidth * 2.0f));
	window->SetTitle("Nani Canvas");
	window->SetBackgroundColor(Color(0xF4F6F8FF));
	window->SetBorderWidth(kBorderWidth);
	window->SetRadius(kWindowRadius);
	window->SetBorderColor(Color(0xCBD5E1FF));
	window->SetHints(Window::Tool | Window::Top | Window::TruncatedPassThrough | Window::Resizable);

	const Directory exeDir = Directory::FromFile(ToU8Path(argv[0]));
	const std::u8string lightStylesPath = exeDir.FilePath(kLightStylesRelPath);
	const std::u8string darkStylesPath = exeDir.FilePath(kDarkStylesRelPath);

	Styles* styles = window->RootElement()->GetStyles();
	styles->LoadFromFile(lightStylesPath);

	bool darkTheme = false;
	FilterList filters;
	BuildTitleBar(
		window->RootElement(),
		window.get(),
		styles,
		darkTheme,
		lightStylesPath,
		darkStylesPath,
		filters);
	BuildDemoBody(window->RootElement());

	window->Show();
	return env.WaitForQuit();
}

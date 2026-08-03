#include <gtest/gtest.h>
#include "defs.h"
#include "canvas/elements/text_element.h"
#include "canvas/visuals/text_visual.h"
#include "canvas/events/event_filter.h"
#include "canvas/text/font.h"
#include "canvas/text/font_metrics.h"

using namespace nani::canvas;
using namespace nani::canvas::elements;
using namespace nani::canvas::text;
using namespace nani::canvas::visuals;

class TextElementTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		env_ = new Env(0, nullptr);
		window_ = new Window(PointF(), SizeF(600, 400));
		window_->RootElement()->GetStyles()->LoadFromXML(R"(
			<Styles>
				<Style class="NaniWindow">
					<FlexBox flexDirection="row" />
				</Style>
				<Style class="DefaultText">
					<Font family="Segoe UI" size="14" style="normal" weight="normal" />
					<Colors color="#000000FF" />
				</Style>
				<Style class="FixedText">
					<Font family="Segoe UI" size="14" style="normal" weight="normal" />
					<Colors color="#000000FF" />
					<Dimension width="80" height="20" />
				</Style>
			</Styles>
		)");
	}

	void TearDown() override
	{
		delete window_;
		delete env_;
	}

	Window* window_ = nullptr;

private:
	Env* env_ = nullptr;
};

TEST_F(TextElementTest, CreateVisualReturnsTextVisual)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Hello");
	auto visual = text->CreateVisual(window_->GetView(), nullptr);
	ASSERT_NE(visual, nullptr);
	EXPECT_NE(dynamic_cast<TextVisual*>(visual.get()), nullptr);
}

TEST_F(TextElementTest, SetTextUpdatesContent)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Hello");
	EXPECT_EQ(text->Text(), u8"Hello");

	text->SetText(u8"World");
	EXPECT_EQ(text->Text(), u8"World");
}

TEST_F(TextElementTest, SetTextIgnoresSameValue)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Hello");

	class TextChangeWatcher : public EventFilter
	{
	public:
		explicit TextChangeWatcher(Element* element)
		{
			element->RegisterEventFilter(this);
		}

		~TextChangeWatcher()
		{
			if (target_)
				target_->UnRegisterEventFilter(this);
		}

		bool Filter(EventTarget* target, Event* e) override
		{
			target_ = target;
			if (e->type == Type::ElementTextChanged)
				++changeCount;
			return false;
		}

		EventTarget* target_ = nullptr;
		int changeCount = 0;
	};

	TextChangeWatcher watcher(text);
	text->SetText(u8"Hello");
	EXPECT_EQ(watcher.changeCount, 0);

	text->SetText(u8"Updated");
	EXPECT_EQ(watcher.changeCount, 1);
}

TEST_F(TextElementTest, DefaultStyleClassIsDefaultText)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Label");
	EXPECT_EQ(text->StyleClass(), u8"DefaultText");
}

TEST_F(TextElementTest, ElideModeRoundTrip)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Long text content");
	EXPECT_EQ(text->ElideMode(), TextElideMode::Right);

	text->SetElideMode(TextElideMode::Middle);
	EXPECT_EQ(text->ElideMode(), TextElideMode::Middle);
}

TEST_F(TextElementTest, LayoutUsesIntrinsicSize)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Hi");
	window_->Show();
	window_->Hide();

	auto visual = std::dynamic_pointer_cast<TextVisual>(text->CreateVisual(window_->GetView(), nullptr));
	ASSERT_NE(visual, nullptr);
	visual->BuildVisuals();
	visual->CalculateLayout(window_->Size());

	EXPECT_GT(visual->LayoutRect().Width(), 0.0f);
	EXPECT_GT(visual->LayoutRect().Height(), 0.0f);
}

TEST_F(TextElementTest, ViewTreeTextVisualHasLayout)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Hello");
	window_->Show();

	auto* rootVisual = window_->GetView()->Visual();
	ASSERT_NE(rootVisual, nullptr);
	ASSERT_FALSE(rootVisual->Visuals().empty());

	auto textVisual = std::dynamic_pointer_cast<TextVisual>(rootVisual->Visuals().front());
	ASSERT_NE(textVisual, nullptr) << "Root child should be TextVisual";

	window_->GetView()->Flush();

	const RectF layout = textVisual->LayoutRect();
	printf("TextVisual layout: l=%f t=%f r=%f b=%f w=%f h=%f text='%s'\n",
		layout.left, layout.top, layout.right, layout.bottom,
		layout.Width(), layout.Height(),
		reinterpret_cast<const char*>(text->Text().data()));
	EXPECT_GT(layout.Width(), 0.0f);
	EXPECT_GT(layout.Height(), 0.0f);

	text::Font font;
	font.SetFamily(u8"Segoe UI");
	font.SetSize(14.0f);
	FontMetrics metrics(font);
	EXPECT_GT(metrics.HorizontalAdvance(u8"Hello"), 0.0f);

	window_->Hide();
}

#include <gtest/gtest.h>
#include <limits>
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
					<Dimension width="200" height="40" />
					<TextAlignment horizontal="left" vertical="top" />
				</Style>
				<Style class="CenterText" inherit="FixedText">
					<TextAlignment horizontal="right" vertical="bottom" />
				</Style>
				<Style class="WrapText">
					<Font family="Segoe UI" size="14" style="normal" weight="normal" />
					<Colors color="#000000FF" />
					<Dimension width="120" />
					<TextAlignment horizontal="left" vertical="top" />
				</Style>
				<Style class="NarrowElideText">
					<Font family="Segoe UI" size="14" style="normal" weight="normal" />
					<Colors color="#000000FF" />
					<Dimension width="36" height="28" />
					<TextAlignment horizontal="left" vertical="top" />
				</Style>
				<Style class="MultiLineFixed">
					<Font family="Segoe UI" size="14" style="normal" weight="normal" />
					<Colors color="#000000FF" />
					<Dimension width="200" height="80" />
					<TextAlignment horizontal="left" vertical="top" />
				</Style>
				<Style class="MultiLineBottomRight" inherit="MultiLineFixed">
					<TextAlignment horizontal="right" vertical="bottom" />
				</Style>
			</Styles>
		)");
	}

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

	text->SetElideMode(TextElideMode::Left);
	EXPECT_EQ(text->ElideMode(), TextElideMode::Left);

	text->SetElideMode(TextElideMode::None);
	EXPECT_EQ(text->ElideMode(), TextElideMode::None);

	text->SetElideMode(TextElideMode::Right);
	EXPECT_EQ(text->ElideMode(), TextElideMode::Right);
}

TEST_F(TextElementTest, WrapModeRoundTrip)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Wrap me");
	EXPECT_EQ(text->WrapMode(), TextWrapMode::NoWrap);

	text->SetWrapMode(TextWrapMode::Wrap);
	EXPECT_EQ(text->WrapMode(), TextWrapMode::Wrap);

	text->SetWrapMode(TextWrapMode::NoWrap);
	EXPECT_EQ(text->WrapMode(), TextWrapMode::NoWrap);
}

TEST_F(TextElementTest, HardNewlineIncreasesLayoutHeight)
{
	TextElement* single = new TextElement(window_->RootElement(), u8"Hello");
	single->SetElideMode(TextElideMode::None);
	TextElement* multi = new TextElement(window_->RootElement(), u8"Hello\nWorld");
	multi->SetElideMode(TextElideMode::None);

	auto singleVisual = std::dynamic_pointer_cast<TextVisual>(
		single->CreateVisual(window_->GetView(), nullptr));
	auto multiVisual = std::dynamic_pointer_cast<TextVisual>(
		multi->CreateVisual(window_->GetView(), nullptr));
	ASSERT_NE(singleVisual, nullptr);
	ASSERT_NE(multiVisual, nullptr);
	singleVisual->BuildVisuals();
	multiVisual->BuildVisuals();
	// Yoga treats NaN as undefined; Exact available height would force stretch.
	const float undefined = std::numeric_limits<float>::quiet_NaN();
	singleVisual->CalculateLayout(SizeF(600.0f, undefined));
	multiVisual->CalculateLayout(SizeF(600.0f, undefined));

	EXPECT_GT(multiVisual->LayoutRect().Height(), singleVisual->LayoutRect().Height() * 1.5f);
}

TEST_F(TextElementTest, SoftWrapIncreasesLayoutHeight)
{
	TextElement* text = new TextElement(
		window_->RootElement(),
		u8"one two three four five six seven eight");
	text->SetStyleClass(u8"WrapText");
	text->SetWrapMode(TextWrapMode::Wrap);
	text->SetElideMode(TextElideMode::None);

	auto textVisual = std::dynamic_pointer_cast<TextVisual>(
		text->CreateVisual(window_->GetView(), nullptr));
	ASSERT_NE(textVisual, nullptr);
	textVisual->BuildVisuals();
	const float undefined = std::numeric_limits<float>::quiet_NaN();
	textVisual->CalculateLayout(SizeF(120.0f, undefined));

	Font font;
	font.SetFamily(u8"Segoe UI");
	font.SetSize(14.0f);
	FontMetrics metrics(font);
	EXPECT_GT(textVisual->LayoutRect().Height(), metrics.LineHeight() * 1.5f);
	EXPECT_FLOAT_EQ(textVisual->LayoutRect().Width(), 120.0f);
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

TEST_F(TextElementTest, HitTestHitsTextLineBoxOnly)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Hi");
	text->SetStyleClass(u8"FixedText");
	window_->Show();
	window_->GetView()->Flush();

	auto* rootVisual = window_->GetView()->Visual();
	ASSERT_NE(rootVisual, nullptr);
	auto textVisual = std::dynamic_pointer_cast<TextVisual>(rootVisual->Visuals().front());
	ASSERT_NE(textVisual, nullptr);

	Font font;
	font.SetFamily(u8"Segoe UI");
	font.SetSize(14.0f);
	FontMetrics metrics(font);
	const float textWidth = metrics.HorizontalAdvance(u8"Hi");
	ASSERT_GT(textWidth, 0.0f);
	ASSERT_LT(textWidth, textVisual->LayoutRect().Width());

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;

	EXPECT_TRUE(textVisual->HitTest(PointF(1.0f, metrics.Ascent() * 0.5f), &hitVisual, hitLocalPos));
	EXPECT_EQ(hitVisual, textVisual.get());

	EXPECT_FALSE(textVisual->HitTest(
		PointF(textWidth + 20.0f, metrics.Ascent() * 0.5f),
		&hitVisual,
		hitLocalPos));

	EXPECT_FALSE(textVisual->HitTest(
		PointF(1.0f, metrics.Ascent() + metrics.Descent() + 8.0f),
		&hitVisual,
		hitLocalPos));

	window_->Hide();
}

TEST_F(TextElementTest, HitTestEmptyTextMisses)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"");
	text->SetStyleClass(u8"FixedText");
	window_->Show();
	window_->GetView()->Flush();

	auto* rootVisual = window_->GetView()->Visual();
	auto textVisual = std::dynamic_pointer_cast<TextVisual>(rootVisual->Visuals().front());
	ASSERT_NE(textVisual, nullptr);

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;
	EXPECT_FALSE(textVisual->HitTest(PointF(1.0f, 1.0f), &hitVisual, hitLocalPos));

	window_->Hide();
}

TEST_F(TextElementTest, FontMetricsDecorationOffsetsAreSane)
{
	Font font;
	font.SetFamily(u8"Segoe UI");
	font.SetSize(16.0f);
	FontMetrics metrics(font);

	EXPECT_GT(metrics.UnderlineThickness(), 0.0f);
	EXPECT_GT(metrics.StrikeoutThickness(), 0.0f);
	EXPECT_GT(metrics.UnderlineOffset(), 0.0f);
	EXPECT_LT(metrics.StrikeoutOffset(), 0.0f);
	EXPECT_GT(metrics.XHeight(), 0.0f);
	EXPECT_LT(std::abs(metrics.StrikeoutOffset()), metrics.Ascent());
	EXPECT_LT(metrics.UnderlineOffset(), metrics.Descent() + metrics.UnderlineThickness());
}

TEST_F(TextElementTest, HitTestRespectsTextAlignment)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Hi");
	text->SetStyleClass(u8"CenterText");
	window_->Show();
	window_->GetView()->Flush();

	auto* rootVisual = window_->GetView()->Visual();
	auto textVisual = std::dynamic_pointer_cast<TextVisual>(rootVisual->Visuals().front());
	ASSERT_NE(textVisual, nullptr);

	const RectF layout = textVisual->LayoutRect();
	Font font;
	font.SetFamily(u8"Segoe UI");
	font.SetSize(14.0f);
	FontMetrics metrics(font);
	const float textWidth = metrics.HorizontalAdvance(u8"Hi");

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;

	// Right + bottom aligned: top-left of layout should miss.
	EXPECT_FALSE(textVisual->HitTest(PointF(1.0f, 1.0f), &hitVisual, hitLocalPos));

	const float hitX = layout.Width() - textWidth * 0.5f;
	const float hitY = layout.Height() - metrics.Descent() * 0.5f;
	EXPECT_TRUE(textVisual->HitTest(PointF(hitX, hitY), &hitVisual, hitLocalPos));
	EXPECT_EQ(hitVisual, textVisual.get());

	window_->Hide();
}

// View Flush path: NoWrap elide with multibyte glyphs must finish at narrow widths
// (binary-search UTF-8 boundary bug used to hang paint/hit). Also cover hard-newline
// and soft-wrap hit targets on the second visual line.

TEST_F(TextElementTest, NarrowNoWrapMultibyteElideFlushDoesNotHang)
{
	TextElement* text = new TextElement(
		window_->RootElement(),
		u8"Multi-line text — hard break demo");
	text->SetStyleClass(u8"NarrowElideText");

	window_->Show();
	for (TextElideMode mode : {
		TextElideMode::Right,
		TextElideMode::Left,
		TextElideMode::Middle })
	{
		text->SetElideMode(mode);
		window_->GetView()->MarkDirty();
		window_->GetView()->Flush();
	}

	auto* rootVisual = window_->GetView()->Visual();
	auto textVisual = std::dynamic_pointer_cast<TextVisual>(rootVisual->Visuals().front());
	ASSERT_NE(textVisual, nullptr);
	EXPECT_FLOAT_EQ(textVisual->LayoutRect().Width(), 36.0f);

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;
	EXPECT_TRUE(textVisual->HitTest(PointF(2.0f, 8.0f), &hitVisual, hitLocalPos));

	window_->Hide();
}

TEST_F(TextElementTest, HitTestHardNewlineSecondLine)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Hello\nWorld");
	text->SetStyleClass(u8"MultiLineFixed");
	text->SetElideMode(TextElideMode::None);

	window_->Show();
	window_->GetView()->Flush();

	auto* rootVisual = window_->GetView()->Visual();
	auto textVisual = std::dynamic_pointer_cast<TextVisual>(rootVisual->Visuals().front());
	ASSERT_NE(textVisual, nullptr);

	Font font;
	font.SetFamily(u8"Segoe UI");
	font.SetSize(14.0f);
	FontMetrics metrics(font);
	const float lineHeight = metrics.LineHeight();
	const float textWidth = metrics.HorizontalAdvance(u8"Hello");

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;

	EXPECT_TRUE(textVisual->HitTest(
		PointF(1.0f, metrics.Ascent() * 0.5f), &hitVisual, hitLocalPos));
	EXPECT_EQ(hitVisual, textVisual.get());

	EXPECT_TRUE(textVisual->HitTest(
		PointF(1.0f, lineHeight + metrics.Ascent() * 0.5f), &hitVisual, hitLocalPos));
	EXPECT_EQ(hitVisual, textVisual.get());

	EXPECT_FALSE(textVisual->HitTest(
		PointF(textWidth + 30.0f, metrics.Ascent() * 0.5f), &hitVisual, hitLocalPos));
	EXPECT_FALSE(textVisual->HitTest(
		PointF(1.0f, lineHeight * 2.0f + 4.0f), &hitVisual, hitLocalPos));

	window_->Hide();
}

TEST_F(TextElementTest, HitTestSoftWrapSecondLine)
{
	TextElement* text = new TextElement(
		window_->RootElement(),
		u8"one two three four five six seven eight");
	text->SetStyleClass(u8"WrapText");
	text->SetWrapMode(TextWrapMode::Wrap);
	text->SetElideMode(TextElideMode::None);

	window_->Show();
	window_->GetView()->Flush();

	auto* rootVisual = window_->GetView()->Visual();
	auto textVisual = std::dynamic_pointer_cast<TextVisual>(rootVisual->Visuals().front());
	ASSERT_NE(textVisual, nullptr);

	Font font;
	font.SetFamily(u8"Segoe UI");
	font.SetSize(14.0f);
	FontMetrics metrics(font);
	ASSERT_GT(textVisual->LayoutRect().Height(), metrics.LineHeight() * 1.5f);

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;
	EXPECT_TRUE(textVisual->HitTest(
		PointF(2.0f, metrics.Ascent() * 0.5f), &hitVisual, hitLocalPos));
	EXPECT_TRUE(textVisual->HitTest(
		PointF(2.0f, metrics.LineHeight() + metrics.Ascent() * 0.5f),
		&hitVisual,
		hitLocalPos));
	EXPECT_FALSE(textVisual->HitTest(
		PointF(2.0f, textVisual->LayoutRect().Height() + 8.0f),
		&hitVisual,
		hitLocalPos));

	window_->Hide();
}

// Wrap/elide interaction: Wrap skips per-line elide; NoWrap still elides each hard
// line; SetWrapMode/SetElideMode fire ElementTextChanged only on real changes.
// Alignment and shrink-resize Flush cover multi-line layout under the view tree.

TEST_F(TextElementTest, WrapModeChangeFiresTextChangedAndIgnoresSameValue)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Wrap me");
	TextChangeWatcher watcher(text);

	text->SetWrapMode(TextWrapMode::NoWrap);
	EXPECT_EQ(watcher.changeCount, 0);

	text->SetWrapMode(TextWrapMode::Wrap);
	EXPECT_EQ(watcher.changeCount, 1);

	text->SetWrapMode(TextWrapMode::Wrap);
	EXPECT_EQ(watcher.changeCount, 1);
}

TEST_F(TextElementTest, ElideModeChangeFiresTextChangedAndIgnoresSameValue)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Elide me");
	TextChangeWatcher watcher(text);

	text->SetElideMode(TextElideMode::Right);
	EXPECT_EQ(watcher.changeCount, 0);

	text->SetElideMode(TextElideMode::None);
	EXPECT_EQ(watcher.changeCount, 1);

	text->SetElideMode(TextElideMode::None);
	EXPECT_EQ(watcher.changeCount, 1);
}

TEST_F(TextElementTest, WrapSkipsElideAndGrowsHeight)
{
	const std::u8string longText =
		u8"alpha bravo charlie delta echo foxtrot golf hotel india juliet";
	TextElement* wrap = new TextElement(window_->RootElement(), longText);
	wrap->SetStyleClass(u8"WrapText");
	wrap->SetWrapMode(TextWrapMode::Wrap);
	wrap->SetElideMode(TextElideMode::Right);

	TextElement* noWrap = new TextElement(window_->RootElement(), longText);
	noWrap->SetStyleClass(u8"WrapText");
	noWrap->SetWrapMode(TextWrapMode::NoWrap);
	noWrap->SetElideMode(TextElideMode::Right);

	auto wrapVisual = std::dynamic_pointer_cast<TextVisual>(
		wrap->CreateVisual(window_->GetView(), nullptr));
	auto noWrapVisual = std::dynamic_pointer_cast<TextVisual>(
		noWrap->CreateVisual(window_->GetView(), nullptr));
	ASSERT_NE(wrapVisual, nullptr);
	ASSERT_NE(noWrapVisual, nullptr);
	wrapVisual->BuildVisuals();
	noWrapVisual->BuildVisuals();
	const float undefined = std::numeric_limits<float>::quiet_NaN();
	wrapVisual->CalculateLayout(SizeF(120.0f, undefined));
	noWrapVisual->CalculateLayout(SizeF(120.0f, undefined));

	Font font;
	font.SetFamily(u8"Segoe UI");
	font.SetSize(14.0f);
	FontMetrics metrics(font);
	EXPECT_GT(wrapVisual->LayoutRect().Height(), metrics.LineHeight() * 1.5f);
	EXPECT_NEAR(noWrapVisual->LayoutRect().Height(), metrics.LineHeight(), 1.0f);
}

TEST_F(TextElementTest, NoWrapHardNewlineElidesPerLine)
{
	TextElement* text = new TextElement(
		window_->RootElement(),
		u8"AAAAAAAAAAAAAAA\nBBBBBBBBBBBBBBB");
	text->SetStyleClass(u8"MultiLineFixed");
	text->SetWrapMode(TextWrapMode::NoWrap);
	text->SetElideMode(TextElideMode::Right);

	window_->Show();
	window_->GetView()->Flush();

	auto* rootVisual = window_->GetView()->Visual();
	auto textVisual = std::dynamic_pointer_cast<TextVisual>(rootVisual->Visuals().front());
	ASSERT_NE(textVisual, nullptr);

	Font font;
	font.SetFamily(u8"Segoe UI");
	font.SetSize(14.0f);
	FontMetrics metrics(font);
	const float lineHeight = metrics.LineHeight();

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;
	EXPECT_TRUE(textVisual->HitTest(
		PointF(4.0f, metrics.Ascent() * 0.5f), &hitVisual, hitLocalPos));
	EXPECT_TRUE(textVisual->HitTest(
		PointF(4.0f, lineHeight + metrics.Ascent() * 0.5f), &hitVisual, hitLocalPos));
	EXPECT_FALSE(textVisual->HitTest(
		PointF(4.0f, lineHeight * 2.0f + 6.0f), &hitVisual, hitLocalPos));

	window_->Hide();
}

TEST_F(TextElementTest, HitTestMultilineBottomRightAlignment)
{
	TextElement* text = new TextElement(window_->RootElement(), u8"Hi\nYo");
	text->SetStyleClass(u8"MultiLineBottomRight");
	text->SetElideMode(TextElideMode::None);

	window_->Show();
	window_->GetView()->Flush();

	auto* rootVisual = window_->GetView()->Visual();
	auto textVisual = std::dynamic_pointer_cast<TextVisual>(rootVisual->Visuals().front());
	ASSERT_NE(textVisual, nullptr);

	const RectF layout = textVisual->LayoutRect();
	Font font;
	font.SetFamily(u8"Segoe UI");
	font.SetSize(14.0f);
	FontMetrics metrics(font);
	const float lineHeight = metrics.LineHeight();
	const float textWidth = metrics.HorizontalAdvance(u8"Hi");

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;
	EXPECT_FALSE(textVisual->HitTest(PointF(1.0f, 1.0f), &hitVisual, hitLocalPos));

	const float hitX = layout.Width() - textWidth * 0.5f;
	const float hitY = layout.Height() - lineHeight + metrics.Ascent() * 0.5f;
	EXPECT_TRUE(textVisual->HitTest(PointF(hitX, hitY), &hitVisual, hitLocalPos));
	EXPECT_EQ(hitVisual, textVisual.get());

	window_->Hide();
}

TEST_F(TextElementTest, NarrowResizeFlushDoesNotHang)
{
	TextElement* text = new TextElement(
		window_->RootElement(),
		u8"Multi-line text — resize stress alpha bravo charlie");
	text->SetStyleClass(u8"NarrowElideText");
	text->SetElideMode(TextElideMode::Right);

	window_->Show();
	for (float w = 600.0f; w >= 80.0f; w -= 40.0f)
	{
		window_->Resize(SizeF(w, 400.0f));
		window_->GetView()->MarkDirty();
		window_->GetView()->Flush();
	}

	auto* rootVisual = window_->GetView()->Visual();
	ASSERT_NE(rootVisual, nullptr);
	EXPECT_FALSE(rootVisual->Visuals().empty());
	window_->Hide();
}

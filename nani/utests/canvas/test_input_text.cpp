#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "defs.h"
#include "canvas/elements/input_text_element.h"
#include "canvas/visuals/input_text_visual.h"
#include "canvas/events/event.h"

using namespace nani::canvas;
using namespace nani::canvas::elements;
using namespace nani::canvas::events;
using namespace nani::canvas::visuals;
using namespace nani::canvas::basic;

namespace
{
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

	class RequestWatcher : public EventFilter
	{
	public:
		explicit RequestWatcher(EventTarget* target)
			: target_(target)
		{
			if (target_)
				target_->RegisterEventFilter(this);
		}

		~RequestWatcher()
		{
			if (target_)
				target_->UnRegisterEventFilter(this);
		}

		bool Filter(EventTarget*, Event* e) override
		{
			if (e->type == Type::LayoutRequest || e->type == Type::PaintRequest)
				++requestCount;
			return false;
		}

		EventTarget* target_ = nullptr;
		int requestCount = 0;
	};
}

class InputTextTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		env_ = new Env(0, nullptr);
		window_ = new Window(PointF(), SizeF(600, 400));
		window_->RootElement()->GetStyles()->LoadFromXML(R"(
			<Styles>
				<Style class="NaniWindow">
					<FlexBox flexDirection="column" />
				</Style>
				<Style class="DefaultInputText">
					<Font family="Segoe UI" size="14" style="normal" weight="normal" />
					<Dimension width="200" height="32" />
					<Paddings l="8" t="4" r="8" b="4" />
					<Borders value="1" />
					<Colors background="#FFFFFFFF" border="#94A3B8FF" color="#000000FF" selection-background="#2563EBFF" selection-color="#FFFFFFFF" />
				</Style>
				<Style class="DefaultInputText" state="focused">
					<Colors border="#2563EBFF" />
				</Style>
			</Styles>
		)");
	}

	void TearDown() override
	{
		delete window_;
		delete env_;
	}

	void BuildAndFlush()
	{
		window_->GetView()->BuildVisuals();
		window_->GetView()->Flush();
	}

	void ClickAt(const PointF& pos)
	{
		MousePressEvent press(MouseButton::Left, pos, pos);
		window_->FireEvent(&press);
	}

	void ProcessEvents()
	{
		env_->ProcessEvents();
	}

	Window* window_ = nullptr;

private:
	Env* env_ = nullptr;
};

TEST_F(InputTextTest, CreateVisualType)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"hi");
	auto visual = input->CreateVisual(window_->GetView(), nullptr);
	ASSERT_NE(visual, nullptr);
	EXPECT_NE(dynamic_cast<InputTextVisual*>(visual.get()), nullptr);
}

TEST_F(InputTextTest, DefaultStyleClassIsDefaultInputText)
{
	auto* input = new InputTextElement(window_->RootElement());
	EXPECT_EQ(input->StyleClass(), u8"DefaultInputText");
}

TEST_F(InputTextTest, SetTextFiresChangedAndClampsCaret)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"hello");
	TextChangeWatcher watcher(input);

	input->SetCaretIndex(5);
	EXPECT_EQ(input->CaretIndex(), 5u);

	input->SetText(u8"hi");
	EXPECT_EQ(input->Text(), u8"hi");
	EXPECT_EQ(input->CaretIndex(), 2u);
	EXPECT_GE(watcher.changeCount, 1);
}

TEST_F(InputTextTest, CharAndKeyEditing)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"");

	CharEvent a(U'a');
	input->FireEvent(&a);
	CharEvent b(U'b');
	input->FireEvent(&b);
	EXPECT_EQ(input->Text(), u8"ab");
	EXPECT_EQ(input->CaretIndex(), 2u);

	KeyPressEvent left(Key::Left);
	input->FireEvent(&left);
	EXPECT_EQ(input->CaretIndex(), 1u);

	CharEvent x(U'x');
	input->FireEvent(&x);
	EXPECT_EQ(input->Text(), u8"axb");

	KeyPressEvent backspace(Key::Backspace);
	input->FireEvent(&backspace);
	EXPECT_EQ(input->Text(), u8"ab");

	KeyPressEvent home(Key::Home);
	input->FireEvent(&home);
	EXPECT_EQ(input->CaretIndex(), 0u);

	KeyPressEvent end(Key::End);
	input->FireEvent(&end);
	EXPECT_EQ(input->CaretIndex(), 2u);

	input->SetCaretIndex(1);
	KeyPressEvent del(Key::Delete);
	input->FireEvent(&del);
	EXPECT_EQ(input->Text(), u8"a");
}

TEST_F(InputTextTest, Utf8CaretNavigation)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"你a好");
	input->SetCaretIndex(input->Text().size());

	KeyPressEvent left(Key::Left);
	input->FireEvent(&left);
	EXPECT_EQ(input->Text().substr(input->CaretIndex()), u8"好");

	KeyPressEvent backspace(Key::Backspace);
	input->FireEvent(&backspace);
	EXPECT_EQ(input->Text(), u8"你好");
}

TEST_F(InputTextTest, EditingBoundariesDoNotCrashOrMutate)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"");
	EXPECT_EQ(input->CaretIndex(), 0u);

	KeyPressEvent backspace(Key::Backspace);
	input->FireEvent(&backspace);
	KeyPressEvent del(Key::Delete);
	input->FireEvent(&del);
	KeyPressEvent left(Key::Left);
	input->FireEvent(&left);
	KeyPressEvent right(Key::Right);
	input->FireEvent(&right);
	EXPECT_TRUE(input->Text().empty());
	EXPECT_EQ(input->CaretIndex(), 0u);

	input->SetText(u8"ab");
	input->SetCaretIndex(2);
	input->FireEvent(&right);
	input->FireEvent(&del);
	EXPECT_EQ(input->Text(), u8"ab");
	EXPECT_EQ(input->CaretIndex(), 2u);

	input->SetCaretIndex(0);
	input->FireEvent(&left);
	EXPECT_EQ(input->CaretIndex(), 0u);
}

TEST_F(InputTextTest, RepeatedKeyPressMovesAndDeletesContinuously)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"abcd");
	input->SetCaretIndex(4);

	KeyPressEvent left(Key::Left);
	input->FireEvent(&left);
	input->FireEvent(&left);
	EXPECT_EQ(input->CaretIndex(), 2u);

	KeyPressEvent backspace(Key::Backspace);
	input->FireEvent(&backspace);
	input->FireEvent(&backspace);
	EXPECT_EQ(input->Text(), u8"cd");
	EXPECT_EQ(input->CaretIndex(), 0u);

	KeyPressEvent del(Key::Delete);
	input->FireEvent(&del);
	input->FireEvent(&del);
	EXPECT_TRUE(input->Text().empty());
	EXPECT_EQ(input->CaretIndex(), 0u);
}

TEST_F(InputTextTest, ControlCharIsIgnoredExceptTab)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"");

	CharEvent newline(U'\n');
	input->FireEvent(&newline);
	CharEvent ctrlA(U'\x01');
	input->FireEvent(&ctrlA);
	EXPECT_TRUE(input->Text().empty());

	CharEvent tab(U'\t');
	input->FireEvent(&tab);
	EXPECT_EQ(input->Text(), u8"\t");
}

TEST_F(InputTextTest, CaretIndexAlignsToUtf8Boundary)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"你好");
	EXPECT_EQ(input->Text().size(), 6u);

	input->SetCaretIndex(1);
	EXPECT_EQ(input->CaretIndex(), 0u);

	input->SetCaretIndex(4);
	EXPECT_EQ(input->CaretIndex(), 3u);

	input->SetCaretIndex(100);
	EXPECT_EQ(input->CaretIndex(), 6u);
}

TEST_F(InputTextTest, ImeCompositionPreedit)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"hi");
	input->SetCaretIndex(2);
	TextChangeWatcher watcher(input);

	ImeCompositionStartEvent start;
	input->FireEvent(&start);
	EXPECT_TRUE(input->IsComposing());

	ImeCompositionUpdateEvent update(u8"ni");
	input->FireEvent(&update);
	EXPECT_EQ(input->PreeditText(), u8"ni");
	EXPECT_EQ(input->Text(), u8"hi");
	EXPECT_GE(watcher.changeCount, 1);

	ImeCompositionEndEvent end;
	input->FireEvent(&end);
	EXPECT_TRUE(input->PreeditText().empty());
	EXPECT_FALSE(input->IsComposing());

	CharEvent n(U'你');
	input->FireEvent(&n);
	EXPECT_EQ(input->Text(), u8"hi你");
}

TEST_F(InputTextTest, EndCompositionClearsPreedit)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"ab");

	ImeCompositionUpdateEvent update(u8"hao");
	input->FireEvent(&update);
	EXPECT_EQ(input->PreeditText(), u8"hao");

	input->EndComposition();
	EXPECT_TRUE(input->PreeditText().empty());
	EXPECT_FALSE(input->IsComposing());
}

TEST_F(InputTextTest, KeyIgnoredWhileComposing)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"ab");
	input->SetCaretIndex(2);

	ImeCompositionStartEvent start;
	input->FireEvent(&start);
	ImeCompositionUpdateEvent update(u8"ni");
	input->FireEvent(&update);

	KeyPressEvent left(Key::Left);
	input->FireEvent(&left);
	KeyPressEvent backspace(Key::Backspace);
	input->FireEvent(&backspace);

	EXPECT_EQ(input->Text(), u8"ab");
	EXPECT_EQ(input->CaretIndex(), 2u);
	EXPECT_EQ(input->PreeditText(), u8"ni");
}

TEST_F(InputTextTest, CharCommitClearsPreeditWithoutEndEvent)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"hi");
	input->SetCaretIndex(2);

	ImeCompositionUpdateEvent update(u8"hao");
	input->FireEvent(&update);
	EXPECT_EQ(input->PreeditText(), u8"hao");

	CharEvent committed(U'好');
	input->FireEvent(&committed);
	EXPECT_TRUE(input->PreeditText().empty());
	EXPECT_FALSE(input->IsComposing());
	EXPECT_EQ(input->Text(), u8"hi好");
}

TEST_F(InputTextTest, FocusViaMousePress)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"focus");
	BuildAndFlush();

	const RectF client = window_->ClientRect();
	ClickAt(PointF(client.left + 20.0f, client.top + 16.0f));
	EXPECT_TRUE(input->States()->IsFocused());
}

TEST_F(InputTextTest, ViewRoutesKeyAndCharToFocusedInput)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"");
	BuildAndFlush();

	const RectF client = window_->ClientRect();
	ClickAt(PointF(client.left + 20.0f, client.top + 16.0f));
	ASSERT_TRUE(input->States()->IsFocused());

	CharEvent a(U'a');
	window_->FireEvent(&a);
	// Printable letters arrive via Char; bare Key_B should not insert.
	KeyPressEvent bKey(Key::B);
	window_->FireEvent(&bKey);
	EXPECT_EQ(input->Text(), u8"a");

	KeyPressEvent backspace(Key::Backspace);
	window_->FireEvent(&backspace);
	EXPECT_TRUE(input->Text().empty());
}

TEST_F(InputTextTest, FocusSwitchAndBlurClearComposition)
{
	window_->RootElement()->GetStyles()->LoadFromXML(R"(
		<Styles>
			<Style class="NaniWindow">
				<FlexBox flexDirection="column" />
				<Dimension width="600" height="400" />
			</Style>
			<Style class="InputTop">
				<Font family="Segoe UI" size="14" />
				<Dimension width="200" height="32" />
				<Positions type="absolute" l="10" t="10" />
				<Colors background="#FFFFFFFF" border="#000000FF" color="#000000FF" />
				<Borders value="1" />
			</Style>
			<Style class="InputBottom">
				<Font family="Segoe UI" size="14" />
				<Dimension width="200" height="32" />
				<Positions type="absolute" l="10" t="60" />
				<Colors background="#FFFFFFFF" border="#000000FF" color="#000000FF" />
				<Borders value="1" />
			</Style>
		</Styles>
	)");

	auto* first = new InputTextElement(window_->RootElement(), u8"one");
	first->SetStyleClass(u8"InputTop");
	auto* second = new InputTextElement(window_->RootElement(), u8"two");
	second->SetStyleClass(u8"InputBottom");
	BuildAndFlush();

	ClickAt(PointF(30.0f, 20.0f));
	ASSERT_TRUE(first->States()->IsFocused());
	ASSERT_FALSE(second->States()->IsFocused());

	ImeCompositionUpdateEvent update(u8"ni");
	window_->FireEvent(&update);
	EXPECT_EQ(first->PreeditText(), u8"ni");

	ClickAt(PointF(30.0f, 70.0f));
	EXPECT_FALSE(first->States()->IsFocused());
	EXPECT_TRUE(second->States()->IsFocused());
	EXPECT_TRUE(first->PreeditText().empty());
	EXPECT_FALSE(first->IsComposing());

	ClickAt(PointF(500.0f, 300.0f));
	EXPECT_FALSE(second->States()->IsFocused());
}

TEST_F(InputTextTest, VisualLayoutAndHitTestCoverContentBox)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"layout");
	window_->Show();
	window_->GetView()->Flush();

	auto* rootVisual = window_->GetView()->Visual();
	ASSERT_NE(rootVisual, nullptr);
	ASSERT_FALSE(rootVisual->Visuals().empty());

	auto* inputVisual = dynamic_cast<InputTextVisual*>(rootVisual->Visuals().front().get());
	ASSERT_NE(inputVisual, nullptr);
	EXPECT_FLOAT_EQ(inputVisual->LayoutRect().Width(), 200.0f);
	EXPECT_FLOAT_EQ(inputVisual->LayoutRect().Height(), 32.0f);

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;
	EXPECT_TRUE(inputVisual->HitTest(PointF(10.0f, 10.0f), &hitVisual, hitLocalPos));
	EXPECT_EQ(hitVisual, inputVisual);
	EXPECT_TRUE(inputVisual->HitTest(PointF(190.0f, 28.0f), &hitVisual, hitLocalPos));

	window_->Hide();
}

TEST_F(InputTextTest, TextChangeMarksViewDirtyViaVisualFilter)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"a");
	BuildAndFlush();

	RequestWatcher watcher(window_->GetView());
	input->SetText(u8"bc");
	EXPECT_GT(watcher.requestCount, 0);

	const int afterText = watcher.requestCount;
	ImeCompositionUpdateEvent update(u8"x");
	input->FireEvent(&update);
	EXPECT_GT(watcher.requestCount, afterText);
}

TEST_F(InputTextTest, SelectionApiAndShiftExtend)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"abcd");
	EXPECT_FALSE(input->HasSelection());

	input->SetSelection(1, 3);
	EXPECT_TRUE(input->HasSelection());
	EXPECT_EQ(input->AnchorIndex(), 1u);
	EXPECT_EQ(input->CaretIndex(), 3u);
	EXPECT_EQ(input->SelectionStart(), 1u);
	EXPECT_EQ(input->SelectionEnd(), 3u);

	input->SetCaretIndex(2);
	EXPECT_FALSE(input->HasSelection());
	EXPECT_EQ(input->AnchorIndex(), 2u);
	EXPECT_EQ(input->CaretIndex(), 2u);

	input->SetCaretIndex(0);
	KeyPressEvent right(Key::Right, Modifier::Shift);
	input->FireEvent(&right);
	input->FireEvent(&right);
	EXPECT_TRUE(input->HasSelection());
	EXPECT_EQ(input->AnchorIndex(), 0u);
	EXPECT_EQ(input->CaretIndex(), 2u);

	KeyPressEvent left(Key::Left);
	input->FireEvent(&left);
	EXPECT_FALSE(input->HasSelection());
	EXPECT_EQ(input->CaretIndex(), 0u);
}

TEST_F(InputTextTest, SelectionReplaceAndDelete)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"abcd");
	input->SetSelection(1, 3);

	CharEvent x(U'x');
	input->FireEvent(&x);
	EXPECT_EQ(input->Text(), u8"axd");
	EXPECT_FALSE(input->HasSelection());
	EXPECT_EQ(input->CaretIndex(), 2u);

	input->SetSelection(0, 2);
	KeyPressEvent backspace(Key::Backspace);
	input->FireEvent(&backspace);
	EXPECT_EQ(input->Text(), u8"d");
	EXPECT_EQ(input->CaretIndex(), 0u);

	input->SetText(u8"abcd");
	input->SetSelection(1, 3);
	KeyPressEvent del(Key::Delete);
	input->FireEvent(&del);
	EXPECT_EQ(input->Text(), u8"ad");
}

TEST_F(InputTextTest, SelectAllWithCtrlA)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"hello");
	KeyPressEvent selectAll(Key::A, Modifier::Ctrl);
	input->FireEvent(&selectAll);
	EXPECT_TRUE(input->HasSelection());
	EXPECT_EQ(input->SelectionStart(), 0u);
	EXPECT_EQ(input->SelectionEnd(), 5u);
}

TEST_F(InputTextTest, HomeEndWithShiftSelects)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"abcd");
	input->SetCaretIndex(2);

	KeyPressEvent home(Key::Home, Modifier::Shift);
	input->FireEvent(&home);
	EXPECT_EQ(input->SelectionStart(), 0u);
	EXPECT_EQ(input->SelectionEnd(), 2u);

	KeyPressEvent end(Key::End, Modifier::Shift);
	input->FireEvent(&end);
	EXPECT_EQ(input->SelectionStart(), 2u);
	EXPECT_EQ(input->SelectionEnd(), 4u);
}

TEST_F(InputTextTest, MouseDragSelectsText)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"abcdef");
	BuildAndFlush();

	const RectF client = window_->ClientRect();
	ClickAt(PointF(client.left + 20.0f, client.top + 16.0f));
	ASSERT_TRUE(input->States()->IsFocused());

	auto* rootVisual = window_->GetView()->Visual();
	ASSERT_NE(rootVisual, nullptr);
	ASSERT_FALSE(rootVisual->Visuals().empty());
	auto* inputVisual = dynamic_cast<InputTextVisual*>(rootVisual->Visuals().front().get());
	ASSERT_NE(inputVisual, nullptr);

	MousePressEvent press(MouseButton::Left, PointF(12.0f, 16.0f), PointF(12.0f, 16.0f));
	input->FireEvent(&press);
	MouseMoveEvent move(PointF(80.0f, 16.0f), PointF(80.0f, 16.0f));
	input->FireEvent(&move);
	MouseReleaseEvent release(MouseButton::Left, PointF(80.0f, 16.0f), PointF(80.0f, 16.0f));
	input->FireEvent(&release);

	EXPECT_TRUE(input->HasSelection());
	EXPECT_LT(input->SelectionStart(), input->SelectionEnd());
}

TEST_F(InputTextTest, FocusStartsCaretBlinkRepaint)
{
	auto* input = new InputTextElement(window_->RootElement(), u8"blink");
	BuildAndFlush();

	RequestWatcher watcher(window_->GetView());
	const RectF client = window_->ClientRect();
	ClickAt(PointF(client.left + 20.0f, client.top + 16.0f));
	ASSERT_TRUE(input->States()->IsFocused());
	EXPECT_GT(watcher.requestCount, 0);

	const int afterFocus = watcher.requestCount;
	std::this_thread::sleep_for(std::chrono::milliseconds(560));
	ProcessEvents();
	EXPECT_GT(watcher.requestCount, afterFocus);
}

TEST_F(InputTextTest, ScrollFollowsCaretWhenTextOverflows)
{
	window_->RootElement()->GetStyles()->LoadFromXML(R"(
		<Styles>
			<Style class="NaniWindow">
				<FlexBox flexDirection="column" />
			</Style>
			<Style class="NarrowInput">
				<Font family="Segoe UI" size="14" style="normal" weight="normal" />
				<Dimension width="80" height="32" />
				<Paddings l="4" t="4" r="4" b="4" />
				<Borders value="1" />
				<Colors background="#FFFFFFFF" border="#000000FF" color="#000000FF" />
			</Style>
		</Styles>
	)");

	auto* input = new InputTextElement(
		window_->RootElement(),
		u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	input->SetStyleClass(u8"NarrowInput");
	input->SetCaretIndex(input->Text().size());
	window_->Show();
	window_->GetView()->Flush();

	auto* rootVisual = window_->GetView()->Visual();
	ASSERT_NE(rootVisual, nullptr);
	ASSERT_FALSE(rootVisual->Visuals().empty());
	auto* inputVisual = dynamic_cast<InputTextVisual*>(rootVisual->Visuals().front().get());
	ASSERT_NE(inputVisual, nullptr);

	EXPECT_GT(inputVisual->ScrollOffset(), 0.0f);

	KeyPressEvent home(Key::Home);
	input->FireEvent(&home);
	window_->GetView()->Flush();
	EXPECT_FLOAT_EQ(inputVisual->ScrollOffset(), 0.0f);

	KeyPressEvent end(Key::End);
	input->FireEvent(&end);
	window_->GetView()->Flush();
	EXPECT_GT(inputVisual->ScrollOffset(), 0.0f);

	window_->Hide();
}

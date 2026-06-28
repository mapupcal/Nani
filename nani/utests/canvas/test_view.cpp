#include <gtest/gtest.h>
#include "defs.h"

class ViewTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		env_ = new Env(0, nullptr);
		window = new Window(PointF(), SizeF(600, 400));
		window->RootElement()->GetStyles()->LoadFromXML(R"(
			<Styles>
				<Style class="NaniWindow">
					<FlexDirection value="row"/>
				</Style>
				<Style class="Rectangle">
					<Width value="200"/>
					<Height value="100"/>
				</Style>
				<Style class="Square">
					<Width value="100"/>
					<Height value="100"/>
				</Style>
			</Styles>
		)");
	}

	void TearDown() override
	{
		delete window;
		delete env_;
	}

public:
	Window* window = nullptr;
	const char8_t* StyleClassSquare = u8"Square";
	const char8_t* StyleClassRectangle = u8"Rectangle";

private:
	Env* env_ = nullptr;
};

class ElementHitTestWatcher : public EventFilter
{
public:
	ElementHitTestWatcher(Element* element)
		: element_(element)
	{
		if (element_)
			element_->RegisterEventFilter(this);
	}
	~ElementHitTestWatcher()
	{
		if (element_)
			element_->UnRegisterEventFilter(this);
	}

	virtual bool Filter(EventTarget* target, Event* e)
	{
		if (e->type == Type::Enter)
			EnterCount++;
		if (e->type == Type::Leave)
			LeaveCount++;
		return false;
	}

public:
	Element* element_ = nullptr;
	int EnterCount = 0;
	int LeaveCount = 0;
};

TEST_F(ViewTest, HitTest)
{
	Element* squareLeft = new Element(window->RootElement());
	squareLeft->SetStyleClass(StyleClassSquare);

	Element* rectangle = new Element(window->RootElement());
	rectangle->SetStyleClass(StyleClassRectangle);

	Element* squareRight = new Element(window->RootElement());
	squareRight->SetStyleClass(StyleClassSquare);

	ElementHitTestWatcher leftWatcher(squareLeft);
	ElementHitTestWatcher midWatcher(rectangle);
	ElementHitTestWatcher rightWatcher(squareRight);

	window->Show();
	window->Hide();

	{
		MouseMoveEvent e(PointF(0, 0), Cursor::Pos());
		window->FireEvent(&e);

		EXPECT_EQ(leftWatcher.EnterCount, 1);
		EXPECT_EQ(leftWatcher.LeaveCount, 0);
		EXPECT_EQ(midWatcher.EnterCount, 0);
		EXPECT_EQ(midWatcher.LeaveCount, 0);
		EXPECT_EQ(rightWatcher.EnterCount, 0);
		EXPECT_EQ(rightWatcher.LeaveCount, 0);
	}


	{
		MouseMoveEvent e(PointF(100, 0), Cursor::Pos());
		window->FireEvent(&e);

		EXPECT_EQ(leftWatcher.EnterCount, 1);
		EXPECT_EQ(leftWatcher.LeaveCount, 1);
		EXPECT_EQ(midWatcher.EnterCount, 1);
		EXPECT_EQ(midWatcher.LeaveCount, 0);
		EXPECT_EQ(rightWatcher.EnterCount, 0);
		EXPECT_EQ(rightWatcher.LeaveCount, 0);
	}

	{
		MouseMoveEvent e(PointF(300, 0), Cursor::Pos());
		window->FireEvent(&e);

		EXPECT_EQ(leftWatcher.EnterCount, 1);
		EXPECT_EQ(leftWatcher.LeaveCount, 1);
		EXPECT_EQ(midWatcher.EnterCount, 1);
		EXPECT_EQ(midWatcher.LeaveCount, 1);
		EXPECT_EQ(rightWatcher.EnterCount, 1);
		EXPECT_EQ(rightWatcher.LeaveCount, 0);

		MouseMoveEvent le(PointF(-1, -1), Cursor::Pos());
		window->FireEvent(&le);

		EXPECT_EQ(leftWatcher.EnterCount, 1);
		EXPECT_EQ(leftWatcher.LeaveCount, 1);
		EXPECT_EQ(midWatcher.EnterCount, 1);
		EXPECT_EQ(midWatcher.LeaveCount, 1);
		EXPECT_EQ(rightWatcher.EnterCount, 1);
		EXPECT_EQ(rightWatcher.LeaveCount, 1);
	}
}

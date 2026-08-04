#include <gtest/gtest.h>
#include "defs.h"
#include "canvas/events/event.h"

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
					<FlexBox flexDirection="row" />
				</Style>
				<Style class="Rectangle">
					<Dimension width="200" height="100" />
				</Style>
				<Style class="Square">
					<Dimension width="100" height="100" />
				</Style>
				<Style class="ShadowBox">
					<Dimension width="100" height="100" />
					<Colors background="#FFFFFFFF" border="#000000FF" />
					<Borders value="2" />
					<Radius tl="2" tr="4" bl="6" br="8" />
					<Shadow color="#808080FF" x="10" y="12" b="4" s="2" />
				</Style>
				<Style class="RotatedBox">
					<Dimension width="100" height="40" />
					<Colors background="#FF0000FF" />
					<Transform>
						<Rotation a="45" />
					</Transform>
				</Style>
				<Style class="FadedBox">
					<Dimension width="80" height="80" />
					<Colors opacity="0" background="#00FF00FF" />
				</Style>
				<Style class="ClipBox">
					<Dimension width="100" height="100" />
					<FlexBox overflow="hidden" />
					<Colors background="#CCCCCCFF" />
				</Style>
				<Style class="ClipChild">
					<Dimension width="80" height="80" />
					<Positions type="absolute" t="60" l="60" />
					<Colors background="#FF0000FF" />
				</Style>
				<Style class="RoundBox">
					<Dimension width="100" height="100" />
					<Radius radius="50" />
					<Colors background="#3366FFFF" />
				</Style>
				<Style class="RoundClipBox">
					<Dimension width="100" height="100" />
					<FlexBox overflow="hidden" />
					<Radius radius="50" />
					<Colors background="#CCCCCCFF" />
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

class PaintRequestWatcher : public EventFilter
{
public:
	explicit PaintRequestWatcher(EventTarget* target)
		: target_(target)
	{
		if (target_)
			target_->RegisterEventFilter(this);
	}

	~PaintRequestWatcher()
	{
		if (target_)
			target_->UnRegisterEventFilter(this);
	}

	bool Filter(EventTarget*, Event* e) override
	{
		if (e->type == Type::PaintRequest)
		{
			auto* pre = static_cast<PaintRequestEvent*>(e);
			Count++;
			LastDirty = pre->dirtyRect;
		}
		return false;
	}

	EventTarget* target_ = nullptr;
	int Count = 0;
	RectF LastDirty;
};

TEST_F(ViewTest, PaintRequestDirtyRectIncludesShadowExtent)
{
	Element* box = new Element(window->RootElement());
	box->SetStyleClass(u8"ShadowBox");

	window->Show();
	window->GetView()->Flush();

	auto* rootVisual = window->GetView()->Visual();
	ASSERT_NE(rootVisual, nullptr);
	ASSERT_FALSE(rootVisual->Visuals().empty());
	auto boxVisual = rootVisual->Visuals().front();

	PaintRequestWatcher watcher(window->GetView());
	box->Visibility()->SetHidden(true);
	box->Visibility()->SetHidden(false);

	ASSERT_GE(watcher.Count, 1);
	const RectF layout = boxVisual->LayoutRect();
	EXPECT_GT(watcher.LastDirty.Width(), layout.Width());
	EXPECT_GT(watcher.LastDirty.Height(), layout.Height());
	EXPECT_LT(watcher.LastDirty.left, layout.left);
	EXPECT_LT(watcher.LastDirty.top, layout.top);
	EXPECT_GT(watcher.LastDirty.right, layout.right);
	EXPECT_GT(watcher.LastDirty.bottom, layout.bottom);

	window->Hide();
}

TEST_F(ViewTest, PaintRequestDirtyRectIncludesTransformBounds)
{
	Element* box = new Element(window->RootElement());
	box->SetStyleClass(u8"RotatedBox");

	window->Show();
	window->GetView()->Flush();

	auto* rootVisual = window->GetView()->Visual();
	ASSERT_NE(rootVisual, nullptr);
	ASSERT_FALSE(rootVisual->Visuals().empty());
	auto boxVisual = rootVisual->Visuals().front();

	PaintRequestWatcher watcher(window->GetView());
	box->Visibility()->SetHidden(true);
	box->Visibility()->SetHidden(false);

	ASSERT_GE(watcher.Count, 1);
	const RectF layout = boxVisual->LayoutRect();
	// 45-degree rotation around center grows height and changes AABB from layout rect.
	EXPECT_GT(watcher.LastDirty.Height(), layout.Height());
	EXPECT_NE(watcher.LastDirty, layout);

	window->Hide();
}

TEST_F(ViewTest, OpacityZeroFlushDoesNotCrash)
{
	Element* box = new Element(window->RootElement());
	box->SetStyleClass(u8"FadedBox");

	window->Show();
	EXPECT_NO_THROW(window->GetView()->Flush());
	EXPECT_FALSE(window->GetView()->IsDirty());
	window->Hide();
}

TEST_F(ViewTest, OverflowHiddenHitTestClipsChildren)
{
	Element* clip = new Element(window->RootElement());
	clip->SetStyleClass(u8"ClipBox");
	Element* child = new Element(clip);
	child->SetStyleClass(u8"ClipChild");

	window->Show();
	window->GetView()->Flush();

	auto* rootVisual = window->GetView()->Visual();
	ASSERT_NE(rootVisual, nullptr);
	ASSERT_FALSE(rootVisual->Visuals().empty());
	auto clipVisual = rootVisual->Visuals().front();
	ASSERT_FALSE(clipVisual->Visuals().empty());

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;
	// Child extends past the 100x100 clip box; a point outside the clip box must miss.
	EXPECT_FALSE(clipVisual->HitTest(PointF(120.0f, 120.0f), &hitVisual, hitLocalPos));
	// A point inside both the clip box and the child must still hit.
	EXPECT_TRUE(clipVisual->HitTest(PointF(70.0f, 70.0f), &hitVisual, hitLocalPos));

	window->Hide();
}

TEST_F(ViewTest, HitTestRespectsBorderRadius)
{
	Element* box = new Element(window->RootElement());
	box->SetStyleClass(u8"RoundBox");

	window->Show();
	window->GetView()->Flush();

	auto* rootVisual = window->GetView()->Visual();
	ASSERT_NE(rootVisual, nullptr);
	ASSERT_FALSE(rootVisual->Visuals().empty());
	auto boxVisual = rootVisual->Visuals().front();

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;

	EXPECT_TRUE(boxVisual->HitTest(PointF(50.0f, 50.0f), &hitVisual, hitLocalPos));
	EXPECT_EQ(hitVisual, boxVisual.get());

	// Corner of the AABB lies outside the circular border radius.
	EXPECT_FALSE(boxVisual->HitTest(PointF(1.0f, 1.0f), &hitVisual, hitLocalPos));
	EXPECT_FALSE(boxVisual->HitTest(PointF(99.0f, 1.0f), &hitVisual, hitLocalPos));
	EXPECT_FALSE(boxVisual->HitTest(PointF(1.0f, 99.0f), &hitVisual, hitLocalPos));
	EXPECT_FALSE(boxVisual->HitTest(PointF(99.0f, 99.0f), &hitVisual, hitLocalPos));

	window->Hide();
}

TEST_F(ViewTest, OverflowHiddenHitTestRespectsBorderRadius)
{
	Element* clip = new Element(window->RootElement());
	clip->SetStyleClass(u8"RoundClipBox");
	Element* child = new Element(clip);
	child->SetStyleClass(u8"ClipChild");

	window->Show();
	window->GetView()->Flush();

	auto* rootVisual = window->GetView()->Visual();
	ASSERT_NE(rootVisual, nullptr);
	ASSERT_FALSE(rootVisual->Visuals().empty());
	auto clipVisual = rootVisual->Visuals().front();

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;
	// Inside AABB and over the absolute child, but outside the circular clip shape.
	EXPECT_FALSE(clipVisual->HitTest(PointF(90.0f, 90.0f), &hitVisual, hitLocalPos));
	// Center remains hittable.
	EXPECT_TRUE(clipVisual->HitTest(PointF(50.0f, 50.0f), &hitVisual, hitLocalPos));

	window->Hide();
}

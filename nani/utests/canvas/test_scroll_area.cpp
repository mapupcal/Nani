#include <gtest/gtest.h>

#include "defs.h"
#include "canvas/elements/scroll_area_element.h"
#include "canvas/events/event.h"

class ScrollAreaTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		env_ = new Env(0, nullptr);
		window = new Window(PointF(), SizeF(600, 400));
		window->RootElement()->GetStyles()->LoadFromXML(R"(
			<Styles>
				<Style class="NaniWindow">
					<FlexBox flexDirection="column" />
				</Style>
				<Style class="ScrollViewport">
					<Dimension width="100" height="100" />
					<FlexBox overflow="scroll" flexDirection="column" />
					<Colors background="#CCCCCCFF" />
				</Style>
				<Style class="TallChild">
					<Dimension width="80" height="300" />
					<Positions type="absolute" t="0" l="10" />
					<Colors background="#FF0000FF" />
				</Style>
				<Style class="LowerChild">
					<Dimension width="80" height="80" />
					<Positions type="absolute" t="150" l="10" />
					<Colors background="#FF0000FF" />
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

private:
	Env* env_ = nullptr;
};

TEST_F(ScrollAreaTest, ScrollOffsetClampsToContent)
{
	ScrollAreaElement scroll(nullptr);
	scroll.UpdateScrollMetrics(SizeF(100.0f, 400.0f), SizeF(100.0f, 100.0f));

	scroll.SetScrollOffset(PointF(0.0f, 500.0f));
	EXPECT_FLOAT_EQ(scroll.ScrollOffset().y, 300.0f);

	scroll.SetScrollOffset(PointF(-20.0f, -10.0f));
	EXPECT_EQ(scroll.ScrollOffset(), PointF(0.0f, 0.0f));

	scroll.ScrollBy(0.0f, 50.0f);
	EXPECT_FLOAT_EQ(scroll.ScrollOffset().y, 50.0f);
}

TEST_F(ScrollAreaTest, OverflowScrollClipsHitTestLikeHidden)
{
	auto* scroll = new ScrollAreaElement(window->RootElement());
	scroll->SetStyleClass(u8"ScrollViewport");
	auto* child = new Element(scroll);
	child->SetStyleClass(u8"TallChild");

	window->Show();
	window->GetView()->Flush();

	auto* rootVisual = window->GetView()->Visual();
	ASSERT_NE(rootVisual, nullptr);
	ASSERT_FALSE(rootVisual->Visuals().empty());
	auto scrollVisual = rootVisual->Visuals().front();

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;
	EXPECT_FALSE(scrollVisual->HitTest(PointF(50.0f, 150.0f), &hitVisual, hitLocalPos));
	EXPECT_TRUE(scrollVisual->HitTest(PointF(50.0f, 50.0f), &hitVisual, hitLocalPos));

	window->Hide();
}

TEST_F(ScrollAreaTest, HitTestFollowsScrollOffset)
{
	auto* scroll = new ScrollAreaElement(window->RootElement());
	scroll->SetStyleClass(u8"ScrollViewport");
	auto* child = new Element(scroll);
	child->SetStyleClass(u8"LowerChild");

	window->Show();
	window->GetView()->Flush();

	auto* rootVisual = window->GetView()->Visual();
	auto scrollVisual = rootVisual->Visuals().front();
	ASSERT_FALSE(scrollVisual->Visuals().empty());
	auto childVisual = scrollVisual->Visuals().front();

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;

	// Child starts at y=150; before scrolling, (50,50) hits the scroll area itself.
	EXPECT_TRUE(scrollVisual->HitTest(PointF(50.0f, 50.0f), &hitVisual, hitLocalPos));
	EXPECT_EQ(hitVisual, scrollVisual.get());

	scroll->SetScrollOffset(PointF(0.0f, 150.0f));

	// After scrolling, the lower child moves into the viewport under (50,50).
	EXPECT_TRUE(scrollVisual->HitTest(PointF(50.0f, 50.0f), &hitVisual, hitLocalPos));
	EXPECT_EQ(hitVisual, childVisual.get());

	window->Hide();
}

TEST_F(ScrollAreaTest, WheelEventScrollsHitScrollArea)
{
	auto* scroll = new ScrollAreaElement(window->RootElement());
	scroll->SetStyleClass(u8"ScrollViewport");
	auto* child = new Element(scroll);
	child->SetStyleClass(u8"TallChild");

	window->Show();
	window->GetView()->Flush();

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;
	auto scrollVisual = window->GetView()->Visual()->Visuals().front();
	ASSERT_TRUE(scrollVisual->HitTest(PointF(50.0f, 50.0f), &hitVisual, hitLocalPos));
	EXPECT_GT(scroll->ContentSize().height, scroll->ViewportSize().height);

	WheelEvent wheel(PointF(50.0f, 50.0f), PointF(50.0f, 50.0f), 0.0, -1.0);
	window->FireEvent(&wheel);

	EXPECT_FLOAT_EQ(scroll->ScrollOffset().y, 40.0f);

	window->Hide();
}

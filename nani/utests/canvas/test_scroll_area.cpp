#include <gtest/gtest.h>

#include "defs.h"
#include "canvas/elements/scroll_area_element.h"
#include "canvas/visuals/scroll_area_visual.h"
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
				<Style class="ScrollViewportBordered">
					<Dimension width="100" height="100" />
					<FlexBox overflow="scroll" flexDirection="column" />
					<Borders value="10" />
					<Colors background="#CCCCCCFF" border="#000000FF" />
				</Style>
				<Style class="OuterScroll">
					<Dimension width="200" height="200" />
					<FlexBox overflow="scroll" />
					<Colors background="#EEEEEEFF" />
				</Style>
				<Style class="InnerScroll">
					<Dimension width="100" height="100" />
					<Positions type="absolute" t="20" l="20" />
					<FlexBox overflow="scroll" />
					<Colors background="#DDDDDDFF" />
				</Style>
				<Style class="TallChild">
					<Dimension width="80" height="300" />
					<Positions type="absolute" t="0" l="10" />
					<Colors background="#FF0000FF" />
				</Style>
				<Style class="WideChild">
					<Dimension width="300" height="80" />
					<Positions type="absolute" t="10" l="0" />
					<Colors background="#00FF00FF" />
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

class WheelCountWatcher : public EventFilter
{
public:
	explicit WheelCountWatcher(EventTarget* target)
		: target_(target)
	{
		if (target_)
			target_->RegisterEventFilter(this);
	}

	~WheelCountWatcher()
	{
		if (target_)
			target_->UnRegisterEventFilter(this);
	}

	bool Filter(EventTarget*, Event* e) override
	{
		if (e->type == Type::Wheel)
			Count++;
		return false;
	}

	EventTarget* target_ = nullptr;
	int Count = 0;
};

TEST_F(ScrollAreaTest, DefaultStyleClassIsScrollableColumn)
{
	ScrollAreaElement scroll(nullptr);
	EXPECT_EQ(scroll.StyleClass(), u8"ScrollableColumn");
}

TEST_F(ScrollAreaTest, CreateVisualReturnsScrollAreaVisual)
{
	auto* scroll = new ScrollAreaElement(window->RootElement());
	auto visual = scroll->CreateVisual(window->GetView(), nullptr);
	ASSERT_NE(visual, nullptr);
	EXPECT_NE(dynamic_cast<ScrollAreaVisual*>(visual.get()), nullptr);
}

TEST_F(ScrollAreaTest, ScrollOffsetClampsToContent)
{
	ScrollAreaElement scroll(nullptr);
	scroll.UpdateScrollMetrics(SizeF(400.0f, 400.0f), SizeF(100.0f, 100.0f));

	scroll.SetScrollOffset(PointF(0.0f, 500.0f));
	EXPECT_FLOAT_EQ(scroll.ScrollOffset().y, 300.0f);

	scroll.SetScrollOffset(PointF(500.0f, 0.0f));
	EXPECT_FLOAT_EQ(scroll.ScrollOffset().x, 300.0f);

	scroll.SetScrollOffset(PointF(-20.0f, -10.0f));
	EXPECT_EQ(scroll.ScrollOffset(), PointF(0.0f, 0.0f));

	scroll.ScrollBy(50.0f, 50.0f);
	EXPECT_EQ(scroll.ScrollOffset(), PointF(50.0f, 50.0f));
}

TEST_F(ScrollAreaTest, UpdateScrollMetricsReclampsOffset)
{
	ScrollAreaElement scroll(nullptr);
	scroll.UpdateScrollMetrics(SizeF(100.0f, 400.0f), SizeF(100.0f, 100.0f));
	scroll.SetScrollOffset(PointF(0.0f, 300.0f));
	EXPECT_FLOAT_EQ(scroll.ScrollOffset().y, 300.0f);

	// Content shrinks; max offset becomes 50.
	scroll.UpdateScrollMetrics(SizeF(100.0f, 150.0f), SizeF(100.0f, 100.0f));
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

TEST_F(ScrollAreaTest, WheelEventScrollsHorizontally)
{
	auto* scroll = new ScrollAreaElement(window->RootElement());
	scroll->SetStyleClass(u8"ScrollViewport");
	auto* child = new Element(scroll);
	child->SetStyleClass(u8"WideChild");

	window->Show();
	window->GetView()->Flush();

	Visual* hitVisual = nullptr;
	PointF hitLocalPos;
	auto scrollVisual = window->GetView()->Visual()->Visuals().front();
	ASSERT_TRUE(scrollVisual->HitTest(PointF(50.0f, 50.0f), &hitVisual, hitLocalPos));
	EXPECT_GT(scroll->ContentSize().width, scroll->ViewportSize().width);

	WheelEvent wheel(PointF(50.0f, 50.0f), PointF(50.0f, 50.0f), -1.0, 0.0);
	window->FireEvent(&wheel);

	EXPECT_FLOAT_EQ(scroll->ScrollOffset().x, 40.0f);
	EXPECT_FLOAT_EQ(scroll->ScrollOffset().y, 0.0f);

	window->Hide();
}

TEST_F(ScrollAreaTest, WheelOnChildBubblesToScrollArea)
{
	auto* scroll = new ScrollAreaElement(window->RootElement());
	scroll->SetStyleClass(u8"ScrollViewport");
	auto* child = new Element(scroll);
	child->SetStyleClass(u8"TallChild");

	WheelCountWatcher childWatcher(child);
	WheelCountWatcher scrollWatcher(scroll);

	window->Show();
	window->GetView()->Flush();

	// (50,50) lands on TallChild inside the scroll viewport.
	WheelEvent wheel(PointF(50.0f, 50.0f), PointF(50.0f, 50.0f), 0.0, -1.0);
	window->FireEvent(&wheel);

	EXPECT_EQ(childWatcher.Count, 1);
	EXPECT_EQ(scrollWatcher.Count, 1);
	EXPECT_FLOAT_EQ(scroll->ScrollOffset().y, 40.0f);

	window->Hide();
}

TEST_F(ScrollAreaTest, NestedScrollAreaStopsAtInnermost)
{
	auto* outer = new ScrollAreaElement(window->RootElement());
	outer->SetStyleClass(u8"OuterScroll");
	auto* inner = new ScrollAreaElement(outer);
	inner->SetStyleClass(u8"InnerScroll");
	auto* child = new Element(inner);
	child->SetStyleClass(u8"TallChild");

	WheelCountWatcher outerWatcher(outer);
	WheelCountWatcher innerWatcher(inner);

	window->Show();
	window->GetView()->Flush();

	// Point inside the inner scroll (outer origin + inner offset 20,20 + local 50,50).
	WheelEvent wheel(PointF(70.0f, 70.0f), PointF(70.0f, 70.0f), 0.0, -1.0);
	window->FireEvent(&wheel);

	EXPECT_EQ(innerWatcher.Count, 1);
	EXPECT_EQ(outerWatcher.Count, 0);
	EXPECT_FLOAT_EQ(inner->ScrollOffset().y, 40.0f);
	EXPECT_FLOAT_EQ(outer->ScrollOffset().y, 0.0f);

	window->Hide();
}

class ScrollChangedWatcher : public EventFilter
{
public:
	explicit ScrollChangedWatcher(EventTarget* target)
		: target_(target)
	{
		if (target_)
			target_->RegisterEventFilter(this);
	}

	~ScrollChangedWatcher()
	{
		if (target_)
			target_->UnRegisterEventFilter(this);
	}

	bool Filter(EventTarget*, Event* e) override
	{
		if (e->type == Type::ElementScrollChanged)
			Count++;
		return false;
	}

	EventTarget* target_ = nullptr;
	int Count = 0;
};

TEST_F(ScrollAreaTest, SetScrollOffsetFiresScrollChanged)
{
	ScrollAreaElement scroll(nullptr);
	scroll.UpdateScrollMetrics(SizeF(100.0f, 400.0f), SizeF(100.0f, 100.0f));

	ScrollChangedWatcher watcher(&scroll);
	scroll.SetScrollOffset(PointF(0.0f, 40.0f));
	EXPECT_EQ(watcher.Count, 1);

	scroll.SetScrollOffset(PointF(0.0f, 40.0f));
	EXPECT_EQ(watcher.Count, 1);
}

TEST_F(ScrollAreaTest, OverflowScrollHitTestRespectsBorderInsetShape)
{
	auto* scroll = new ScrollAreaElement(window->RootElement());
	scroll->SetStyleClass(u8"ScrollViewportBordered");
	auto* child = new Element(scroll);
	child->SetStyleClass(u8"TallChild");

	window->Show();
	window->GetView()->Flush();

	auto scrollVisual = window->GetView()->Visual()->Visuals().front();
	Visual* hitVisual = nullptr;
	PointF hitLocalPos;

	// Outside the 100x100 border box still misses (overflow clip).
	EXPECT_FALSE(scrollVisual->HitTest(PointF(50.0f, 150.0f), &hitVisual, hitLocalPos));
	// Inside the padded content area still hits.
	EXPECT_TRUE(scrollVisual->HitTest(PointF(50.0f, 50.0f), &hitVisual, hitLocalPos));

	window->Hide();
}

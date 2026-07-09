#include <gtest/gtest.h>
#include "defs.h"
class WindowTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		env_ = new Env(0, nullptr);
	}

	void TearDown() override
	{
		delete env_;
	}

private:
	Env* env_ = nullptr;
};


TEST_F(WindowTest, WindowProperties)
{
	std::shared_ptr<Window> window = std::make_shared < Window >(PointF(0, 0), SizeF(400, 600));
	EXPECT_FLOAT_EQ(window->Position().x, 0.0f);
	EXPECT_FLOAT_EQ(window->Position().y, 0.0f);
	EXPECT_FLOAT_EQ(window->Size().width, 400.0f);
	EXPECT_FLOAT_EQ(window->Size().height, 600.0f);

	EXPECT_FLOAT_EQ(window->IsVisible(), false);
	window->Show();
	EXPECT_FLOAT_EQ(window->IsVisible(), true);
	window->Hide();
	EXPECT_FLOAT_EQ(window->IsVisible(), false);

	window->Move(PointF(100,100));
	EXPECT_FLOAT_EQ(window->Position().x, 100.0f);
	EXPECT_FLOAT_EQ(window->Position().y, 100.0f);
	EXPECT_FLOAT_EQ(window->Size().width, 400.0f);
	EXPECT_FLOAT_EQ(window->Size().height, 600.0f);

	window->Resize(SizeF(200, 300));
	EXPECT_FLOAT_EQ(window->Position().x, 100.0f);
	EXPECT_FLOAT_EQ(window->Position().y, 100.0f);
	EXPECT_FLOAT_EQ(window->Size().width, 200.0f);
	EXPECT_FLOAT_EQ(window->Size().height, 300.0f);

	window->Show();
	EXPECT_FLOAT_EQ(window->IsVisible(), true);
	EXPECT_FLOAT_EQ(window->Position().x, 100.0f);
	EXPECT_FLOAT_EQ(window->Position().y, 100.0f);
	EXPECT_FLOAT_EQ(window->Size().width, 200.0f);
	EXPECT_FLOAT_EQ(window->Size().height, 300.0f);
	window->Move(PointF(50.f, 50.f));
	window->Resize(SizeF(500, 800));
	EXPECT_FLOAT_EQ(window->IsVisible(), true);
	EXPECT_FLOAT_EQ(window->Position().x, 50.0f);
	EXPECT_FLOAT_EQ(window->Position().y, 50.0f);
	EXPECT_FLOAT_EQ(window->Size().width, 500.0f);
	EXPECT_FLOAT_EQ(window->Size().height, 800.0f);

	window->Close();
	EXPECT_FLOAT_EQ(window->IsVisible(), false);
	EXPECT_FLOAT_EQ(window->Position().x, 50.0f);
	EXPECT_FLOAT_EQ(window->Position().y, 50.0f);
	EXPECT_FLOAT_EQ(window->Size().width, 500.0f);
	EXPECT_FLOAT_EQ(window->Size().height, 800.0f);

	window->Show();
	EXPECT_FLOAT_EQ(window->IsVisible(), true);
	EXPECT_FLOAT_EQ(window->Position().x, 50.0f);
	EXPECT_FLOAT_EQ(window->Position().y, 50.0f);
	EXPECT_FLOAT_EQ(window->Size().width, 500.0f);
	EXPECT_FLOAT_EQ(window->Size().height, 800.0f);
}

TEST_F(WindowTest, WindowDecorationProperties)
{
	std::shared_ptr<Window> window = std::make_shared<Window>(PointF(0, 0), SizeF(400, 500));

	// BorderWidth
	window->SetBorderWidth(3.0f);
	EXPECT_FLOAT_EQ(window->BorderWidth(), 3.0f);

	// Radius
	window->SetRadius(8.0f);
	EXPECT_FLOAT_EQ(window->Radius(), 8.0f);

	// BorderColor
	window->SetBorderColor(Color(255, 0, 0, 255));
	EXPECT_EQ(window->BorderColor(), Color(255, 0, 0, 255));

	// BackgroundColor
	window->SetBackgroundColor(Color(0, 255, 0, 128));
	EXPECT_EQ(window->BackgroundColor(), Color(0, 255, 0, 128));

	// Title
	window->SetTitle("Nani Test Window");
	EXPECT_EQ(window->Title(), "Nani Test Window");
}

TEST_F(WindowTest, WindowHints)
{
	std::shared_ptr<Window> window = std::make_shared<Window>(PointF(0, 0), SizeF(400, 500));

	EXPECT_EQ(window->Hints(), Window::None);

	window->SetHints(Window::Resizable | Window::Tool);
	EXPECT_EQ(window->Hints(), Window::Resizable | Window::Tool);

	window->SetHints(Window::Top | Window::TruncatedPassThrough);
	EXPECT_EQ(window->Hints(), Window::Top | Window::TruncatedPassThrough);
}

TEST_F(WindowTest, WindowHasRootElementAndView)
{
	std::shared_ptr<Window> window = std::make_shared<Window>(PointF(0, 0), SizeF(400, 500));

	EXPECT_NE(window->RootElement(), nullptr);
	EXPECT_NE(window->GetView(), nullptr);
}

TEST_F(WindowTest, WindowTruncatedColor)
{
	std::shared_ptr<Window> window = std::make_shared<Window>(PointF(0, 0), SizeF(400, 500));

	window->SetTruncatedColor(Color(255, 128, 64, 255));
	// SetTruncatedColor is a void, we just verify no crash
	EXPECT_NO_FATAL_FAILURE({
		window->SetTruncatedColor(Color(255, 128, 64, 255));
	});
}



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


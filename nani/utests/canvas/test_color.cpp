#include <gtest/gtest.h>
#include "canvas/basic/color.h"

using namespace nani::canvas::basic;

class ColorTest : public ::testing::Test
{
protected:
	void SetUp() override
	{

	}

	void TearDown() override
	{

	}
};


TEST_F(ColorTest, ColorProperties)
{
	Color defaultBlack;
	EXPECT_EQ(defaultBlack.r, 0);
	EXPECT_EQ(defaultBlack.g, 0);
	EXPECT_EQ(defaultBlack.b, 0);
	EXPECT_EQ(defaultBlack.a, 255);

	EXPECT_EQ(defaultBlack, Color(0, 0, 0, 255));

	Color white(Colors::White);
	EXPECT_EQ(white.r, 255);
	EXPECT_EQ(white.g, 255);
	EXPECT_EQ(white.b, 255);
	EXPECT_EQ(white.a, 255);

	EXPECT_EQ(white, Color(255, 255, 255, 255));

	Color red(Colors::Red);
	EXPECT_EQ(red.r, 255);
	EXPECT_EQ(red.g, 0);
	EXPECT_EQ(red.b, 0);
	EXPECT_EQ(red.a, 255);

	Color green(Colors::Green);
	EXPECT_EQ(green.r, 0);
	EXPECT_EQ(green.g, 255);
	EXPECT_EQ(green.b, 0);
	EXPECT_EQ(green.a, 255);

	Color blue(Colors::Blue);
	EXPECT_EQ(blue.r, 0);
	EXPECT_EQ(blue.g, 0);
	EXPECT_EQ(blue.b, 255);
	EXPECT_EQ(blue.a, 255);

	Color yellow(Colors::Yellow);
	EXPECT_EQ(yellow.r, 255);
	EXPECT_EQ(yellow.g, 255);
	EXPECT_EQ(yellow.b, 0);
	EXPECT_EQ(yellow.a, 255);

	Color magenta(Colors::Magenta);
	EXPECT_EQ(magenta.r, 255);
	EXPECT_EQ(magenta.g, 0);
	EXPECT_EQ(magenta.b, 255);
	EXPECT_EQ(magenta.a, 255);

	Color cyan(Colors::Cyan);
	EXPECT_EQ(cyan.r, 0);
	EXPECT_EQ(cyan.g, 255);
	EXPECT_EQ(cyan.b, 255);
	EXPECT_EQ(cyan.a, 255);

	Color gray(Colors::Gray);
	EXPECT_EQ(gray.r, 128);
	EXPECT_EQ(gray.g, 128);
	EXPECT_EQ(gray.b, 128);
	EXPECT_EQ(gray.a, 255);

	Color black(Colors::Black);
	EXPECT_EQ(black.r, 0);
	EXPECT_EQ(black.g, 0);
	EXPECT_EQ(black.b, 0);
	EXPECT_EQ(black.a, 255);

	Color transparent(Colors::Transparent);
	EXPECT_EQ(transparent.a, 0);

	Color rgba(255, 128, 64, 192);
	EXPECT_EQ(rgba.r, 255);
	EXPECT_EQ(rgba.g, 128);
	EXPECT_EQ(rgba.b, 64);
	EXPECT_EQ(rgba.a, 192);

	EXPECT_EQ(rgba, Color(255, 128, 64, 192));
	EXPECT_NE(rgba, Color(255, 128, 64, 191));
	EXPECT_NE(rgba, Color(255, 128, 63, 192));
	EXPECT_NE(rgba, Color(255, 127, 64, 192));
	EXPECT_NE(rgba, Color(254, 128, 64, 192));

	//from RGBA value
	{
		Color rgbaColor(0xFF00FF80); // Magenta with 50% opacity
		EXPECT_EQ(rgbaColor.r, 255);
		EXPECT_EQ(rgbaColor.g, 0);
		EXPECT_EQ(rgbaColor.b, 255);
		EXPECT_EQ(rgbaColor.a, 128);
	}

	//from hex string
	{
		Color hexColor("#FF00FF80"); // Magenta with 50% opacity
		EXPECT_EQ(hexColor.r, 255);
		EXPECT_EQ(hexColor.g, 0);
		EXPECT_EQ(hexColor.b, 255);
		EXPECT_EQ(hexColor.a, 128);
	}

	{
		Color hexColor("#FF00FF"); // Magenta #RRGGBB
		EXPECT_EQ(hexColor.r, 255);
		EXPECT_EQ(hexColor.g, 0);
		EXPECT_EQ(hexColor.b, 255);
		EXPECT_EQ(hexColor.a, 255);
	}

	{
		Color hexColor("FF00FF"); // Magenta RRGGBB
		EXPECT_EQ(hexColor.r, 255);
		EXPECT_EQ(hexColor.g, 0);
		EXPECT_EQ(hexColor.b, 255);
		EXPECT_EQ(hexColor.a, 255);
	}

	{
		Color hexColor("FF000FF"); // Wrong format, should fallback to black
		EXPECT_EQ(hexColor.r, 0);
		EXPECT_EQ(hexColor.g, 0);
		EXPECT_EQ(hexColor.b, 0);
		EXPECT_EQ(hexColor.a, 255);
	}

	{
		Color hexColor("ZF00FF"); // Wrong format, should fallback to black
		EXPECT_EQ(hexColor.r, 0);
		EXPECT_EQ(hexColor.g, 0);
		EXPECT_EQ(hexColor.b, 0);
		EXPECT_EQ(hexColor.a, 255);
	}
}

TEST_F(ColorTest, ColorMethods)
{
	{
		Color color(0x12345678);
		EXPECT_EQ(color.Argb(), 0x78123456);
		EXPECT_EQ(color.Rgba(), 0x12345678);
		EXPECT_EQ(color.ToString(), "#12345678");
		EXPECT_EQ(color.ToString(Color::HexArgb), "#78123456");
	}

	{
		Color color(0x123ABCDE);
		EXPECT_EQ(color.Argb(), 0xDE123ABC);
		EXPECT_EQ(color.Rgba(), 0x123ABCDE);
		EXPECT_EQ(color.ToString(), "#123ABCDE");
		EXPECT_EQ(color.ToString(Color::HexArgb), "#DE123ABC");
	}
}


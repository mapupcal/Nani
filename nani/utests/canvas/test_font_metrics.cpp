#include <gtest/gtest.h>
#include "defs.h"
#include "canvas/text/font.h"
#include "canvas/text/font_metrics.h"

using namespace nani::canvas::text;

class FontMetricsTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		env_ = new Env(0, nullptr);
		font_.SetFamily(u8"Segoe UI");
		font_.SetSize(16.0f);
	}

	void TearDown() override
	{
		delete env_;
	}

	Font font_;

private:
	Env* env_ = nullptr;
};

TEST_F(FontMetricsTest, BasicMetricsArePositive)
{
	FontMetrics metrics(font_);
	EXPECT_GT(metrics.Ascent(), 0.0f);
	EXPECT_GT(metrics.Descent(), 0.0f);
	EXPECT_GE(metrics.Leading(), 0.0f);
	EXPECT_FLOAT_EQ(metrics.LineHeight(), metrics.Ascent() + metrics.Descent() + metrics.Leading());
	EXPECT_GT(metrics.XHeight(), 0.0f);
	EXPECT_GT(metrics.CapHeight(), 0.0f);
}

TEST_F(FontMetricsTest, DecorationMetricsAreSane)
{
	FontMetrics metrics(font_);
	EXPECT_GT(metrics.UnderlineThickness(), 0.0f);
	EXPECT_GT(metrics.StrikeoutThickness(), 0.0f);
	EXPECT_GT(metrics.UnderlineOffset(), 0.0f);
	EXPECT_LT(metrics.StrikeoutOffset(), 0.0f);
	EXPECT_LT(std::abs(metrics.StrikeoutOffset()), metrics.Ascent());
}

TEST_F(FontMetricsTest, HorizontalAdvanceEmptyIsZero)
{
	FontMetrics metrics(font_);
	EXPECT_FLOAT_EQ(metrics.HorizontalAdvance(u8""), 0.0f);
}

TEST_F(FontMetricsTest, HorizontalAdvanceScalesWithTextLength)
{
	FontMetrics metrics(font_);
	const float shortW = metrics.HorizontalAdvance(u8"Hi");
	const float longW = metrics.HorizontalAdvance(u8"Hello World");
	EXPECT_GT(shortW, 0.0f);
	EXPECT_GT(longW, shortW);
}

TEST_F(FontMetricsTest, BoundingRectAndMeasureText)
{
	FontMetrics metrics(font_);

	const auto empty = metrics.BoundingRect(u8"");
	EXPECT_FLOAT_EQ(empty.Width(), 0.0f);
	EXPECT_FLOAT_EQ(empty.Height(), 0.0f);

	const auto bounds = metrics.BoundingRect(u8"A");
	EXPECT_GT(bounds.Width(), 0.0f);

	const auto size = metrics.MeasureText(u8"A");
	EXPECT_FLOAT_EQ(size.width, bounds.Width());
	EXPECT_FLOAT_EQ(size.height, bounds.Height());
}

TEST_F(FontMetricsTest, ElidedTextNoneKeepsOriginal)
{
	FontMetrics metrics(font_);
	const std::u8string text = u8"Hello World";
	EXPECT_TRUE(metrics.ElidedText(text, 1.0f, TextElideMode::None) == text);
}

TEST_F(FontMetricsTest, ElidedTextRightShortensWhenNeeded)
{
	FontMetrics metrics(font_);
	const std::u8string text = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const float fullWidth = metrics.HorizontalAdvance(text);
	ASSERT_GT(fullWidth, 0.0f);

	const auto elided = metrics.ElidedText(text, fullWidth * 0.4f, TextElideMode::Right);
	EXPECT_LT(metrics.HorizontalAdvance(elided), fullWidth);
	EXPECT_FALSE(elided == text);
	EXPECT_FALSE(elided.empty());
}

TEST_F(FontMetricsTest, ElidedTextLeftAndMiddleShortenWhenNeeded)
{
	FontMetrics metrics(font_);
	const std::u8string text = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const float fullWidth = metrics.HorizontalAdvance(text);
	const float maxWidth = fullWidth * 0.4f;

	const auto left = metrics.ElidedText(text, maxWidth, TextElideMode::Left);
	const auto middle = metrics.ElidedText(text, maxWidth, TextElideMode::Middle);
	EXPECT_LT(metrics.HorizontalAdvance(left), fullWidth);
	EXPECT_LT(metrics.HorizontalAdvance(middle), fullWidth);
	EXPECT_FALSE(left == text);
	EXPECT_FALSE(middle == text);
}

TEST_F(FontMetricsTest, ElidedTextWideEnoughKeepsOriginal)
{
	FontMetrics metrics(font_);
	const std::u8string text = u8"Hi";
	const float width = metrics.HorizontalAdvance(text);
	EXPECT_TRUE(metrics.ElidedText(text, width + 10.0f, TextElideMode::Right) == text);
	EXPECT_TRUE(metrics.ElidedText(text, width + 10.0f, TextElideMode::Left) == text);
	EXPECT_TRUE(metrics.ElidedText(text, width + 10.0f, TextElideMode::Middle) == text);
}

TEST_F(FontMetricsTest, LayoutLinesHardBreaksOnNewline)
{
	FontMetrics metrics(font_);
	const auto lines = metrics.LayoutLines(u8"Hello\nWorld\n", 0.0f, false);
	ASSERT_EQ(lines.size(), 3u);
	EXPECT_TRUE(lines[0] == u8"Hello");
	EXPECT_TRUE(lines[1] == u8"World");
	EXPECT_TRUE(lines[2].empty());
}

TEST_F(FontMetricsTest, LayoutLinesSoftWrapsByWidth)
{
	FontMetrics metrics(font_);
	const std::u8string text = u8"one two three four five six";
	const float fullWidth = metrics.HorizontalAdvance(text);
	ASSERT_GT(fullWidth, 0.0f);

	const auto lines = metrics.LayoutLines(text, fullWidth * 0.35f, true);
	ASSERT_GT(lines.size(), 1u);
	for (const auto& line : lines)
		EXPECT_LE(metrics.HorizontalAdvance(line), fullWidth * 0.35f + 1.0f);
}

TEST_F(FontMetricsTest, ElidedTextTinyWidthWithMultibyteDoesNotHang)
{
	FontMetrics metrics(font_);
	// Em dash is 3-byte UTF-8; old FindUtf8Boundary moved forward and
	// deadlocked the elide binary search when the glyph did not fit.
	const std::u8string text = u8"Multi-line text — hard break";
	const float fullWidth = metrics.HorizontalAdvance(text);
	const float ellipsisWidth = metrics.HorizontalAdvance(u8"…");
	ASSERT_GT(fullWidth, 0.0f);
	ASSERT_GT(ellipsisWidth, 0.0f);

	for (float w = fullWidth; w >= 0.0f; w -= 1.0f)
	{
		const auto right = metrics.ElidedText(text, w, TextElideMode::Right);
		const auto left = metrics.ElidedText(text, w, TextElideMode::Left);
		const auto middle = metrics.ElidedText(text, w, TextElideMode::Middle);
		// When maxWidth is below the ellipsis glyph, API returns the ellipsis as-is.
		const float budget = std::max(w, ellipsisWidth) + 1.0f;
		EXPECT_LE(metrics.HorizontalAdvance(right), budget);
		EXPECT_LE(metrics.HorizontalAdvance(left), budget);
		EXPECT_LE(metrics.HorizontalAdvance(middle), budget);
	}
}

TEST_F(FontMetricsTest, LayoutLinesTinyWidthDoesNotHang)
{
	FontMetrics metrics(font_);
	const std::u8string text =
		u8"Line one via hard break.\nLine two — soft wrap: "
		u8"alpha bravo charlie delta echo foxtrot golf hotel";
	for (float w = 80.0f; w >= 0.0f; w -= 1.0f)
	{
		const auto lines = metrics.LayoutLines(text, w, true);
		EXPECT_FALSE(lines.empty());
		EXPECT_LE(lines.size(), text.size() + 2u);
	}
}

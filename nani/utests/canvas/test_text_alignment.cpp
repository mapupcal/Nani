#include <gtest/gtest.h>
#include "defs.h"
#include "canvas/text/text_alignment.h"

using namespace nani::canvas::text;

class TextAlignmentTest : public ::testing::Test
{
};

TEST_F(TextAlignmentTest, DefaultConstruction)
{
	TextAlignment align;
	EXPECT_EQ(align.HorizontalAlign(), TextAlignment::Horizontal::Left);
	EXPECT_EQ(align.VerticalAlign(), TextAlignment::Vertical::Top);
}

TEST_F(TextAlignmentTest, HorizontalRoundTrip)
{
	TextAlignment align;
	align.SetHorizontal(TextAlignment::Horizontal::Center);
	EXPECT_EQ(align.HorizontalAlign(), TextAlignment::Horizontal::Center);

	align.SetHorizontal(TextAlignment::Horizontal::Right);
	EXPECT_EQ(align.HorizontalAlign(), TextAlignment::Horizontal::Right);

	align.SetHorizontal(TextAlignment::Horizontal::Left);
	EXPECT_EQ(align.HorizontalAlign(), TextAlignment::Horizontal::Left);
}

TEST_F(TextAlignmentTest, VerticalRoundTrip)
{
	TextAlignment align;
	align.SetVertical(TextAlignment::Vertical::Center);
	EXPECT_EQ(align.VerticalAlign(), TextAlignment::Vertical::Center);

	align.SetVertical(TextAlignment::Vertical::Bottom);
	EXPECT_EQ(align.VerticalAlign(), TextAlignment::Vertical::Bottom);

	align.SetVertical(TextAlignment::Vertical::Top);
	EXPECT_EQ(align.VerticalAlign(), TextAlignment::Vertical::Top);
}

TEST_F(TextAlignmentTest, EqualityOperator_Identical)
{
	TextAlignment a;
	a.SetHorizontal(TextAlignment::Horizontal::Center);
	a.SetVertical(TextAlignment::Vertical::Bottom);

	TextAlignment b;
	b.SetHorizontal(TextAlignment::Horizontal::Center);
	b.SetVertical(TextAlignment::Vertical::Bottom);

	EXPECT_EQ(a, b);
	EXPECT_FALSE(a != b);
}

TEST_F(TextAlignmentTest, EqualityOperator_DifferentHorizontal)
{
	TextAlignment a;
	a.SetHorizontal(TextAlignment::Horizontal::Left);
	TextAlignment b;
	b.SetHorizontal(TextAlignment::Horizontal::Right);
	EXPECT_NE(a, b);
}

TEST_F(TextAlignmentTest, EqualityOperator_DifferentVertical)
{
	TextAlignment a;
	a.SetVertical(TextAlignment::Vertical::Top);
	TextAlignment b;
	b.SetVertical(TextAlignment::Vertical::Bottom);
	EXPECT_NE(a, b);
}

TEST_F(TextAlignmentTest, CopyConstruction)
{
	TextAlignment orig;
	orig.SetHorizontal(TextAlignment::Horizontal::Right);
	orig.SetVertical(TextAlignment::Vertical::Center);

	TextAlignment copy(orig);
	EXPECT_EQ(copy.HorizontalAlign(), TextAlignment::Horizontal::Right);
	EXPECT_EQ(copy.VerticalAlign(), TextAlignment::Vertical::Center);
	EXPECT_EQ(copy, orig);
}

TEST_F(TextAlignmentTest, CopyAssignment)
{
	TextAlignment orig;
	orig.SetHorizontal(TextAlignment::Horizontal::Center);
	orig.SetVertical(TextAlignment::Vertical::Bottom);

	TextAlignment assigned;
	assigned = orig;
	EXPECT_EQ(assigned.HorizontalAlign(), TextAlignment::Horizontal::Center);
	EXPECT_EQ(assigned.VerticalAlign(), TextAlignment::Vertical::Bottom);
	EXPECT_EQ(assigned, orig);
}

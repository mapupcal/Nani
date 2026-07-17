#include <gtest/gtest.h>
#include "defs.h"
#include "canvas/text/font.h"

using namespace nani::canvas::text;

// ============================================================
// Fixture: FontTest — pure data-class tests for Font
// ============================================================
class FontTest : public ::testing::Test
{
};

// -----------------------------------------------------------
// Default constructor — verify all default values
// -----------------------------------------------------------
TEST_F(FontTest, DefaultConstruction)
{
	Font font;

	EXPECT_TRUE(font.Family().empty());
	EXPECT_FLOAT_EQ(font.Size(), 12.0f);
	EXPECT_EQ(font.Weight(), Font::Weight::Normal);
	EXPECT_EQ(font.Style(), Font::Style::Normal);
}

// -----------------------------------------------------------
// SetFamily / Family round-trip
// -----------------------------------------------------------
TEST_F(FontTest, FamilyRoundTrip)
{
	Font font;
	font.SetFamily(u8"Arial");
	EXPECT_EQ(font.Family(), u8"Arial");

	font.SetFamily(u8"微软雅黑");
	EXPECT_EQ(font.Family(), u8"微软雅黑");

	font.SetFamily(u8"");
	EXPECT_TRUE(font.Family().empty());
}

// -----------------------------------------------------------
// SetSize / Size round-trip
// -----------------------------------------------------------
TEST_F(FontTest, SizeRoundTrip)
{
	Font font;
	font.SetSize(24.0f);
	EXPECT_FLOAT_EQ(font.Size(), 24.0f);

	font.SetSize(0.0f);
	EXPECT_FLOAT_EQ(font.Size(), 0.0f);

	font.SetSize(72.5f);
	EXPECT_FLOAT_EQ(font.Size(), 72.5f);
}

// -----------------------------------------------------------
// SetWeight / Weight round-trip — all enum values
// -----------------------------------------------------------
TEST_F(FontTest, WeightRoundTrip)
{
	Font font;

	font.SetWeight(Font::Weight::Thin);
	EXPECT_EQ(font.Weight(), Font::Weight::Thin);

	font.SetWeight(Font::Weight::ExtraLight);
	EXPECT_EQ(font.Weight(), Font::Weight::ExtraLight);

	font.SetWeight(Font::Weight::Light);
	EXPECT_EQ(font.Weight(), Font::Weight::Light);

	font.SetWeight(Font::Weight::Normal);
	EXPECT_EQ(font.Weight(), Font::Weight::Normal);

	font.SetWeight(Font::Weight::Medium);
	EXPECT_EQ(font.Weight(), Font::Weight::Medium);

	font.SetWeight(Font::Weight::SemiBold);
	EXPECT_EQ(font.Weight(), Font::Weight::SemiBold);

	font.SetWeight(Font::Weight::Bold);
	EXPECT_EQ(font.Weight(), Font::Weight::Bold);

	font.SetWeight(Font::Weight::ExtraBold);
	EXPECT_EQ(font.Weight(), Font::Weight::ExtraBold);

	font.SetWeight(Font::Weight::Black);
	EXPECT_EQ(font.Weight(), Font::Weight::Black);
}

// -----------------------------------------------------------
// SetStyle / Style round-trip — all enum values
// -----------------------------------------------------------
TEST_F(FontTest, StyleRoundTrip)
{
	Font font;

	font.SetStyle(Font::Style::Normal);
	EXPECT_EQ(font.Style(), Font::Style::Normal);

	font.SetStyle(Font::Style::Italic);
	EXPECT_EQ(font.Style(), Font::Style::Italic);

	font.SetStyle(Font::Style::Oblique);
	EXPECT_EQ(font.Style(), Font::Style::Oblique);
}

// -----------------------------------------------------------
// operator== — identical fonts
// -----------------------------------------------------------
TEST_F(FontTest, EqualityOperator_Identical)
{
	Font a, b;
	EXPECT_TRUE(a == b);

	a.SetFamily(u8"Consolas");
	a.SetSize(14.0f);
	a.SetWeight(Font::Weight::Bold);
	a.SetStyle(Font::Style::Italic);

	b.SetFamily(u8"Consolas");
	b.SetSize(14.0f);
	b.SetWeight(Font::Weight::Bold);
	b.SetStyle(Font::Style::Italic);

	EXPECT_TRUE(a == b);
}

// -----------------------------------------------------------
// operator== — different family
// -----------------------------------------------------------
TEST_F(FontTest, EqualityOperator_DifferentFamily)
{
	Font a, b;
	a.SetFamily(u8"Arial");
	b.SetFamily(u8"Times New Roman");
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
}

// -----------------------------------------------------------
// operator== — different size
// -----------------------------------------------------------
TEST_F(FontTest, EqualityOperator_DifferentSize)
{
	Font a, b;
	a.SetSize(16.0f);
	b.SetSize(18.0f);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
}

// -----------------------------------------------------------
// operator== — different weight
// -----------------------------------------------------------
TEST_F(FontTest, EqualityOperator_DifferentWeight)
{
	Font a, b;
	a.SetWeight(Font::Weight::Normal);
	b.SetWeight(Font::Weight::Bold);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
}

// -----------------------------------------------------------
// operator== — different style
// -----------------------------------------------------------
TEST_F(FontTest, EqualityOperator_DifferentStyle)
{
	Font a, b;
	a.SetStyle(Font::Style::Normal);
	b.SetStyle(Font::Style::Italic);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
}

// -----------------------------------------------------------
// Copy construction
// -----------------------------------------------------------
TEST_F(FontTest, CopyConstruction)
{
	Font orig;
	orig.SetFamily(u8"Verdana");
	orig.SetSize(20.0f);
	orig.SetWeight(Font::Weight::SemiBold);
	orig.SetStyle(Font::Style::Oblique);

	Font copy(orig);
	EXPECT_EQ(copy.Family(), u8"Verdana");
	EXPECT_FLOAT_EQ(copy.Size(), 20.0f);
	EXPECT_EQ(copy.Weight(), Font::Weight::SemiBold);
	EXPECT_EQ(copy.Style(), Font::Style::Oblique);
	EXPECT_TRUE(copy == orig);
}

// -----------------------------------------------------------
// Copy assignment
// -----------------------------------------------------------
TEST_F(FontTest, CopyAssignment)
{
	Font orig;
	orig.SetFamily(u8"Courier New");
	orig.SetSize(10.0f);
	orig.SetWeight(Font::Weight::Light);
	orig.SetStyle(Font::Style::Italic);

	Font assigned;
	assigned = orig;
	EXPECT_EQ(assigned.Family(), u8"Courier New");
	EXPECT_FLOAT_EQ(assigned.Size(), 10.0f);
	EXPECT_EQ(assigned.Weight(), Font::Weight::Light);
	EXPECT_EQ(assigned.Style(), Font::Style::Italic);
	EXPECT_TRUE(assigned == orig);
}

// -----------------------------------------------------------
// Weight enum underlying values
// -----------------------------------------------------------
TEST_F(FontTest, WeightEnumValues)
{
	EXPECT_EQ(static_cast<int>(Font::Weight::Thin), 100);
	EXPECT_EQ(static_cast<int>(Font::Weight::ExtraLight), 200);
	EXPECT_EQ(static_cast<int>(Font::Weight::Light), 300);
	EXPECT_EQ(static_cast<int>(Font::Weight::Normal), 400);
	EXPECT_EQ(static_cast<int>(Font::Weight::Medium), 500);
	EXPECT_EQ(static_cast<int>(Font::Weight::SemiBold), 600);
	EXPECT_EQ(static_cast<int>(Font::Weight::Bold), 700);
	EXPECT_EQ(static_cast<int>(Font::Weight::ExtraBold), 800);
	EXPECT_EQ(static_cast<int>(Font::Weight::Black), 900);
}

// -----------------------------------------------------------
// FontWeight and FontStyle type aliases
// -----------------------------------------------------------
TEST_F(FontTest, TypeAliasesWork)
{
	Font::FontWeight w = Font::Weight::Bold;
	Font::FontStyle s = Font::Style::Italic;

	Font font;
	font.SetWeight(w);
	font.SetStyle(s);

	EXPECT_EQ(font.Weight(), Font::Weight::Bold);
	EXPECT_EQ(font.Style(), Font::Style::Italic);
}

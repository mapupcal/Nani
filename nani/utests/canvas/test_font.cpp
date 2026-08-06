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
	EXPECT_EQ(font.Weight(), FontWeight::Normal);
	EXPECT_EQ(font.Style(), FontStyle::Normal);
}

// -----------------------------------------------------------
// SetFamily / Family round-trip
// -----------------------------------------------------------
TEST_F(FontTest, FamilyRoundTrip)
{
	Font font;
	font.SetFamily(u8"Arial");
	EXPECT_EQ(font.Family(), u8"Arial");
	ASSERT_EQ(font.Families().size(), 1u);
	EXPECT_TRUE(font.Families().front() == u8"Arial");

	font.SetFamily(u8"微软雅黑");
	EXPECT_EQ(font.Family(), u8"微软雅黑");

	font.SetFamily(u8"");
	EXPECT_TRUE(font.Family().empty());
	EXPECT_TRUE(font.Families().empty());
}

TEST_F(FontTest, FamiliesRoundTripAndEquality)
{
	Font a;
	a.SetFamilies({ u8"Segoe UI", u8"Microsoft YaHei UI", u8"" });
	ASSERT_EQ(a.Families().size(), 2u);
	EXPECT_EQ(a.Family(), u8"Segoe UI");
	EXPECT_TRUE(a.Families()[1] == u8"Microsoft YaHei UI");

	Font b;
	b.SetFamilies({ u8"Segoe UI", u8"Microsoft YaHei UI" });
	EXPECT_EQ(a, b);
	EXPECT_EQ(a.Hash(), b.Hash());

	b.SetFamily(u8"Segoe UI");
	EXPECT_NE(a, b);
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

	font.SetWeight(FontWeight::Thin);
	EXPECT_EQ(font.Weight(), FontWeight::Thin);

	font.SetWeight(FontWeight::ExtraLight);
	EXPECT_EQ(font.Weight(), FontWeight::ExtraLight);

	font.SetWeight(FontWeight::Light);
	EXPECT_EQ(font.Weight(), FontWeight::Light);

	font.SetWeight(FontWeight::Normal);
	EXPECT_EQ(font.Weight(), FontWeight::Normal);

	font.SetWeight(FontWeight::Medium);
	EXPECT_EQ(font.Weight(), FontWeight::Medium);

	font.SetWeight(FontWeight::SemiBold);
	EXPECT_EQ(font.Weight(), FontWeight::SemiBold);

	font.SetWeight(FontWeight::Bold);
	EXPECT_EQ(font.Weight(), FontWeight::Bold);

	font.SetWeight(FontWeight::ExtraBold);
	EXPECT_EQ(font.Weight(), FontWeight::ExtraBold);

	font.SetWeight(FontWeight::Black);
	EXPECT_EQ(font.Weight(), FontWeight::Black);
}

// -----------------------------------------------------------
// SetStyle / Style round-trip — all enum values
// -----------------------------------------------------------
TEST_F(FontTest, StyleRoundTrip)
{
	Font font;

	font.SetStyle(FontStyle::Normal);
	EXPECT_EQ(font.Style(), FontStyle::Normal);

	font.SetStyle(FontStyle::Italic);
	EXPECT_EQ(font.Style(), FontStyle::Italic);

	font.SetStyle(FontStyle::Oblique);
	EXPECT_EQ(font.Style(), FontStyle::Oblique);
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
	a.SetWeight(FontWeight::Bold);
	a.SetStyle(FontStyle::Italic);

	b.SetFamily(u8"Consolas");
	b.SetSize(14.0f);
	b.SetWeight(FontWeight::Bold);
	b.SetStyle(FontStyle::Italic);

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
	a.SetWeight(FontWeight::Normal);
	b.SetWeight(FontWeight::Bold);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
}

// -----------------------------------------------------------
// operator== — different style
// -----------------------------------------------------------
TEST_F(FontTest, EqualityOperator_DifferentStyle)
{
	Font a, b;
	a.SetStyle(FontStyle::Normal);
	b.SetStyle(FontStyle::Italic);
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
	orig.SetWeight(FontWeight::SemiBold);
	orig.SetStyle(FontStyle::Oblique);

	Font copy(orig);
	EXPECT_EQ(copy.Family(), u8"Verdana");
	EXPECT_FLOAT_EQ(copy.Size(), 20.0f);
	EXPECT_EQ(copy.Weight(), FontWeight::SemiBold);
	EXPECT_EQ(copy.Style(), FontStyle::Oblique);
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
	orig.SetWeight(FontWeight::Light);
	orig.SetStyle(FontStyle::Italic);

	Font assigned;
	assigned = orig;
	EXPECT_EQ(assigned.Family(), u8"Courier New");
	EXPECT_FLOAT_EQ(assigned.Size(), 10.0f);
	EXPECT_EQ(assigned.Weight(), FontWeight::Light);
	EXPECT_EQ(assigned.Style(), FontStyle::Italic);
	EXPECT_TRUE(assigned == orig);
}

// -----------------------------------------------------------
// Weight enum underlying values
// -----------------------------------------------------------
TEST_F(FontTest, WeightEnumValues)
{
	EXPECT_EQ(static_cast<int>(FontWeight::Thin), 100);
	EXPECT_EQ(static_cast<int>(FontWeight::ExtraLight), 200);
	EXPECT_EQ(static_cast<int>(FontWeight::Light), 300);
	EXPECT_EQ(static_cast<int>(FontWeight::Normal), 400);
	EXPECT_EQ(static_cast<int>(FontWeight::Medium), 500);
	EXPECT_EQ(static_cast<int>(FontWeight::SemiBold), 600);
	EXPECT_EQ(static_cast<int>(FontWeight::Bold), 700);
	EXPECT_EQ(static_cast<int>(FontWeight::ExtraBold), 800);
	EXPECT_EQ(static_cast<int>(FontWeight::Black), 900);
}

// -----------------------------------------------------------
// FontWeight and FontStyle namespace enums
// -----------------------------------------------------------
TEST_F(FontTest, NamespaceEnumsWork)
{
	FontWeight w = FontWeight::Bold;
	FontStyle s = FontStyle::Italic;

	Font font;
	font.SetWeight(w);
	font.SetStyle(s);

	EXPECT_EQ(font.Weight(), FontWeight::Bold);
	EXPECT_EQ(font.Style(), FontStyle::Italic);
}

TEST_F(FontTest, HashChangesWithProperties)
{
	Font a;
	a.SetFamily(u8"Segoe UI");
	a.SetSize(14.0f);
	a.SetWeight(FontWeight::Normal);
	a.SetStyle(FontStyle::Normal);

	Font b = a;
	EXPECT_EQ(a.Hash(), b.Hash());

	b.SetSize(16.0f);
	EXPECT_NE(a.Hash(), b.Hash());

	Font c = a;
	c.SetWeight(FontWeight::Bold);
	EXPECT_NE(a.Hash(), c.Hash());

	Font d = a;
	d.SetStyle(FontStyle::Italic);
	EXPECT_NE(a.Hash(), d.Hash());

	Font e = a;
	e.SetFamily(u8"Arial");
	EXPECT_NE(a.Hash(), e.Hash());
}

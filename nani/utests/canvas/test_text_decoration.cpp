#include <gtest/gtest.h>
#include "defs.h"
#include "canvas/text/text_decoration.h"

using namespace nani::canvas::text;

// ============================================================
// Fixture: TextDecorationTest — pure data-class tests
// ============================================================
class TextDecorationTest : public ::testing::Test
{
};

// -----------------------------------------------------------
// Default constructor — verify all default values
// -----------------------------------------------------------
TEST_F(TextDecorationTest, DefaultConstruction)
{
	TextDecoration td;

	EXPECT_EQ(td.Color(), nani::canvas::basic::Colors::Black);
	EXPECT_EQ(td.Lines(), DecorationLine::None);
	EXPECT_EQ(td.Style(), DecorationStyle::Solid);
}

// -----------------------------------------------------------
// SetColor / Color round-trip
// -----------------------------------------------------------
TEST_F(TextDecorationTest, ColorRoundTrip)
{
	TextDecoration td;

	td.SetColor(nani::canvas::basic::Colors::Red);
	EXPECT_EQ(td.Color(), nani::canvas::basic::Colors::Red);

	td.SetColor(nani::canvas::basic::Colors::Blue);
	EXPECT_EQ(td.Color(), nani::canvas::basic::Colors::Blue);

	nani::canvas::basic::Color custom(128, 64, 32, 255);
	td.SetColor(custom);
	EXPECT_EQ(td.Color(), custom);
}

// -----------------------------------------------------------
// SetLines / Lines round-trip — single line values
// -----------------------------------------------------------
TEST_F(TextDecorationTest, LinesRoundTrip_SingleLine)
{
	TextDecoration td;

	td.SetLines(DecorationLine::None);
	EXPECT_EQ(td.Lines(), DecorationLine::None);

	td.SetLines(DecorationLine::Underline);
	EXPECT_EQ(td.Lines(), DecorationLine::Underline);

	td.SetLines(DecorationLine::Overline);
	EXPECT_EQ(td.Lines(), DecorationLine::Overline);

	td.SetLines(DecorationLine::LineThrough);
	EXPECT_EQ(td.Lines(), DecorationLine::LineThrough);
}

// -----------------------------------------------------------
// SetLines / Lines round-trip — combined (bitwise OR) lines
// -----------------------------------------------------------
TEST_F(TextDecorationTest, LinesRoundTrip_Combined)
{
	using Line = DecorationLine;
	using byte = nani::canvas::basic::byte;

	TextDecoration td;

	// Underline + Overline
	Line combined1 = static_cast<Line>(
		static_cast<byte>(Line::Underline) | static_cast<byte>(Line::Overline));
	td.SetLines(combined1);
	EXPECT_EQ(td.Lines(), combined1);

	// Underline + LineThrough
	Line combined2 = static_cast<Line>(
		static_cast<byte>(Line::Underline) | static_cast<byte>(Line::LineThrough));
	td.SetLines(combined2);
	EXPECT_EQ(td.Lines(), combined2);

	// Overline + LineThrough
	Line combined3 = static_cast<Line>(
		static_cast<byte>(Line::Overline) | static_cast<byte>(Line::LineThrough));
	td.SetLines(combined3);
	EXPECT_EQ(td.Lines(), combined3);

	// All three
	Line combined4 = static_cast<Line>(
		static_cast<byte>(Line::Underline) |
		static_cast<byte>(Line::Overline) |
		static_cast<byte>(Line::LineThrough));
	td.SetLines(combined4);
	EXPECT_EQ(td.Lines(), combined4);
}

// -----------------------------------------------------------
// SetStyle / Style round-trip — all enum values
// -----------------------------------------------------------
TEST_F(TextDecorationTest, StyleRoundTrip)
{
	TextDecoration td;

	td.SetStyle(DecorationStyle::Solid);
	EXPECT_EQ(td.Style(), DecorationStyle::Solid);

	td.SetStyle(DecorationStyle::Double);
	EXPECT_EQ(td.Style(), DecorationStyle::Double);

	td.SetStyle(DecorationStyle::Dotted);
	EXPECT_EQ(td.Style(), DecorationStyle::Dotted);

	td.SetStyle(DecorationStyle::Dashed);
	EXPECT_EQ(td.Style(), DecorationStyle::Dashed);

	td.SetStyle(DecorationStyle::Wavy);
	EXPECT_EQ(td.Style(), DecorationStyle::Wavy);
}

// -----------------------------------------------------------
// operator== — identical decorations
// -----------------------------------------------------------
TEST_F(TextDecorationTest, EqualityOperator_Identical)
{
	TextDecoration a, b;
	EXPECT_TRUE(a == b);

	a.SetColor(nani::canvas::basic::Colors::Red);
	a.SetLines(DecorationLine::Underline);
	a.SetStyle(DecorationStyle::Dashed);

	b.SetColor(nani::canvas::basic::Colors::Red);
	b.SetLines(DecorationLine::Underline);
	b.SetStyle(DecorationStyle::Dashed);

	EXPECT_TRUE(a == b);
}

// -----------------------------------------------------------
// operator== — different color
// -----------------------------------------------------------
TEST_F(TextDecorationTest, EqualityOperator_DifferentColor)
{
	TextDecoration a, b;
	a.SetColor(nani::canvas::basic::Colors::Red);
	b.SetColor(nani::canvas::basic::Colors::Blue);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
}

// -----------------------------------------------------------
// operator== — different lines
// -----------------------------------------------------------
TEST_F(TextDecorationTest, EqualityOperator_DifferentLines)
{
	TextDecoration a, b;
	a.SetLines(DecorationLine::Underline);
	b.SetLines(DecorationLine::Overline);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
}

// -----------------------------------------------------------
// operator== — different style
// -----------------------------------------------------------
TEST_F(TextDecorationTest, EqualityOperator_DifferentStyle)
{
	TextDecoration a, b;
	a.SetStyle(DecorationStyle::Solid);
	b.SetStyle(DecorationStyle::Wavy);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
}

// -----------------------------------------------------------
// Copy construction
// -----------------------------------------------------------
TEST_F(TextDecorationTest, CopyConstruction)
{
	TextDecoration orig;
	orig.SetColor(nani::canvas::basic::Colors::Green);
	orig.SetLines(DecorationLine::Underline);
	orig.SetStyle(DecorationStyle::Dotted);

	TextDecoration copy(orig);
	EXPECT_EQ(copy.Color(), nani::canvas::basic::Colors::Green);
	EXPECT_EQ(copy.Lines(), DecorationLine::Underline);
	EXPECT_EQ(copy.Style(), DecorationStyle::Dotted);
	EXPECT_TRUE(copy == orig);
}

// -----------------------------------------------------------
// Copy assignment
// -----------------------------------------------------------
TEST_F(TextDecorationTest, CopyAssignment)
{
	TextDecoration orig;
	orig.SetColor(nani::canvas::basic::Colors::Magenta);
	orig.SetLines(DecorationLine::LineThrough);
	orig.SetStyle(DecorationStyle::Double);

	TextDecoration assigned;
	assigned = orig;
	EXPECT_EQ(assigned.Color(), nani::canvas::basic::Colors::Magenta);
	EXPECT_EQ(assigned.Lines(), DecorationLine::LineThrough);
	EXPECT_EQ(assigned.Style(), DecorationStyle::Double);
	EXPECT_TRUE(assigned == orig);
}

// -----------------------------------------------------------
// DecorationLine and DecorationStyle namespace enums
// -----------------------------------------------------------
TEST_F(TextDecorationTest, NamespaceEnumsWork)
{
	DecorationLine line = DecorationLine::Underline;
	DecorationStyle style = DecorationStyle::Wavy;

	TextDecoration td;
	td.SetLines(line);
	td.SetStyle(style);

	EXPECT_EQ(td.Lines(), DecorationLine::Underline);
	EXPECT_EQ(td.Style(), DecorationStyle::Wavy);
}

// -----------------------------------------------------------
// Line enum bit values — must be bitwise combinable
// -----------------------------------------------------------
TEST_F(TextDecorationTest, LineEnumBitValues)
{
	using byte = nani::canvas::basic::byte;

	EXPECT_EQ(static_cast<byte>(DecorationLine::None), 0);
	EXPECT_EQ(static_cast<byte>(DecorationLine::Underline), 1);
	EXPECT_EQ(static_cast<byte>(DecorationLine::Overline), 2);
	EXPECT_EQ(static_cast<byte>(DecorationLine::LineThrough), 4);
}

TEST_F(TextDecorationTest, DecorationLineBitwiseOperators)
{
	const DecorationLine combined = DecorationLine::Underline | DecorationLine::Overline;
	EXPECT_EQ(combined & DecorationLine::Underline, DecorationLine::Underline);
	EXPECT_EQ(combined & DecorationLine::Overline, DecorationLine::Overline);
	EXPECT_EQ(combined & DecorationLine::LineThrough, DecorationLine::None);

	const DecorationLine all =
		DecorationLine::Underline | DecorationLine::Overline | DecorationLine::LineThrough;
	EXPECT_NE(all & DecorationLine::LineThrough, DecorationLine::None);
}

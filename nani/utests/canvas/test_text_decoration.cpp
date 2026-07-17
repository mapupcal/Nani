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
	EXPECT_EQ(td.Lines(), TextDecoration::Line::None);
	EXPECT_EQ(td.Style(), TextDecoration::Style::Solid);
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

	td.SetLines(TextDecoration::Line::None);
	EXPECT_EQ(td.Lines(), TextDecoration::Line::None);

	td.SetLines(TextDecoration::Line::Underline);
	EXPECT_EQ(td.Lines(), TextDecoration::Line::Underline);

	td.SetLines(TextDecoration::Line::Overline);
	EXPECT_EQ(td.Lines(), TextDecoration::Line::Overline);

	td.SetLines(TextDecoration::Line::LineThrough);
	EXPECT_EQ(td.Lines(), TextDecoration::Line::LineThrough);
}

// -----------------------------------------------------------
// SetLines / Lines round-trip — combined (bitwise OR) lines
// -----------------------------------------------------------
TEST_F(TextDecorationTest, LinesRoundTrip_Combined)
{
	using Line = TextDecoration::Line;
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

	td.SetStyle(TextDecoration::Style::Solid);
	EXPECT_EQ(td.Style(), TextDecoration::Style::Solid);

	td.SetStyle(TextDecoration::Style::Double);
	EXPECT_EQ(td.Style(), TextDecoration::Style::Double);

	td.SetStyle(TextDecoration::Style::Dotted);
	EXPECT_EQ(td.Style(), TextDecoration::Style::Dotted);

	td.SetStyle(TextDecoration::Style::Dashed);
	EXPECT_EQ(td.Style(), TextDecoration::Style::Dashed);

	td.SetStyle(TextDecoration::Style::Wavy);
	EXPECT_EQ(td.Style(), TextDecoration::Style::Wavy);
}

// -----------------------------------------------------------
// operator== — identical decorations
// -----------------------------------------------------------
TEST_F(TextDecorationTest, EqualityOperator_Identical)
{
	TextDecoration a, b;
	EXPECT_TRUE(a == b);

	a.SetColor(nani::canvas::basic::Colors::Red);
	a.SetLines(TextDecoration::Line::Underline);
	a.SetStyle(TextDecoration::Style::Dashed);

	b.SetColor(nani::canvas::basic::Colors::Red);
	b.SetLines(TextDecoration::Line::Underline);
	b.SetStyle(TextDecoration::Style::Dashed);

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
	a.SetLines(TextDecoration::Line::Underline);
	b.SetLines(TextDecoration::Line::Overline);
	EXPECT_FALSE(a == b);
	EXPECT_TRUE(a != b);
}

// -----------------------------------------------------------
// operator== — different style
// -----------------------------------------------------------
TEST_F(TextDecorationTest, EqualityOperator_DifferentStyle)
{
	TextDecoration a, b;
	a.SetStyle(TextDecoration::Style::Solid);
	b.SetStyle(TextDecoration::Style::Wavy);
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
	orig.SetLines(TextDecoration::Line::Underline);
	orig.SetStyle(TextDecoration::Style::Dotted);

	TextDecoration copy(orig);
	EXPECT_EQ(copy.Color(), nani::canvas::basic::Colors::Green);
	EXPECT_EQ(copy.Lines(), TextDecoration::Line::Underline);
	EXPECT_EQ(copy.Style(), TextDecoration::Style::Dotted);
	EXPECT_TRUE(copy == orig);
}

// -----------------------------------------------------------
// Copy assignment
// -----------------------------------------------------------
TEST_F(TextDecorationTest, CopyAssignment)
{
	TextDecoration orig;
	orig.SetColor(nani::canvas::basic::Colors::Magenta);
	orig.SetLines(TextDecoration::Line::LineThrough);
	orig.SetStyle(TextDecoration::Style::Double);

	TextDecoration assigned;
	assigned = orig;
	EXPECT_EQ(assigned.Color(), nani::canvas::basic::Colors::Magenta);
	EXPECT_EQ(assigned.Lines(), TextDecoration::Line::LineThrough);
	EXPECT_EQ(assigned.Style(), TextDecoration::Style::Double);
	EXPECT_TRUE(assigned == orig);
}

// -----------------------------------------------------------
// DecorationLine and DecorationStyle type aliases
// -----------------------------------------------------------
TEST_F(TextDecorationTest, TypeAliasesWork)
{
	TextDecoration::DecorationLine line = TextDecoration::Line::Underline;
	TextDecoration::DecorationStyle style = TextDecoration::Style::Wavy;

	TextDecoration td;
	td.SetLines(line);
	td.SetStyle(style);

	EXPECT_EQ(td.Lines(), TextDecoration::Line::Underline);
	EXPECT_EQ(td.Style(), TextDecoration::Style::Wavy);
}

// -----------------------------------------------------------
// Line enum bit values — must be bitwise combinable
// -----------------------------------------------------------
TEST_F(TextDecorationTest, LineEnumBitValues)
{
	using byte = nani::canvas::basic::byte;

	EXPECT_EQ(static_cast<byte>(TextDecoration::Line::None), 0);
	EXPECT_EQ(static_cast<byte>(TextDecoration::Line::Underline), 1);
	EXPECT_EQ(static_cast<byte>(TextDecoration::Line::Overline), 2);
	EXPECT_EQ(static_cast<byte>(TextDecoration::Line::LineThrough), 4);
}

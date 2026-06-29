#include <gtest/gtest.h>
#include "defs.h"
#include "canvas/internal/computed_style.h"

using namespace nani::canvas::internal;

// ============================================================
// Fixture: StylesTest — tests Styles directly via
//   LoadFromXML() + Compute(styleClass),
//   no Window/Visual dependency.
// ============================================================
class StylesTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		styles_ = std::make_shared<Styles>();
	}

	void TearDown() override
	{
	}

	std::shared_ptr<Styles> styles_;
};

// -----------------------------------------------------------
// 1. Basic dimension: width / height
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_BasicDimension)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Rect">
				<Dimension width="200" height="100" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"Rect");
	ASSERT_NE(cs, nullptr);

	auto w = cs->layoutProps.style.dimension(facebook::yoga::Dimension::Width);
	auto h = cs->layoutProps.style.dimension(facebook::yoga::Dimension::Height);
	EXPECT_FLOAT_EQ(w.value().unwrap(), 200.0f);
	EXPECT_FLOAT_EQ(h.value().unwrap(), 100.0f);
}

// -----------------------------------------------------------
// 2. Colors node
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithColors)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Colored">
				<Colors color="#FF0000FF" background="#00FF00FF" border="#0000FFFF" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"Colored");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->visualProps.color, Color("#FF0000FF"));
	EXPECT_EQ(cs->visualProps.backgroundColor, Color("#00FF00FF"));
	EXPECT_EQ(cs->visualProps.borderColor, Color("#0000FFFF"));
}

// -----------------------------------------------------------
// 3. Opacity
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithOpacity)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Faded">
				<Colors opacity="0.3" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"Faded");
	ASSERT_NE(cs, nullptr);

	EXPECT_FLOAT_EQ(cs->visualProps.opacity, 0.3f);
}

// -----------------------------------------------------------
// 4. Uniform border-radius
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithBorderRadius_Uniform)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Round">
				<Radius radius="5" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"Round");
	ASSERT_NE(cs, nullptr);

	EXPECT_FLOAT_EQ(cs->visualProps.radius.topLeft, 5.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.radius.topRight, 5.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.radius.bottomLeft, 5.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.radius.bottomRight, 5.0f);
}

// -----------------------------------------------------------
// 5. Individual border-radius (tl, tr, bl, br)
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithBorderRadius_Individual)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="CornerRound">
				<Radius tl="2" tr="4" bl="6" br="8" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"CornerRound");
	ASSERT_NE(cs, nullptr);

	EXPECT_FLOAT_EQ(cs->visualProps.radius.topLeft, 2.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.radius.topRight, 4.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.radius.bottomLeft, 6.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.radius.bottomRight, 8.0f);
}

// -----------------------------------------------------------
// 6. Shadow
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithShadow)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="DropShadow">
				<Shadow color="#808080FF" x="2" y="2" b="4" s="1" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"DropShadow");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->visualProps.shadow.color, Color("#808080FF"));
	EXPECT_FLOAT_EQ(cs->visualProps.shadow.offsetX, 2.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.shadow.offsetY, 2.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.shadow.blur, 4.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.shadow.spread, 1.0f);
}

// -----------------------------------------------------------
// 7. Transform — Translation, Rotation, Scaling, Shearing
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithTransform)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Transformed">
				<Dimension width="200" height="100" />
				<Transform>
					<Translation x="20" y="10" />
					<Rotation a="0.5" />
					<Scaling x="1.5" y="1.5" />
					<Shearing x="0.1" y="0.2" />
				</Transform>
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"Transformed");
	ASSERT_NE(cs, nullptr);

	// The Transform should be non-identity.
	TransformF t = cs->visualProps.transform;
	PointF p(10, 10);
	PointF result = t.ApplyTo(p);
	// Identity would return (10, 10), transform composed -> different.
	EXPECT_FALSE(basic::IsScalarEqual(result.x, 10.0f) && basic::IsScalarEqual(result.y, 10.0f));
}

// -----------------------------------------------------------
// 8. Flex and direction
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithFlexAndDirection)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="ColumnBox" flex="column" direction="rtl" />
		</Styles>
	)");

	auto cs = styles_->Compute(u8"ColumnBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->layoutProps.style.flexDirection(),
		facebook::yoga::FlexDirection::Column);
	EXPECT_EQ(cs->layoutProps.style.direction(),
		facebook::yoga::Direction::RTL);
}

// -----------------------------------------------------------
// 9. Reload — loading XML twice, second overwrites first
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_ReloadOverwrites)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Reloadable">
				<Dimension width="100" height="50" />
			</Style>
		</Styles>
	)");

	auto cs1 = styles_->Compute(u8"Reloadable");
	ASSERT_NE(cs1, nullptr);
	auto w1 = cs1->layoutProps.style.dimension(facebook::yoga::Dimension::Width);
	EXPECT_FLOAT_EQ(w1.value().unwrap(), 100.0f);

	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Reloadable">
				<Dimension width="250" height="75" />
			</Style>
		</Styles>
	)");

	auto cs2 = styles_->Compute(u8"Reloadable");
	ASSERT_NE(cs2, nullptr);
	auto w2 = cs2->layoutProps.style.dimension(facebook::yoga::Dimension::Width);
	auto h2 = cs2->layoutProps.style.dimension(facebook::yoga::Dimension::Height);
	EXPECT_FLOAT_EQ(w2.value().unwrap(), 250.0f);
	EXPECT_FLOAT_EQ(h2.value().unwrap(), 75.0f);
}

// -----------------------------------------------------------
// 10. Empty <Styles> node — no crash, Compute returns default
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_NoStyles)
{
	EXPECT_NO_FATAL_FAILURE({
		styles_->LoadFromXML(R"(
			<Styles>
			</Styles>
		)");
	});

	auto cs = styles_->Compute(u8"Anything");
	ASSERT_NE(cs, nullptr);
	// Default values: opacity should be 1.0
	EXPECT_FLOAT_EQ(cs->visualProps.opacity, 1.0f);
}

// -----------------------------------------------------------
// 11. Style with no class attribute — skipped
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_EmptyStyleClass)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style>
				<Dimension width="999" height="999" />
			</Style>
			<Style class="ValidOne">
				<Dimension width="111" height="111" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"ValidOne");
	ASSERT_NE(cs, nullptr);

	auto w = cs->layoutProps.style.dimension(facebook::yoga::Dimension::Width);
	auto h = cs->layoutProps.style.dimension(facebook::yoga::Dimension::Height);
	EXPECT_FLOAT_EQ(w.value().unwrap(), 111.0f);
	EXPECT_FLOAT_EQ(h.value().unwrap(), 111.0f);
}

// -----------------------------------------------------------
// 12. Inherit — child inherits parent dimension
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_ChildInheritsParentDimension)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Base">
				<Dimension width="200" height="100" />
			</Style>
			<Style class="Derived" inherit="Base">
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"Derived");
	ASSERT_NE(cs, nullptr);

	auto w = cs->layoutProps.style.dimension(facebook::yoga::Dimension::Width);
	auto h = cs->layoutProps.style.dimension(facebook::yoga::Dimension::Height);
	EXPECT_FLOAT_EQ(w.value().unwrap(), 200.0f);
	EXPECT_FLOAT_EQ(h.value().unwrap(), 100.0f);
}

// -----------------------------------------------------------
// 13. Inherit — child overrides parent's width, inherits height
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_ChildOverridesParentProperty)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Base2">
				<Dimension width="200" height="100" />
			</Style>
			<Style class="Derived2" inherit="Base2">
				<Dimension width="300" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"Derived2");
	ASSERT_NE(cs, nullptr);

	auto w = cs->layoutProps.style.dimension(facebook::yoga::Dimension::Width);
	auto h = cs->layoutProps.style.dimension(facebook::yoga::Dimension::Height);
	EXPECT_FLOAT_EQ(w.value().unwrap(), 300.0f);
	EXPECT_FLOAT_EQ(h.value().unwrap(), 100.0f);
}

// -----------------------------------------------------------
// 14. Inherit — chain: Leaf -> Middle -> Base
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_MultiLevelInheritance)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Base3">
				<Dimension width="200" height="100" />
			</Style>
			<Style class="Middle3" inherit="Base3">
				<Dimension height="80" />
			</Style>
			<Style class="Leaf3" inherit="Middle3">
				<Dimension width="150" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"Leaf3");
	ASSERT_NE(cs, nullptr);

	auto w = cs->layoutProps.style.dimension(facebook::yoga::Dimension::Width);
	auto h = cs->layoutProps.style.dimension(facebook::yoga::Dimension::Height);
	EXPECT_FLOAT_EQ(w.value().unwrap(), 150.0f);
	EXPECT_FLOAT_EQ(h.value().unwrap(), 80.0f);
}

// -----------------------------------------------------------
// 15. Inherit — non-existent parent, uses own values
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_InheritFromNonExistent)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Orphan" inherit="NoSuchClass">
				<Dimension width="111" height="111" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"Orphan");
	ASSERT_NE(cs, nullptr);

	auto w = cs->layoutProps.style.dimension(facebook::yoga::Dimension::Width);
	auto h = cs->layoutProps.style.dimension(facebook::yoga::Dimension::Height);
	EXPECT_FLOAT_EQ(w.value().unwrap(), 111.0f);
	EXPECT_FLOAT_EQ(h.value().unwrap(), 111.0f);
}

// -----------------------------------------------------------
// 16. Inherit — child inherits parent's colors, overrides opacity
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_InheritWithColors)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="BaseColor">
				<Colors opacity="0.3" background="#FF0000FF" />
			</Style>
			<Style class="DerivedColor" inherit="BaseColor">
				<Colors opacity="0.8" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"DerivedColor");
	ASSERT_NE(cs, nullptr);

	EXPECT_FLOAT_EQ(cs->visualProps.opacity, 0.8f);
	EXPECT_EQ(cs->visualProps.backgroundColor, Color("#FF0000FF"));
}

// -----------------------------------------------------------
// 17. Inherit — child inherits parent's border-radius
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_InheritBorderRadius)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="BaseRadius">
				<Radius tl="3" tr="5" bl="7" br="9" />
			</Style>
			<Style class="DerivedRadius" inherit="BaseRadius">
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"DerivedRadius");
	ASSERT_NE(cs, nullptr);

	EXPECT_FLOAT_EQ(cs->visualProps.radius.topLeft, 3.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.radius.topRight, 5.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.radius.bottomLeft, 7.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.radius.bottomRight, 9.0f);
}

// -----------------------------------------------------------
// 18. Inherit — child inherits parent's shadow
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_InheritShadow)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="BaseShadow">
				<Shadow color="#AABBCCFF" x="5" y="6" b="7" s="8" />
			</Style>
			<Style class="DerivedShadow" inherit="BaseShadow">
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"DerivedShadow");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->visualProps.shadow.color, Color("#AABBCCFF"));
	EXPECT_FLOAT_EQ(cs->visualProps.shadow.offsetX, 5.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.shadow.offsetY, 6.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.shadow.blur, 7.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.shadow.spread, 8.0f);
}

// -----------------------------------------------------------
// 19. Inherit — child inherits parent's transform
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_InheritTransform)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="BaseTransform">
				<Transform>
					<Translation x="30" y="40" />
				</Transform>
			</Style>
			<Style class="DerivedTransform" inherit="BaseTransform">
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"DerivedTransform");
	ASSERT_NE(cs, nullptr);

	TransformF t = cs->visualProps.transform;
	PointF p(0, 0);
	PointF r = t.ApplyTo(p);
	EXPECT_FLOAT_EQ(r.x, 30.0f);
	EXPECT_FLOAT_EQ(r.y, 40.0f);
}

// -----------------------------------------------------------
// 20. Unknown style class returns default ComputedStyle
// -----------------------------------------------------------
TEST_F(StylesTest, Compute_UnknownClassReturnsDefault)
{
	auto cs = styles_->Compute(u8"NoSuchClass");
	ASSERT_NE(cs, nullptr);

	EXPECT_FLOAT_EQ(cs->visualProps.opacity, 1.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.radius.topLeft, 0.0f);
}

// -----------------------------------------------------------
// 21. Hot reload — previously-loaded inheritor picks up
//     updated base when base is reloaded alone.
// -----------------------------------------------------------
TEST_F(StylesTest, HotReload_BaseUpdated_OldInheritorSeesNewValues)
{
	// First load: Base width=100, Derived inherits Base.
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Base">
				<Dimension width="100" height="50" />
			</Style>
			<Style class="Derived" inherit="Base">
			</Style>
		</Styles>
	)");

	auto cs1 = styles_->Compute(u8"Derived");
	auto w1 = cs1->layoutProps.style.dimension(facebook::yoga::Dimension::Width);
	EXPECT_FLOAT_EQ(w1.value().unwrap(), 100.0f);

	// Hot reload: only Base changes, Derived is NOT in the new XML.
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="Base">
				<Dimension width="200" height="80" />
			</Style>
		</Styles>
	)");

	auto cs2 = styles_->Compute(u8"Derived");
	auto w2 = cs2->layoutProps.style.dimension(facebook::yoga::Dimension::Width);
	auto h2 = cs2->layoutProps.style.dimension(facebook::yoga::Dimension::Height);
	EXPECT_FLOAT_EQ(w2.value().unwrap(), 200.0f);
	EXPECT_FLOAT_EQ(h2.value().unwrap(), 80.0f);
}

// -----------------------------------------------------------
// 22. Hot reload — reverse XML order: inheritor defined
//     before base, still picks up base after reload.
// -----------------------------------------------------------
TEST_F(StylesTest, HotReload_ReverseOrder)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="DerivedRev" inherit="BaseRev">
			</Style>
			<Style class="BaseRev">
				<Dimension width="300" height="150" />
			</Style>
		</Styles>
	)");

	auto cs1 = styles_->Compute(u8"DerivedRev");
	auto w1 = cs1->layoutProps.style.dimension(facebook::yoga::Dimension::Width);
	EXPECT_FLOAT_EQ(w1.value().unwrap(), 300.0f);

	// Hot reload: reload Base with new values.
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="BaseRev">
				<Dimension width="400" height="200" />
			</Style>
		</Styles>
	)");

	auto cs2 = styles_->Compute(u8"DerivedRev");
	auto w2 = cs2->layoutProps.style.dimension(facebook::yoga::Dimension::Width);
	auto h2 = cs2->layoutProps.style.dimension(facebook::yoga::Dimension::Height);
	EXPECT_FLOAT_EQ(w2.value().unwrap(), 400.0f);
	EXPECT_FLOAT_EQ(h2.value().unwrap(), 200.0f);
}

// ============================================================
// Fixture: ComputedStyleTest — pure data-class tests
// ============================================================
class ComputedStyleTest : public ::testing::Test
{
};

// -----------------------------------------------------------
// 21. Default values
// -----------------------------------------------------------
TEST_F(ComputedStyleTest, DefaultValues)
{
	ComputedStyle cs;

	EXPECT_FLOAT_EQ(cs.visualProps.opacity, 1.0f);

	EXPECT_FLOAT_EQ(cs.visualProps.radius.topLeft, 0.0f);
	EXPECT_FLOAT_EQ(cs.visualProps.radius.topRight, 0.0f);
	EXPECT_FLOAT_EQ(cs.visualProps.radius.bottomLeft, 0.0f);
	EXPECT_FLOAT_EQ(cs.visualProps.radius.bottomRight, 0.0f);

	EXPECT_FLOAT_EQ(cs.visualProps.shadow.offsetX, 0.0f);
	EXPECT_FLOAT_EQ(cs.visualProps.shadow.offsetY, 0.0f);
	EXPECT_FLOAT_EQ(cs.visualProps.shadow.blur, 0.0f);
	EXPECT_FLOAT_EQ(cs.visualProps.shadow.spread, 0.0f);
}

// -----------------------------------------------------------
// 22. Diff — identical styles
// -----------------------------------------------------------
TEST_F(ComputedStyleTest, Diff_NoChange)
{
	ComputedStyle a;
	ComputedStyle b;

	auto result = a.Diff(&b);
	EXPECT_FALSE(result.layoutChanged);
	EXPECT_FALSE(result.visualChanged);
}

// -----------------------------------------------------------
// 23. Diff — detects changes
// -----------------------------------------------------------
TEST_F(ComputedStyleTest, Diff_DetectsChanges)
{
	ComputedStyle a;
	ComputedStyle b;

	b.visualProps.opacity = 0.5f;

	auto result = a.Diff(&b);
	EXPECT_FALSE(result.layoutChanged);
	EXPECT_TRUE(result.visualChanged);
}

// -----------------------------------------------------------
// 24. Diff against nullptr
// -----------------------------------------------------------
TEST_F(ComputedStyleTest, Diff_NullOther)
{
	ComputedStyle a;

	auto result = a.Diff(nullptr);
	EXPECT_TRUE(result.layoutChanged);
	EXPECT_TRUE(result.visualChanged);
}

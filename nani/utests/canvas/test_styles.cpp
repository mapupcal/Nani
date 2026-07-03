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
// Basic dimension: width / height
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
// Colors node
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
// Opacity
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
// Uniform border-radius
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
// Individual border-radius (tl, tr, bl, br)
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
// Shadow
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
// Transform — Translation, Rotation, Scaling, Shearing
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
// FlexBox node: flexDirection and direction
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithFlexBoxDirection)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="ColumnBox">
				<FlexBox flexDirection="column" direction="rtl" />
			</Style>
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
// FlexBox node: flex (grow)
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithFlexBoxFlex)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="FlexGrowBox">
				<FlexBox flex="2.5" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"FlexGrowBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_FLOAT_EQ(cs->layoutProps.style.flex().unwrap(), 2.5f);
}

// -----------------------------------------------------------
// Inherit — child inherits parent's FlexBox flexDirection, overrides flex, basis
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_InheritFlexBox)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="BaseFlex">
				<FlexBox flex="1.0" flexDirection="column" direction="rtl" grow="1.0" shrink="0.5" basis="20" />
			</Style>
			<Style class="DerivedFlex" inherit="BaseFlex">
				<FlexBox flex="2.0" basis="30%" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"DerivedFlex");
	ASSERT_NE(cs, nullptr);

	EXPECT_FLOAT_EQ(cs->layoutProps.style.flex().unwrap(), 2.0f);
	EXPECT_EQ(cs->layoutProps.style.flexDirection(),
		facebook::yoga::FlexDirection::Column);
	EXPECT_EQ(cs->layoutProps.style.direction(),
		facebook::yoga::Direction::RTL);
	EXPECT_EQ(cs->layoutProps.style.flexGrow().unwrap(),
		1.0f);
	EXPECT_EQ(cs->layoutProps.style.flexShrink().unwrap(),
		0.5f);
	EXPECT_EQ(cs->layoutProps.style.flexBasis().value().unwrap(),
		30.0f);
}

// -----------------------------------------------------------
// Reload — loading XML twice, second overwrites first
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
// Empty <Styles> node — no crash, Compute returns default
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
// Style with no class attribute — skipped
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
// Inherit — child inherits parent dimension
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
// Inherit — child overrides parent's width, inherits height
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
// Inherit — chain: Leaf -> Middle -> Base
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
// Inherit — non-existent parent, uses own values
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
// Inherit — child inherits parent's colors, overrides opacity
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
// Inherit — child inherits parent's border-radius
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
// Inherit — child inherits parent's shadow
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
// Inherit — child inherits parent's transform
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
// Inherit — child inherits parent's gap, gap.
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_InheritGaps)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="BaseGap">
				<Gaps gap ="2" />
			</Style>
			<Style class="DerivedGap" inherit="BaseGap">
			</Style>
		</Styles>
	)");

	auto pcs = styles_->Compute(u8"BaseGap");
	ASSERT_NE(pcs, nullptr);
	EXPECT_EQ(pcs->layoutProps.style.gap(facebook::yoga::Gutter::Row).value().unwrap(), 2.0f);
	EXPECT_EQ(pcs->layoutProps.style.gap(facebook::yoga::Gutter::Column).value().unwrap(), 2.0f);

	auto cs = styles_->Compute(u8"DerivedGap");
	ASSERT_NE(cs, nullptr);
	EXPECT_EQ(cs->layoutProps.style.gap(facebook::yoga::Gutter::Row).value().unwrap(), 2.0f);
	EXPECT_EQ(cs->layoutProps.style.gap(facebook::yoga::Gutter::Column).value().unwrap(), 2.0f);
}

// -----------------------------------------------------------
// Inherit — child inherits parent's gap, row and column.
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_InheritGapsRowAndColumn)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="BaseGap">
				<Gaps row="1" column="3" />
			</Style>
			<Style class="DerivedGap" inherit="BaseGap">
				<Gaps row="2" />
			</Style>
		</Styles>
	)");

	auto pcs = styles_->Compute(u8"BaseGap");
	ASSERT_NE(pcs, nullptr);
	EXPECT_EQ(pcs->layoutProps.style.gap(facebook::yoga::Gutter::Row).value().unwrap(), 1.0f);
	EXPECT_EQ(pcs->layoutProps.style.gap(facebook::yoga::Gutter::Column).value().unwrap(), 3.0f);

	auto cs = styles_->Compute(u8"DerivedGap");
	ASSERT_NE(cs, nullptr);
	EXPECT_EQ(cs->layoutProps.style.gap(facebook::yoga::Gutter::Row).value().unwrap(), 2.0f);
	EXPECT_EQ(cs->layoutProps.style.gap(facebook::yoga::Gutter::Column).value().unwrap(), 3.0f);
}


// -----------------------------------------------------------
// Inherit — child inherits parent's Edges.
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_InheritEdges)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="BaseEdges">
				<Margins value="1" />
				<Paddings l="2" t="3" r="4" b="5"/>
				<Borders l="6" r="7"/>
			</Style>
			<Style class="DerivedEdges" inherit="BaseEdges">
				<Borders t="8" b="9"/>
			</Style>
		</Styles>
	)");

	auto pcs = styles_->Compute(u8"BaseEdges");
	ASSERT_NE(pcs, nullptr);
	EXPECT_EQ(pcs->layoutProps.style.margin(facebook::yoga::Edge::Left).value().unwrap(), 1.0f);
	EXPECT_EQ(pcs->layoutProps.style.margin(facebook::yoga::Edge::Top).value().unwrap(), 1.0f);
	EXPECT_EQ(pcs->layoutProps.style.margin(facebook::yoga::Edge::Right).value().unwrap(), 1.0f);
	EXPECT_EQ(pcs->layoutProps.style.margin(facebook::yoga::Edge::Bottom).value().unwrap(), 1.0f);

	EXPECT_EQ(pcs->layoutProps.style.padding(facebook::yoga::Edge::Left).value().unwrap(), 2.0f);
	EXPECT_EQ(pcs->layoutProps.style.padding(facebook::yoga::Edge::Top).value().unwrap(), 3.0f);
	EXPECT_EQ(pcs->layoutProps.style.padding(facebook::yoga::Edge::Right).value().unwrap(), 4.0f);
	EXPECT_EQ(pcs->layoutProps.style.padding(facebook::yoga::Edge::Bottom).value().unwrap(), 5.0f);

	EXPECT_EQ(pcs->layoutProps.style.border(facebook::yoga::Edge::Left).value().unwrap(), 6.0f);
	EXPECT_TRUE(pcs->layoutProps.style.border(facebook::yoga::Edge::Top).isUndefined());
	EXPECT_EQ(pcs->layoutProps.style.border(facebook::yoga::Edge::Right).value().unwrap(), 7.0f);
	EXPECT_TRUE(pcs->layoutProps.style.border(facebook::yoga::Edge::Bottom).isUndefined());

	auto cs = styles_->Compute(u8"DerivedEdges");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->layoutProps.style.margin(facebook::yoga::Edge::Left).value().unwrap(), 1.0f);
	EXPECT_EQ(cs->layoutProps.style.margin(facebook::yoga::Edge::Top).value().unwrap(), 1.0f);
	EXPECT_EQ(cs->layoutProps.style.margin(facebook::yoga::Edge::Right).value().unwrap(), 1.0f);
	EXPECT_EQ(cs->layoutProps.style.margin(facebook::yoga::Edge::Bottom).value().unwrap(), 1.0f);

	EXPECT_EQ(cs->layoutProps.style.padding(facebook::yoga::Edge::Left).value().unwrap(), 2.0f);
	EXPECT_EQ(cs->layoutProps.style.padding(facebook::yoga::Edge::Top).value().unwrap(), 3.0f);
	EXPECT_EQ(cs->layoutProps.style.padding(facebook::yoga::Edge::Right).value().unwrap(), 4.0f);
	EXPECT_EQ(cs->layoutProps.style.padding(facebook::yoga::Edge::Bottom).value().unwrap(), 5.0f);

	EXPECT_TRUE(cs->layoutProps.style.border(facebook::yoga::Edge::Left).isUndefined());
	EXPECT_EQ(cs->layoutProps.style.border(facebook::yoga::Edge::Top).value().unwrap(), 8.0f);
	EXPECT_TRUE(cs->layoutProps.style.border(facebook::yoga::Edge::Right).isUndefined());
	EXPECT_EQ(cs->layoutProps.style.border(facebook::yoga::Edge::Bottom).value().unwrap(), 9.0f);
}

// -----------------------------------------------------------
// Unknown style class returns default ComputedStyle
// -----------------------------------------------------------
TEST_F(StylesTest, Compute_UnknownClassReturnsDefault)
{
	auto cs = styles_->Compute(u8"NoSuchClass");
	ASSERT_NE(cs, nullptr);

	EXPECT_FLOAT_EQ(cs->visualProps.opacity, 1.0f);
	EXPECT_FLOAT_EQ(cs->visualProps.radius.topLeft, 0.0f);
}

// -----------------------------------------------------------
// Hot reload — previously-loaded inheritor picks up
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
// Hot reload — reverse XML order: inheritor defined
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

// -----------------------------------------------------------
// FlexBox node: wrap
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithFlexBoxWrap)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="WrapBox">
				<FlexBox wrap="wrap" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"WrapBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->layoutProps.style.flexWrap(),
		facebook::yoga::Wrap::Wrap);
}

// -----------------------------------------------------------
// FlexBox node: wrap-reverse
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithFlexBoxWrapReverse)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="WrapRevBox">
				<FlexBox wrap="wrap-reverse" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"WrapRevBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->layoutProps.style.flexWrap(),
		facebook::yoga::Wrap::WrapReverse);
}

// -----------------------------------------------------------
// FlexBox node: justify
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithFlexBoxJustify)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="JustifyBox">
				<FlexBox justify="space-between" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"JustifyBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->layoutProps.style.justifyContent(),
		facebook::yoga::Justify::SpaceBetween);
}

// -----------------------------------------------------------
// FlexBox node: alignContent / alignItems / alignSelf
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithFlexBoxAlign)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="AlignBox">
				<FlexBox alignContent="center" alignItems="stretch" alignSelf="baseline" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"AlignBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->layoutProps.style.alignContent(),
		facebook::yoga::Align::Center);
	EXPECT_EQ(cs->layoutProps.style.alignItems(),
		facebook::yoga::Align::Stretch);
	EXPECT_EQ(cs->layoutProps.style.alignSelf(),
		facebook::yoga::Align::Baseline);
}

// -----------------------------------------------------------
// FlexBox node: overflow
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithFlexBoxOverflow)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="OverflowBox">
				<FlexBox overflow="scroll" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"OverflowBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->layoutProps.style.overflow(),
		facebook::yoga::Overflow::Scroll);
}

// -----------------------------------------------------------
// FlexBox node: aspectRatio
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithFlexBoxAspectRatio)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="AspectBox">
				<FlexBox aspectRatio="1.7777" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"AspectBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_FLOAT_EQ(cs->layoutProps.style.aspectRatio().unwrap(), 1.7777f);
}

// -----------------------------------------------------------
// FlexBox node: all new properties combined
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithFlexBoxAllNewProps)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="FullFlexBox">
				<FlexBox flex="1.5" flexDirection="row" direction="ltr"
					grow="2.0" shrink="0.5" basis="50%"
					wrap="wrap" justify="center"
					alignContent="start" alignItems="end" alignSelf="auto"
					overflow="hidden" aspectRatio="2.0" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"FullFlexBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_FLOAT_EQ(cs->layoutProps.style.flex().unwrap(), 1.5f);
	EXPECT_EQ(cs->layoutProps.style.flexDirection(),
		facebook::yoga::FlexDirection::Row);
	EXPECT_EQ(cs->layoutProps.style.direction(),
		facebook::yoga::Direction::LTR);
	EXPECT_FLOAT_EQ(cs->layoutProps.style.flexGrow().unwrap(), 2.0f);
	EXPECT_FLOAT_EQ(cs->layoutProps.style.flexShrink().unwrap(), 0.5f);
	EXPECT_EQ(cs->layoutProps.style.flexBasis().value().unwrap(), 50.0f);
	EXPECT_EQ(cs->layoutProps.style.flexWrap(),
		facebook::yoga::Wrap::Wrap);
	EXPECT_EQ(cs->layoutProps.style.justifyContent(),
		facebook::yoga::Justify::Center);
	EXPECT_EQ(cs->layoutProps.style.alignContent(),
		facebook::yoga::Align::FlexStart);
	EXPECT_EQ(cs->layoutProps.style.alignItems(),
		facebook::yoga::Align::FlexEnd);
	EXPECT_EQ(cs->layoutProps.style.alignSelf(),
		facebook::yoga::Align::Auto);
	EXPECT_EQ(cs->layoutProps.style.overflow(),
		facebook::yoga::Overflow::Hidden);
	EXPECT_FLOAT_EQ(cs->layoutProps.style.aspectRatio().unwrap(), 2.0f);
}

// -----------------------------------------------------------
// Positions node: position type
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithPositionsType)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="AbsoluteBox">
				<Positions type="absolute" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"AbsoluteBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->layoutProps.style.positionType(),
		facebook::yoga::PositionType::Absolute);
}

// -----------------------------------------------------------
// Positions node: position type static
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithPositionsTypeStatic)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="StaticBox">
				<Positions type="static" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"StaticBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->layoutProps.style.positionType(),
		facebook::yoga::PositionType::Static);
}

// -----------------------------------------------------------
// Positions node: edge values via value attribute
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithPositionsEdgesUniform)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="PosUniformBox">
				<Positions value="10" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"PosUniformBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Left).value().unwrap(), 10.0f);
	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Top).value().unwrap(), 10.0f);
	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Right).value().unwrap(), 10.0f);
	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Bottom).value().unwrap(), 10.0f);
}

// -----------------------------------------------------------
// Positions node: individual edge values
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithPositionsEdgesIndividual)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="PosEdgesBox">
				<Positions l="1" t="2" r="3" b="4" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"PosEdgesBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Left).value().unwrap(), 1.0f);
	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Top).value().unwrap(), 2.0f);
	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Right).value().unwrap(), 3.0f);
	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Bottom).value().unwrap(), 4.0f);
}

// -----------------------------------------------------------
// Positions node: type + edges combined
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_WithPositionsTypeAndEdges)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="PosCombinedBox">
				<Positions type="absolute" l="5" t="10" r="15" b="20" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"PosCombinedBox");
	ASSERT_NE(cs, nullptr);

	EXPECT_EQ(cs->layoutProps.style.positionType(),
		facebook::yoga::PositionType::Absolute);
	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Left).value().unwrap(), 5.0f);
	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Top).value().unwrap(), 10.0f);
	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Right).value().unwrap(), 15.0f);
	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Bottom).value().unwrap(), 20.0f);
}

// -----------------------------------------------------------
// Inherit — child inherits parent's FlexBox new props
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_InheritFlexBoxNewProps)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="BaseFlexNew">
				<FlexBox wrap="wrap" justify="center"
					alignContent="start" alignItems="end" alignSelf="auto"
					overflow="hidden" aspectRatio="1.5" />
			</Style>
			<Style class="DerivedFlexNew" inherit="BaseFlexNew">
				<FlexBox justify="space-around" overflow="scroll" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"DerivedFlexNew");
	ASSERT_NE(cs, nullptr);

	// Overridden by child
	EXPECT_EQ(cs->layoutProps.style.justifyContent(),
		facebook::yoga::Justify::SpaceAround);
	EXPECT_EQ(cs->layoutProps.style.overflow(),
		facebook::yoga::Overflow::Scroll);
	// Inherited from parent
	EXPECT_EQ(cs->layoutProps.style.flexWrap(),
		facebook::yoga::Wrap::Wrap);
	EXPECT_EQ(cs->layoutProps.style.alignContent(),
		facebook::yoga::Align::FlexStart);
	EXPECT_EQ(cs->layoutProps.style.alignItems(),
		facebook::yoga::Align::FlexEnd);
	EXPECT_EQ(cs->layoutProps.style.alignSelf(),
		facebook::yoga::Align::Auto);
	EXPECT_FLOAT_EQ(cs->layoutProps.style.aspectRatio().unwrap(), 1.5f);
}

// -----------------------------------------------------------
// Inherit — child inherits parent's Positions
// -----------------------------------------------------------
TEST_F(StylesTest, LoadFromXML_InheritPositions)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="BasePos">
				<Positions type="absolute" l="1" t="2" r="3" b="4" />
			</Style>
			<Style class="DerivedPos" inherit="BasePos">
				<Positions l="10" t="20" />
			</Style>
		</Styles>
	)");

	auto cs = styles_->Compute(u8"DerivedPos");
	ASSERT_NE(cs, nullptr);

	// Inherited type from parent
	EXPECT_EQ(cs->layoutProps.style.positionType(),
		facebook::yoga::PositionType::Absolute);
	// Overridden by child
	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Left).value().unwrap(), 10.0f);
	EXPECT_EQ(cs->layoutProps.style.position(facebook::yoga::Edge::Top).value().unwrap(), 20.0f);
	// Edges are replaced wholesale, not merged — Right and Bottom should be undefined
	EXPECT_TRUE(cs->layoutProps.style.position(facebook::yoga::Edge::Right).isUndefined());
	EXPECT_TRUE(cs->layoutProps.style.position(facebook::yoga::Edge::Bottom).isUndefined());
}

// ============================================================

// ============================================================
// State Polymorphism — automatic state builder generation
// ============================================================

// Basic: derived class inherits parent's state-specific styles.
TEST_F(StylesTest, StatePolymorphism_BasicInheritWithStates)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="button">
				<Colors background="#FFFFFFFF" border="#808080FF"/>
			</Style>
			<Style class="button" state="hovered">
				<Colors border="#FF00FFFF"/>
			</Style>
			<Style class="button" state="pressed">
				<Colors border="#FF00FFFF"/>
			</Style>
			<Style class="close-button" inherit="button">
				<Colors background="#FF000000" border="#808080FF"/>
			</Style>
		</Styles>
	)");

	// close-button normal: own background+button's border
	auto csNormal = styles_->Compute(u8"close-button");
	ASSERT_NE(csNormal, nullptr);
	EXPECT_EQ(csNormal->visualProps.backgroundColor, Color("#FF000000"));
	EXPECT_EQ(csNormal->visualProps.borderColor, Color("#808080FF"));

	// close-button hovered: button-hovered's border delta + close-button's own background
	auto csHovered = styles_->Compute(u8"close-button", u8"hovered");
	ASSERT_NE(csHovered, nullptr);
	EXPECT_EQ(csHovered->visualProps.borderColor, Color("#FF00FFFF"));
	EXPECT_EQ(csHovered->visualProps.backgroundColor, Color("#FF000000"));

	// close-button pressed: button-pressed's border delta + close-button's own background
	auto csPressed = styles_->Compute(u8"close-button", u8"pressed");
	ASSERT_NE(csPressed, nullptr);
	EXPECT_EQ(csPressed->visualProps.borderColor, Color("#FF00FFFF"));
	EXPECT_EQ(csPressed->visualProps.backgroundColor, Color("#FF000000"));
}

// Explicit state-specific builder beats implicit generation.
TEST_F(StylesTest, StatePolymorphism_ExplicitChildStateBeatsImplicit)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="button">
				<Colors border="#808080FF"/>
			</Style>
			<Style class="button" state="hovered">
				<Colors border="#FF00FFFF"/>
			</Style>
			<Style class="close-button" inherit="button">
				<Colors border="#0000FFFF"/>
			</Style>
			<Style class="close-button" state="hovered">
				<Colors border="#FFFF0000"/>
			</Style>
		</Styles>
	)");

	auto csHovered = styles_->Compute(u8"close-button", u8"hovered");
	ASSERT_NE(csHovered, nullptr);
	// Explicit child hovered beats implicit generation
	EXPECT_EQ(csHovered->visualProps.borderColor, Color("#FFFF0000"));
}

// Parent has no state-specific builder -> no implicit generated.
TEST_F(StylesTest, StatePolymorphism_ParentHasNoState)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="button">
				<Colors border="#808080FF"/>
			</Style>
			<Style class="close-button" inherit="button">
				<Colors border="#0000FFFF"/>
			</Style>
		</Styles>
	)");

	// close-button-hovered should fall back to close-button's normal (via Compute lookup fallback)
	auto csHovered = styles_->Compute(u8"close-button", u8"hovered");
	ASSERT_NE(csHovered, nullptr);
	// No implicit builder: falls back to class-only builder
	EXPECT_EQ(csHovered->visualProps.borderColor, Color("#0000FFFF"));
}

// Multi-level inheritance with state polymorphism.
TEST_F(StylesTest, StatePolymorphism_MultiLevelInheritance)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="grandparent">
				<Colors background="#FF000000" border="#808080FF"/>
			</Style>
			<Style class="grandparent" state="hovered">
				<Colors border="#FF00FFFF"/>
			</Style>
			<Style class="parent" inherit="grandparent">
				<Colors background="#0000FF00"/>
			</Style>
			<Style class="child" inherit="parent">
				<Colors background="#000000FF"/>
			</Style>
		</Styles>
	)");

	// childHovered: grandparent-hovered border delta + child's own background
	auto csHovered = styles_->Compute(u8"child", u8"hovered");
	ASSERT_NE(csHovered, nullptr);
	EXPECT_EQ(csHovered->visualProps.borderColor, Color("#FF00FFFF"));
	EXPECT_EQ(csHovered->visualProps.backgroundColor, Color("#000000FF"));

	// parentHovered: grandparent-hovered border delta + parent's own background
	auto pcsHovered = styles_->Compute(u8"parent", u8"hovered");
	ASSERT_NE(pcsHovered, nullptr);
	EXPECT_EQ(pcsHovered->visualProps.borderColor, Color("#FF00FFFF"));
	EXPECT_EQ(pcsHovered->visualProps.backgroundColor, Color("#0000FF00"));
}

// Hot reload: implicit builder picks up updated parent state.
TEST_F(StylesTest, StatePolymorphism_HotReload)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="button">
				<Colors border="#808080FF"/>
			</Style>
			<Style class="button" state="hovered">
				<Colors border="#FF00FFFF"/>
			</Style>
			<Style class="close-button" inherit="button">
				<Colors border="#0000FFFF"/>
			</Style>
		</Styles>
	)");

	auto cs1 = styles_->Compute(u8"close-button", u8"hovered");
	ASSERT_NE(cs1, nullptr);
	EXPECT_EQ(cs1->visualProps.borderColor, Color("#FF00FFFF"));

	// Hot reload: button-hovered changes border color
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="button">
				<Colors border="#808080FF"/>
			</Style>
			<Style class="button" state="hovered">
				<Colors border="#FF0000FF"/>
			</Style>
		</Styles>
	)");

	auto cs2 = styles_->Compute(u8"close-button", u8"hovered");
	ASSERT_NE(cs2, nullptr);
	EXPECT_EQ(cs2->visualProps.borderColor, Color("#FF0000FF"));
}

// StatePolymorphism via stand-alone Compute key
TEST_F(StylesTest, StatePolymorphism_ViaStandaloneCompute)
{
	styles_->LoadFromXML(R"(
		<Styles>
			<Style class="button">
				<Colors background="#FFFFFFFF" border="#808080FF"/>
			</Style>
			<Style class="button" state="hovered">
				<Colors border="#FF00FFFF"/>
			</Style>
			<Style class="close-button" inherit="button">
				<Colors background="#FF000000"/>
			</Style>
		</Styles>
	)");

	// Verify the implicit close-button-hovered exists and delta computation
	auto cs = styles_->Compute(u8"close-button", u8"hovered");
	ASSERT_NE(cs, nullptr);
	EXPECT_EQ(cs->visualProps.borderColor, Color("#FF00FFFF"));  // from button-hovered delta
	EXPECT_EQ(cs->visualProps.backgroundColor, Color("#FF000000"));  // from close-button own
}

// ============================================================
// Fixture: ComputedStyleTest — pure data-class tests
// ============================================================
class ComputedStyleTest : public ::testing::Test
{
};

// -----------------------------------------------------------
// Default values
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
// Diff — identical styles
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
// Diff — detects changes
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
// Diff against nullptr
// -----------------------------------------------------------
TEST_F(ComputedStyleTest, Diff_NullOther)
{
	ComputedStyle a;

	auto result = a.Diff(nullptr);
	EXPECT_TRUE(result.layoutChanged);
	EXPECT_TRUE(result.visualChanged);
}

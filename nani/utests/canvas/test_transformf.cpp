#include <gtest/gtest.h>
#include "canvas/basic/transformf.h"
#include "canvas/basic/pointf.h"
#include "canvas/basic/polygonf.h"
#include "canvas/basic/rectf.h"
#include "canvas/basic/geometry_defs.h"

using namespace nani::canvas::basic;

class TransformFTest : public ::testing::Test
{
};

TEST_F(TransformFTest, DefaultConstructorIsIdentity)
{
    TransformF t;

    EXPECT_FLOAT_EQ(t.m11, 1.0f);
    EXPECT_FLOAT_EQ(t.m12, 0.0f);
    EXPECT_FLOAT_EQ(t.dx, 0.0f);
    EXPECT_FLOAT_EQ(t.m21, 0.0f);
    EXPECT_FLOAT_EQ(t.m22, 1.0f);
    EXPECT_FLOAT_EQ(t.dy, 0.0f);
}

TEST_F(TransformFTest, IdentityApplyToPointReturnsSamePoint)
{
    TransformF t;
    PointF pt(3.0f, 4.0f);

    PointF result = t.ApplyTo(pt);

    EXPECT_EQ(result, pt);
}

TEST_F(TransformFTest, TranslationMovesPoint)
{
    TransformF t = TransformF::Translation(5.0f, -3.0f);
    PointF result = t.ApplyTo(PointF(2.0f, 7.0f));

    EXPECT_EQ(result, PointF(7.0f, 4.0f));
}

TEST_F(TransformFTest, Rotation90Degrees)
{
    TransformF t = TransformF::Rotation(Degree2Radian(90.0f));
    PointF result = t.ApplyTo(PointF(1.0f, 0.0f));

    EXPECT_TRUE(IsScalarEqual(result.x, 0.0f));
    EXPECT_TRUE(IsScalarEqual(result.y, 1.0f));
}

TEST_F(TransformFTest, Rotation180Degrees)
{
    TransformF t = TransformF::Rotation(Degree2Radian(180.0f));
    PointF result = t.ApplyTo(PointF(1.0f, 0.0f));

    EXPECT_TRUE(IsScalarEqual(result.x, -1.0f));
    EXPECT_TRUE(IsScalarEqual(result.y, 0.0f));
}

TEST_F(TransformFTest, Rotation360DegreesReturnsToOriginal)
{
    TransformF t = TransformF::Rotation(Degree2Radian(360.0f));
    PointF result = t.ApplyTo(PointF(3.0f, 4.0f));

    EXPECT_TRUE(IsScalarEqual(result.x, 3.0f));
    EXPECT_TRUE(IsScalarEqual(result.y, 4.0f));
}

TEST_F(TransformFTest, ScalingUniform)
{
    TransformF t = TransformF::Scaling(2.0f, 2.0f);
    PointF result = t.ApplyTo(PointF(3.0f, 4.0f));

    EXPECT_EQ(result, PointF(6.0f, 8.0f));
}

TEST_F(TransformFTest, ScalingNonUniform)
{
    TransformF t = TransformF::Scaling(2.0f, 0.5f);
    PointF result = t.ApplyTo(PointF(3.0f, 4.0f));

    EXPECT_EQ(result, PointF(6.0f, 2.0f));
}

TEST_F(TransformFTest, ShearingX)
{
    TransformF t = TransformF::Shearing(2.0f, 0.0f);
    PointF result = t.ApplyTo(PointF(3.0f, 4.0f));

    EXPECT_EQ(result, PointF(11.0f, 4.0f));
}

TEST_F(TransformFTest, ShearingY)
{
    TransformF t = TransformF::Shearing(0.0f, 2.0f);
    PointF result = t.ApplyTo(PointF(3.0f, 4.0f));

    EXPECT_EQ(result, PointF(3.0f, 10.0f));
}

TEST_F(TransformFTest, RotationAroundAnchor)
{
    PointF anchor(5.0f, 5.0f);
    TransformF t = TransformF::RotationAround(anchor, Degree2Radian(90.0f));
    PointF result = t.ApplyTo(PointF(5.0f, 0.0f));

    EXPECT_TRUE(IsScalarEqual(result.x, 10.0f));
    EXPECT_TRUE(IsScalarEqual(result.y, 5.0f));
}

TEST_F(TransformFTest, ScalingAroundAnchor)
{
    PointF anchor(5.0f, 5.0f);
    TransformF t = TransformF::ScalingAround(anchor, 2.0f, 2.0f);
    PointF result = t.ApplyTo(PointF(5.0f, 0.0f));

    EXPECT_TRUE(IsScalarEqual(result.x, 5.0f));
    EXPECT_TRUE(IsScalarEqual(result.y, -5.0f));
}

TEST_F(TransformFTest, ShearingAroundAnchor)
{
    {
        PointF anchor(0.0f, 5.0f);
        TransformF t = TransformF::ShearingAround(anchor, 2.0f, 0.0f);
        PointF result = t.ApplyTo(PointF(3.0f, 5.0f));

        EXPECT_TRUE(IsScalarEqual(result.x, 3.0f));
        EXPECT_TRUE(IsScalarEqual(result.y, 5.0f));
    }

    {
        PointF anchor(0.0f, 0.0f);
        TransformF t = TransformF::ShearingAround(anchor, 2.0f, 0.0f);
        PointF result = t.ApplyTo(PointF(3.0f, 5.0f));
        EXPECT_TRUE(IsScalarEqual(result.x, 13.0f));
        EXPECT_TRUE(IsScalarEqual(result.y, 5.0f));
    }
}

TEST_F(TransformFTest, ComposeMultipleTransforms)
{
    TransformF t = TransformF::Compose({
        TransformF::Translation(1.0f, 0.0f),
        TransformF::Translation(0.0f, 2.0f),
        TransformF::Scaling(3.0f, 3.0f)
    });

    PointF result = t.ApplyTo(PointF(1.0f, 1.0f));

    EXPECT_TRUE(IsScalarEqual(result.x, 6.0f));
    EXPECT_TRUE(IsScalarEqual(result.y, 9.0f));
}

TEST_F(TransformFTest, AppendChainsTransforms)
{
    TransformF t = TransformF::Translation(1.0f, 0.0f);
    t.Append(TransformF::Translation(0.0f, 2.0f));

    PointF result = t.ApplyTo(PointF(0.0f, 0.0f));

    EXPECT_EQ(result, PointF(1.0f, 2.0f));
}

TEST_F(TransformFTest, PrependPrependsTransform)
{
    TransformF t = TransformF::Translation(0.0f, 2.0f);
    t.Prepend(TransformF::Translation(1.0f, 0.0f));

    PointF result = t.ApplyTo(PointF(0.0f, 0.0f));

    EXPECT_EQ(result, PointF(1.0f, 2.0f));
}

TEST_F(TransformFTest, AppendAndPrependOrderMatters)
{
    TransformF scale = TransformF::Scaling(2.0f, 2.0f);
    TransformF translate = TransformF::Translation(3.0f, 0.0f);

    TransformF scaleThenTranslate = scale;
    scaleThenTranslate.Append(translate);
    PointF resultST = scaleThenTranslate.ApplyTo(PointF(1.0f, 0.0f));
    EXPECT_EQ(resultST, PointF(5.0f, 0.0f));

    TransformF translateThenScale = scale;
    translateThenScale.Prepend(translate);
    PointF resultTS = translateThenScale.ApplyTo(PointF(1.0f, 0.0f));
    EXPECT_EQ(resultTS, PointF(8.0f, 0.0f));
}

TEST_F(TransformFTest, ThenReturnsNewTransform)
{
    TransformF t = TransformF::Translation(1.0f, 0.0f);
    TransformF combined = t.Then(TransformF::Translation(0.0f, 2.0f));

    PointF result = combined.ApplyTo(PointF(0.0f, 0.0f));
    EXPECT_EQ(result, PointF(1.0f, 2.0f));

    PointF originalResult = t.ApplyTo(PointF(0.0f, 0.0f));
    EXPECT_EQ(originalResult, PointF(1.0f, 0.0f));
}

TEST_F(TransformFTest, ReversedInvertsTranslation)
{
    TransformF t = TransformF::Translation(5.0f, -3.0f);
    TransformF inv = t.Reversed();

    PointF pt(10.0f, 20.0f);
    PointF result = inv.ApplyTo(t.ApplyTo(pt));

    EXPECT_TRUE(IsScalarEqual(result.x, pt.x));
    EXPECT_TRUE(IsScalarEqual(result.y, pt.y));
}

TEST_F(TransformFTest, ReversedInvertsRotation)
{
    TransformF t = TransformF::Rotation(Degree2Radian(45.0f));
    TransformF inv = t.Reversed();

    PointF pt(3.0f, 4.0f);
    PointF result = inv.ApplyTo(t.ApplyTo(pt));

    EXPECT_TRUE(IsScalarEqual(result.x, pt.x));
    EXPECT_TRUE(IsScalarEqual(result.y, pt.y));
}

TEST_F(TransformFTest, ReversedInvertsScale)
{
    TransformF t = TransformF::Scaling(2.0f, 3.0f);
    TransformF inv = t.Reversed();

    PointF pt(3.0f, 4.0f);
    PointF result = inv.ApplyTo(t.ApplyTo(pt));

    EXPECT_TRUE(IsScalarEqual(result.x, pt.x));
    EXPECT_TRUE(IsScalarEqual(result.y, pt.y));
}

TEST_F(TransformFTest, ReversedOfSingularMatrixReturnsIdentity)
{
    TransformF t = TransformF::Scaling(0.0f, 1.0f);
    TransformF inv = t.Reversed();

    EXPECT_FLOAT_EQ(inv.m11, 1.0f);
    EXPECT_FLOAT_EQ(inv.m12, 0.0f);
    EXPECT_FLOAT_EQ(inv.dx, 0.0f);
    EXPECT_FLOAT_EQ(inv.m21, 0.0f);
    EXPECT_FLOAT_EQ(inv.m22, 1.0f);
    EXPECT_FLOAT_EQ(inv.dy, 0.0f);
}

TEST_F(TransformFTest, ApplyToPolygonTransformsAllVertices)
{
    PolygonF triangle(PointF(0.0f, 0.0f), PointF(1.0f, 0.0f), PointF(0.0f, 1.0f));
    TransformF t = TransformF::Translation(10.0f, 20.0f);

    PolygonF result = t.ApplyTo(triangle);

    ASSERT_EQ(result.vertices.size(), 3u);
    EXPECT_EQ(result.vertices[0], PointF(10.0f, 20.0f));
    EXPECT_EQ(result.vertices[1], PointF(11.0f, 20.0f));
    EXPECT_EQ(result.vertices[2], PointF(10.0f, 21.0f));
}

TEST_F(TransformFTest, ApplyToEmptyPolygonReturnsEmpty)
{
    PolygonF empty;
    TransformF t = TransformF::Translation(10.0f, 20.0f);

    PolygonF result = t.ApplyTo(empty);

    EXPECT_TRUE(result.vertices.empty());
}

TEST_F(TransformFTest, AssignmentOperatorCopiesValues)
{
    TransformF original = TransformF::Translation(3.0f, 4.0f);
    TransformF copy;
    copy = original;

    EXPECT_FLOAT_EQ(copy.m11, original.m11);
    EXPECT_FLOAT_EQ(copy.m12, original.m12);
    EXPECT_FLOAT_EQ(copy.dx, original.dx);
    EXPECT_FLOAT_EQ(copy.m21, original.m21);
    EXPECT_FLOAT_EQ(copy.m22, original.m22);
    EXPECT_FLOAT_EQ(copy.dy, original.dy);

    PointF result = copy.ApplyTo(PointF(1.0f, 1.0f));
    EXPECT_EQ(result, PointF(4.0f, 5.0f));
}

TEST_F(TransformFTest, TranslationThenRotation)
{
    TransformF t = TransformF::Compose({
        TransformF::Translation(2.0f, 0.0f),
        TransformF::Rotation(Degree2Radian(90.0f))
    });

    PointF result = t.ApplyTo(PointF(1.0f, 0.0f));

    EXPECT_TRUE(IsScalarEqual(result.x, 0.0f));
    EXPECT_TRUE(IsScalarEqual(result.y, 3.0f));
}

TEST_F(TransformFTest, ComposeWithEmptyListReturnsIdentity)
{
    TransformF t = TransformF::Compose({});

    EXPECT_FLOAT_EQ(t.m11, 1.0f);
    EXPECT_FLOAT_EQ(t.m22, 1.0f);
    EXPECT_FLOAT_EQ(t.dx, 0.0f);
    EXPECT_FLOAT_EQ(t.dy, 0.0f);
}

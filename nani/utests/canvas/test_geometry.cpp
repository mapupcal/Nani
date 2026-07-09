#include <gtest/gtest.h>
#include "defs.h"

class GeometryTest : public ::testing::Test
{
};

TEST_F(GeometryTest, PointFSupportsVectorOperations)
{
	PointF defaultPoint;
	EXPECT_EQ(defaultPoint, PointF(0.0f, 0.0f));

	PointF point(3.0f, 4.0f);
	EXPECT_EQ(point + PointF(1.0f, 2.0f), PointF(4.0f, 6.0f));
	EXPECT_EQ(point - PointF(1.0f, 2.0f), PointF(2.0f, 2.0f));
	EXPECT_EQ(point * 2.0f, PointF(6.0f, 8.0f));
	EXPECT_EQ(2.0f * point, PointF(6.0f, 8.0f));
	EXPECT_EQ(point / 2.0f, PointF(1.5f, 2.0f));

	point += PointF(1.0f, -1.0f);
	EXPECT_EQ(point, PointF(4.0f, 3.0f));

	point -= PointF(2.0f, 1.0f);
	EXPECT_EQ(point, PointF(2.0f, 2.0f));

	point *= 3.0f;
	EXPECT_EQ(point, PointF(6.0f, 6.0f));

	point /= 2.0f;
	EXPECT_EQ(point, PointF(3.0f, 3.0f));
}

TEST_F(GeometryTest, PointFCalculatesDotCrossAndDistance)
{
	PointF first(3.0f, 4.0f);
	PointF second(6.0f, 8.0f);

	EXPECT_FLOAT_EQ(first.Dot(second), 50.0f);
	EXPECT_FLOAT_EQ(first.Cross(second), 0.0f);
	EXPECT_FLOAT_EQ(first.DistanceTo(second), 5.0f);

	EXPECT_FLOAT_EQ(PointF(1.0f, 0.0f).Cross(PointF(0.0f, 1.0f)), 1.0f);
}

TEST_F(GeometryTest, PointFUnaryNegation)
{
	PointF p(3.0f, -4.0f);
	PointF neg = -p;

	EXPECT_EQ(neg, PointF(-3.0f, 4.0f));
}

TEST_F(GeometryTest, SizeFSupportsArithmeticAndScaling)
{
	SizeF defaultSize;
	EXPECT_EQ(defaultSize, SizeF(0.0f, 0.0f));
	EXPECT_TRUE(defaultSize.IsValid());

	SizeF size(10.0f, 20.0f);
	EXPECT_EQ(size + SizeF(2.0f, 3.0f), SizeF(12.0f, 23.0f));
	EXPECT_EQ(size - SizeF(2.0f, 3.0f), SizeF(8.0f, 17.0f));

	size += SizeF(1.0f, 2.0f);
	EXPECT_EQ(size, SizeF(11.0f, 22.0f));

	size -= SizeF(3.0f, 4.0f);
	EXPECT_EQ(size, SizeF(8.0f, 18.0f));

	EXPECT_EQ(size.Scaled(2.0f, 0.5f), SizeF(16.0f, 9.0f));
	EXPECT_EQ(size, SizeF(8.0f, 18.0f));

	size.Scale(0.5f, 2.0f);
	EXPECT_EQ(size, SizeF(4.0f, 36.0f));

	EXPECT_FALSE(SizeF(-1.0f, 1.0f).IsValid());
	EXPECT_FALSE(SizeF(1.0f, -1.0f).IsValid());
}

TEST_F(GeometryTest, SizeFTransposesDimensions)
{
	SizeF size(10.0f, 20.0f);

	EXPECT_EQ(size.Transposed(), SizeF(20.0f, 10.0f));
	EXPECT_EQ(size, SizeF(10.0f, 20.0f));

	size.Transpose();
	EXPECT_EQ(size, SizeF(20.0f, 10.0f));
}

TEST_F(GeometryTest, SizeFScaleXY)
{
	SizeF size(10.0f, 20.0f);

	size.ScaleX(2.0f);
	EXPECT_EQ(size, SizeF(20.0f, 20.0f));

	size.ScaleY(0.5f);
	EXPECT_EQ(size, SizeF(20.0f, 10.0f));
}

TEST_F(GeometryTest, RectFConstructorsNormalizeAndExposeGeometry)
{
	RectF defaultRect;
	EXPECT_EQ(defaultRect, RectF(0.0f, 0.0f, 0.0f, 0.0f));

	RectF fromSize(SizeF(20.0f, 10.0f));
	EXPECT_EQ(fromSize, RectF(0.0f, 0.0f, 20.0f, 10.0f));

	RectF fromPositionAndSize(5.0f, 6.0f, SizeF(20.0f, 10.0f));
	EXPECT_EQ(fromPositionAndSize, RectF(5.0f, 6.0f, 25.0f, 16.0f));

	RectF fromTopLeft(PointF(5.0f, 6.0f), 20.0f, 10.0f);
	EXPECT_EQ(fromTopLeft, RectF(5.0f, 6.0f, 25.0f, 16.0f));

	RectF fromPoints(PointF(25.0f, 16.0f), PointF(5.0f, 6.0f));
	EXPECT_EQ(fromPoints, RectF(5.0f, 6.0f, 25.0f, 16.0f));

	EXPECT_EQ(fromPoints.X(), 5.0f);
	EXPECT_EQ(fromPoints.Y(), 6.0f);
	EXPECT_EQ(fromPoints.TopLeft(), PointF(5.0f, 6.0f));
	EXPECT_EQ(fromPoints.TopRight(), PointF(25.0f, 6.0f));
	EXPECT_EQ(fromPoints.BottomLeft(), PointF(5.0f, 16.0f));
	EXPECT_EQ(fromPoints.BottomRight(), PointF(25.0f, 16.0f));
	EXPECT_EQ(fromPoints.Center(), PointF(15.0f, 11.0f));
	EXPECT_EQ(fromPoints.Size(), SizeF(20.0f, 10.0f));
	EXPECT_FLOAT_EQ(fromPoints.Area(), 200.0f);
	EXPECT_TRUE(fromPoints.IsValid());
}

TEST_F(GeometryTest, RectFMutatesSizeAndPosition)
{
	RectF rect(1.0f, 2.0f, 11.0f, 22.0f);

	rect.SetWidth(20.0f);
	EXPECT_EQ(rect, RectF(1.0f, 2.0f, 21.0f, 22.0f));

	rect.SetHeight(30.0f);
	EXPECT_EQ(rect, RectF(1.0f, 2.0f, 21.0f, 32.0f));

	rect.SetSize(SizeF(5.0f, 6.0f));
	EXPECT_EQ(rect, RectF(1.0f, 2.0f, 6.0f, 8.0f));

	rect.MoveTo(PointF(10.0f, 20.0f));
	EXPECT_EQ(rect, RectF(10.0f, 20.0f, 15.0f, 26.0f));

	EXPECT_EQ(rect.Transposed(), RectF(10.0f, 20.0f, 16.0f, 25.0f));
	EXPECT_EQ(rect, RectF(10.0f, 20.0f, 15.0f, 26.0f));

	rect.Transpose();
	EXPECT_EQ(rect, RectF(10.0f, 20.0f, 16.0f, 25.0f));

	rect.SetSize(100.0f, 200.0f);
	EXPECT_EQ(rect, RectF(10.0f, 20.0f, 110.0f, 220.0f));
}

TEST_F(GeometryTest, RectFIsValid)
{
	EXPECT_TRUE(RectF().IsValid());
	EXPECT_TRUE(RectF(0, 0, 10, 10).IsValid());
	EXPECT_FALSE(RectF(10, 10, 0, 0).IsValid());
}

TEST_F(GeometryTest, RectFContainsBoundaryPoints)
{
	RectF rect(10.0f, 20.0f, 30.0f, 40.0f);

	EXPECT_TRUE(rect.IsContains(PointF(10.0f, 20.0f)));
	EXPECT_FALSE(rect.IsContains(PointF(30.0f, 40.0f)));
	EXPECT_TRUE(rect.IsContains(PointF(20.0f, 30.0f)));
	EXPECT_FALSE(rect.IsContains(PointF(9.9f, 20.0f)));
	EXPECT_FALSE(rect.IsContains(PointF(30.1f, 40.0f)));
}

TEST_F(GeometryTest, RectFCalculatesBoundsAndIntersections)
{
	RectF rect(0.0f, 0.0f, 10.0f, 10.0f);
	RectF other(5.0f, -5.0f, 15.0f, 5.0f);

	EXPECT_EQ(rect.Bounded(other), RectF(0.0f, -5.0f, 15.0f, 10.0f));
	EXPECT_EQ(rect.Intersected(other), RectF(5.0f, 0.0f, 10.0f, 5.0f));
	EXPECT_EQ(rect | other, RectF(0.0f, -5.0f, 15.0f, 10.0f));
	EXPECT_EQ(rect & other, RectF(5.0f, 0.0f, 10.0f, 5.0f));

	rect |= other;
	EXPECT_EQ(rect, RectF(0.0f, -5.0f, 15.0f, 10.0f));

	rect &= RectF(2.0f, 2.0f, 8.0f, 8.0f);
	EXPECT_EQ(rect, RectF(2.0f, 2.0f, 8.0f, 8.0f));
}

TEST_F(GeometryTest, MarginsFConstructorsAndComparison)
{
	MarginsF defaultMargins;
	EXPECT_EQ(defaultMargins, MarginsF(0.0f, 0.0f, 0.0f, 0.0f));

	MarginsF margins(1.0f, 2.0f, 3.0f, 4.0f);
	EXPECT_EQ(margins.left, 1.0f);
	EXPECT_EQ(margins.top, 2.0f);
	EXPECT_EQ(margins.right, 3.0f);
	EXPECT_EQ(margins.bottom, 4.0f);

	MarginsF copied(margins);
	EXPECT_EQ(copied, margins);

	MarginsF assigned;
	assigned = margins;
	EXPECT_EQ(assigned, margins);
	EXPECT_NE(assigned, MarginsF());
}

TEST_F(GeometryTest, MarginsFSupportsArithmetic)
{
	MarginsF first(1.0f, 2.0f, 3.0f, 4.0f);
	MarginsF second(0.5f, 1.0f, 1.5f, 2.0f);

	EXPECT_EQ(first + second, MarginsF(1.5f, 3.0f, 4.5f, 6.0f));
	EXPECT_EQ(first - second, MarginsF(0.5f, 1.0f, 1.5f, 2.0f));
	EXPECT_EQ(-first, MarginsF(-1.0f, -2.0f, -3.0f, -4.0f));

	first += second;
	EXPECT_EQ(first, MarginsF(1.5f, 3.0f, 4.5f, 6.0f));

	first -= MarginsF(0.5f, 1.0f, 1.5f, 2.0f);
	EXPECT_EQ(first, MarginsF(1.0f, 2.0f, 3.0f, 4.0f));

	first *= 2.0f;
	EXPECT_EQ(first, MarginsF(2.0f, 4.0f, 6.0f, 8.0f));

	first /= 4.0f;
	EXPECT_EQ(first, MarginsF(0.5f, 1.0f, 1.5f, 2.0f));
}

TEST_F(GeometryTest, MarginsFInsetsAndOutsetsRectF)
{
	RectF rect(10.0f, 20.0f, 30.0f, 40.0f);
	MarginsF inset(1.0f, 2.0f, 3.0f, 4.0f);

	EXPECT_EQ(rect - inset, RectF(11.0f, 22.0f, 27.0f, 36.0f));
	EXPECT_EQ(rect + inset, RectF(9.0f, 18.0f, 33.0f, 44.0f));

	MarginsF border(1.0f, 1.0f, 1.0f, 1.0f);
	MarginsF padding(2.0f, 3.0f, 4.0f, 5.0f);
	EXPECT_EQ(rect - (border + padding), RectF(13.0f, 24.0f, 25.0f, 34.0f));
}

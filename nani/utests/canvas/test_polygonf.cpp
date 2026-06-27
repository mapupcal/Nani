#include <gtest/gtest.h>
#include "defs.h"

class PolygonFTest : public ::testing::Test
{
};

TEST_F(PolygonFTest, DefaultConstructorCreatesEmptyPolygon)
{
    PolygonF polygon;
    EXPECT_TRUE(polygon.vertices.empty());
}

TEST_F(PolygonFTest, TriangleConstructorCreatesThreeVertices)
{
    PolygonF triangle(PointF(0.0f, 0.0f), PointF(1.0f, 0.0f), PointF(0.0f, 1.0f));

    ASSERT_EQ(triangle.vertices.size(), 3u);
    EXPECT_EQ(triangle.vertices[0], PointF(0.0f, 0.0f));
    EXPECT_EQ(triangle.vertices[1], PointF(1.0f, 0.0f));
    EXPECT_EQ(triangle.vertices[2], PointF(0.0f, 1.0f));
}

TEST_F(PolygonFTest, RectConstructorCreatesFourVertices)
{
    RectF rect(1.0f, 2.0f, 5.0f, 6.0f);
    PolygonF polygon(rect);

    ASSERT_EQ(polygon.vertices.size(), 4u);
    EXPECT_EQ(polygon.vertices[0], rect.TopLeft());
    EXPECT_EQ(polygon.vertices[1], rect.TopRight());
    EXPECT_EQ(polygon.vertices[2], rect.BottomRight());
    EXPECT_EQ(polygon.vertices[3], rect.BottomLeft());
}

TEST_F(PolygonFTest, BoundingBoxOfEmptyPolygonReturnsDefaultRect)
{
    PolygonF polygon;
    RectF box = polygon.BoundingBox();

    EXPECT_EQ(box, RectF());
}

TEST_F(PolygonFTest, BoundingBoxOfTriangle)
{
    PolygonF triangle(PointF(2.0f, 3.0f), PointF(8.0f, 5.0f), PointF(4.0f, 9.0f));
    RectF box = triangle.BoundingBox();

    EXPECT_FLOAT_EQ(box.X(), 2.0f);
    EXPECT_FLOAT_EQ(box.Y(), 3.0f);
    EXPECT_FLOAT_EQ(box.Width(), 6.0f);
    EXPECT_FLOAT_EQ(box.Height(), 6.0f);
}

TEST_F(PolygonFTest, BoundingBoxOfRectPolygon)
{
    RectF rect(10.0f, 20.0f, 30.0f, 40.0f);
    PolygonF polygon(rect);
    RectF box = polygon.BoundingBox();

    EXPECT_EQ(box, rect);
}

TEST_F(PolygonFTest, BoundingBoxOfSinglePointPolygon)
{
    PolygonF polygon;
    polygon.vertices.push_back(PointF(5.0f, 7.0f));
    RectF box = polygon.BoundingBox();

    EXPECT_FLOAT_EQ(box.X(), 5.0f);
    EXPECT_FLOAT_EQ(box.Y(), 7.0f);
    EXPECT_FLOAT_EQ(box.Width(), 0.0f);
    EXPECT_FLOAT_EQ(box.Height(), 0.0f);
}

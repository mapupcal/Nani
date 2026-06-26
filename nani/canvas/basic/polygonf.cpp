#include "polygonf.h"
namespace nani::canvas::basic
{
	PolygonF::PolygonF(const PointF& p1, const PointF& p2, const PointF& p3)
		: vertices{ p1,p2,p3 }
	{

	}

	PolygonF::PolygonF(const RectF& rectangle)
		: vertices{ 
			rectangle.TopLeft(), rectangle.TopRight(),
			rectangle.BottomRight(), rectangle.BottomLeft()
		}
	{

	}

	const RectF PolygonF::BoundingBox() const
	{
		if (vertices.empty())
		{
			return RectF();
		}

		float minX = vertices[0].x;
		float maxX = vertices[0].x;
		float minY = vertices[0].y;
		float maxY = vertices[0].y;

		for (const auto& vertex : vertices)
		{
			if (vertex.x < minX) minX = vertex.x;
			if (vertex.x > maxX) maxX = vertex.x;
			if (vertex.y < minY) minY = vertex.y;
			if (vertex.y > maxY) maxY = vertex.y;
		}

		return RectF(minX, minY, maxX, maxY);
	}
}
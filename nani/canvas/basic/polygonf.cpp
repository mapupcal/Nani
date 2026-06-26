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

		return RectF(minX, minY, maxX - minX, maxY - minY);
	}

	bool PolygonF::IsContains(const PointF& pt) const
	{
		if (vertices.size() < 3)
		{
			return false;
		}

		// Ray casting algorithm: count intersections of a horizontal ray from the point
		bool inside = false;
		size_t n = vertices.size();

		for (size_t i = 0, j = n - 1; i < n; j = i++)
		{
			const PointF& vi = vertices[i];
			const PointF& vj = vertices[j];

			// Check if the ray intersects with edge (vi, vj)
			bool intersect = ((vi.y > pt.y) != (vj.y > pt.y)) &&
				(pt.x < (vj.x - vi.x) * (pt.y - vi.y) / (vj.y - vi.y) + vi.x);

			if (intersect)
			{
				inside = !inside;
			}
		}

		return inside;
	}
}
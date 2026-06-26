#pragma once
#include "geometry_defs.h"
#include "pointf.h"
#include "rectf.h"
namespace nani::canvas::basic
{
	class NANI_CANVAS_API PolygonF
	{
	public:
		PolygonF() = default;
		~PolygonF() = default;
		explicit PolygonF(const PointF& p1, const PointF& p2, const PointF& p3);
		explicit PolygonF(const RectF& rectangle);

	public:
		const RectF BoundingBox() const;
		bool IsContains(const PointF& pt) const;

	public:
		std::vector<PointF> vertices;
	};
}
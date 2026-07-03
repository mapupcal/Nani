#pragma once
#include "geometry_defs.h"
#include "pointf.h"
#include "polygonf.h"
namespace nani::canvas::basic
{
	class NANI_CANVAS_API TransformF
	{
	public:
		TransformF();
		~TransformF();

	public:
		static const TransformF Translation(scalar dx, scalar dy);
		static const TransformF Rotation(scalar angle);
		static const TransformF Scaling(scalar kx, scalar ky);
		static const TransformF Shearing(scalar khx, scalar khy);
		static const TransformF RotationAround(const PointF& anchor, scalar angle);
		static const TransformF ScalingAround(const PointF& anchor, scalar kx, scalar ky);
		static const TransformF ShearingAround(const PointF& anchor, scalar khx, scalar khy);

		static const TransformF Compose(std::initializer_list<TransformF> transforms);

	public:
		TransformF& Apply(const TransformF& transform);
		const TransformF Reversed() const;
		const TransformF Then(const TransformF& transform) const;

		const PointF ApplyTo(const PointF& pos) const;
		const PolygonF ApplyTo(const PolygonF& polygon) const;

	public:
		TransformF& operator=(const TransformF& other);

	public:
		scalar m11 = 1.0f, m12 = 0.0f, dx = 0.0f;
		scalar m21 = 0.0f, m22 = 1.0f, dy = 0.0f;
	};

	inline bool operator==(const TransformF& lhs, const TransformF& rhs)
	{
		return basic::IsScalarEqual(lhs.m11, rhs.m11) &&
			basic::IsScalarEqual(lhs.m12, rhs.m12) &&
			basic::IsScalarEqual(lhs.dx, rhs.dx) &&
			basic::IsScalarEqual(lhs.m21, rhs.m21) &&
			basic::IsScalarEqual(lhs.m22, rhs.m22) &&
			basic::IsScalarEqual(lhs.dy, rhs.dy);
	}

	inline bool operator!=(const TransformF& lhs, const TransformF& rhs)
	{
		return !(lhs == rhs);
	}
}
#pragma once
#include "geometry_defs.h"
namespace nani::canvas::basic
{
	struct NANI_CANVAS_API PointF
	{
	public:
		PointF();
		PointF(scalar _x, scalar _y);
		PointF(const PointF& other);
		~PointF();

	public:
		PointF& operator=(const PointF& other);
		PointF& operator+=(const PointF& other);
		PointF& operator-=(const PointF& other);
		PointF& operator*=(scalar k);
		PointF& operator/=(scalar k);

	public:
		scalar Dot(const PointF& other) const;
		scalar Cross(const PointF& other) const;
		scalar DistanceTo(const PointF& other) const;

	public:
		scalar x;
		scalar y;
	};

	inline PointF operator-(const PointF& p)
	{
		return PointF(-p.x, -p.y);
	}

	inline PointF operator+(const PointF& lhs, const PointF& rhs)
	{
		return PointF(lhs.x + rhs.x, lhs.y + rhs.y);
	}

	inline PointF operator-(const PointF& lhs, const PointF& rhs)
	{
		return PointF(lhs.x - rhs.x, lhs.y - rhs.y);
	}

	inline PointF operator*(const PointF& lhs, scalar rhs)
	{
		return PointF(lhs.x * rhs, lhs.y * rhs);
	}

	inline PointF operator*(scalar lhs, const PointF& rhs)
	{
		return rhs * lhs;
	}

	inline PointF operator/(const PointF& lhs, scalar rhs)
	{
		return lhs * (1.0f / rhs);
	}

	inline bool operator==(const PointF& lhs, const PointF& rhs)
	{
		return IsScalarEqual(lhs.x, rhs.x) && IsScalarEqual(lhs.y, rhs.y);
	}

	inline bool operator!=(const PointF& lhs, const PointF& rhs)
	{
		return !(lhs == rhs);
	}
}
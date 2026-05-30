#include "pointf.h"
namespace nani::canvas::basic
{
	PointF::PointF()
		: PointF(0.0f, 0.0f)
	{

	}

	PointF::PointF(scalar _x, scalar _y)
		: x(_x)
		, y(_y)
	{

	}

	PointF::PointF(const PointF& other)
		:PointF(other.x, other.y)
	{

	}

	PointF::~PointF()
	{

	}

	PointF& PointF::operator=(const PointF& other)
	{
		x = other.x;
		y = other.y;
		return *this;
	}

	PointF& PointF::operator+=(const PointF& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	PointF& PointF::operator-=(const PointF& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	PointF& PointF::operator*=(scalar k)
	{
		x *= k;
		y *= k;
		return *this;
	}

	PointF& PointF::operator/=(scalar k)
	{
		scalar _k = 1.0f / k;
		x *= _k;
		y *= _k;
		return *this;
	}

	scalar PointF::Dot(const PointF& other) const
	{
		return x * other.x + y * other.y;
	}

	scalar PointF::Cross(const PointF& other) const
	{
		return x * other.y - y * other.x;
	}

	scalar PointF::DistanceTo(const PointF& other) const
	{
		return std::sqrtf(std::pow(other.x - x, 2.0f) + std::pow(other.y - y, 2.0f));
	}
}
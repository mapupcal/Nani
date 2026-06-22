#pragma once
#include "geometry_defs.h"
#include "rectf.h"
namespace nani::canvas::basic
{
	struct NANI_CANVAS_API MarginsF
	{
	public:
		MarginsF();
		MarginsF(scalar left_, scalar top_, scalar right_, scalar bottom_);
		MarginsF(const MarginsF& other);
		~MarginsF();

	public:
		MarginsF& operator=(const MarginsF& other);
		MarginsF& operator+=(const MarginsF& other);
		MarginsF& operator-=(const MarginsF& other);
		MarginsF& operator*=(scalar k);
		MarginsF& operator/=(scalar k);

	public:
		scalar left = 0.0f;
		scalar top = 0.0f;
		scalar right = 0.0f;
		scalar bottom = 0.0f;
	};

	inline bool operator==(const MarginsF& lhs, const MarginsF& rhs)
	{
		return IsScalarEqual(lhs.left, rhs.left) &&
			IsScalarEqual(lhs.top, rhs.top) &&
			IsScalarEqual(lhs.right, rhs.right) &&
			IsScalarEqual(lhs.bottom, rhs.bottom);
	}

	inline bool operator!=(const MarginsF& lhs, const MarginsF& rhs)
	{
		return !(lhs == rhs);
	}

	inline const MarginsF operator-(const MarginsF& value)
	{
		return MarginsF(-value.left, -value.top, -value.right, -value.bottom);
	}

	inline const MarginsF operator+(const MarginsF& lhs, const MarginsF& rhs)
	{
		return MarginsF(lhs.left + rhs.left, lhs.top + rhs.top,
			lhs.right + rhs.right, lhs.bottom + rhs.bottom);
	}

	inline const MarginsF operator-(const MarginsF& lhs, const MarginsF& rhs)
	{
		return MarginsF(lhs.left - rhs.left, lhs.top - rhs.top,
			lhs.right - rhs.right, lhs.bottom - rhs.bottom);
	}

	inline const RectF operator+(const RectF& rect, const MarginsF& margins)
	{
		return RectF(rect.left - margins.left, rect.top - margins.top,
			rect.right + margins.right, rect.bottom + margins.bottom);
	}

	inline const RectF operator-(const RectF& rect, const MarginsF& margins)
	{
		return rect + ( - margins);
	}
}
#pragma once
#include "geometry_defs.h"
namespace nani::canvas::basic
{
	struct NANI_CANVAS_API SizeF
	{
	public:
		SizeF();
		SizeF(scalar _width, scalar _height);
		SizeF(const SizeF& other);
		~SizeF();

	public:
		SizeF& operator=(const SizeF& other);
		SizeF& operator+=(const SizeF& other);
		SizeF& operator-=(const SizeF& other);

	public:
		SizeF& Scale(scalar x, scalar y);
		SizeF Scaled(scalar x, scalar y) const;
		SizeF& ScaleX(scalar x);
		SizeF& ScaleY(scalar y);
		SizeF& Transpose();
		SizeF Transposed() const;
		bool IsValid() const;

	public:
		scalar width;
		scalar height;
	};

	inline SizeF operator+(const SizeF& lhs, const SizeF& rhs)
	{
		return SizeF(lhs.width + rhs.width, lhs.height + rhs.height);
	}

	inline SizeF operator-(const SizeF& lhs, const SizeF& rhs)
	{
		return SizeF(lhs.width - rhs.width, lhs.height - rhs.height);
	}

	inline bool operator==(const SizeF& lhs, const SizeF& rhs)
	{
		return IsScalarEqual(lhs.width, rhs.width) && IsScalarEqual(lhs.height, rhs.height);
	}

	inline bool operator!=(const SizeF& lhs, const SizeF& rhs)
	{
		return !(lhs == rhs);
	}
}
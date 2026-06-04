#pragma once
#include "basic_defs.h"
#include <cmath>

namespace nani::canvas::basic
{
	using scalar = float;

	namespace constant
	{
		template<typename T>
		constexpr T PI()
		{
			return static_cast<T>(3.141592653589793238462643383279502884L);
		}

		template<typename T>
		constexpr T E()
		{
			return static_cast<T>(2.718281828459045235360287471352662497L);
		}
	}

	inline bool IsScalarEqual(scalar lhs, scalar rhs)
	{
		return std::abs(rhs - lhs) <= 0.00001f;
	}

	constexpr scalar Degree2Radian(scalar degree)
	{
		return (degree / 180.0f) * constant::PI<scalar>();
	}

	constexpr scalar Radian2Degree(scalar radian)
	{
		return (radian / constant::PI<scalar>()) * 180.0f;
	}
}
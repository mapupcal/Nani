#include "marginsf.h"
namespace nani::canvas::basic
{
	MarginsF::MarginsF()
	{

	}

	MarginsF::MarginsF(scalar left_, scalar top_, scalar right_, scalar bottom_)
		: left(left_)
		, top(top_)
		, right(right_)
		, bottom(bottom_)
	{

	}

	MarginsF::MarginsF(const MarginsF& other)
		: left(other.left)
		, top(other.top)
		, right(other.right)
		, bottom(other.bottom)
	{

	}

	MarginsF::~MarginsF()
	{

	}

	MarginsF& MarginsF::operator=(const MarginsF& other)
	{
		left = other.left;
		top = other.top;
		right = other.right;
		bottom = other.bottom;
		return *this;
	}

	MarginsF& MarginsF::operator+=(const MarginsF& other)
	{
		left += other.left;
		top += other.top;
		right += other.right;
		bottom += other.bottom;
		return *this;
	}

	MarginsF& MarginsF::operator-=(const MarginsF& other)
	{
		left -= other.left;
		top -= other.top;
		right -= other.right;
		bottom -= other.bottom;
		return *this;
	}

	MarginsF& MarginsF::operator*=(scalar k)
	{
		left *= k;
		top *= k;
		right *= k;
		bottom *= k;
		return *this;
	}

	MarginsF& MarginsF::operator/=(scalar k)
	{
		return (*this *= (1.0f / k));
	}
}
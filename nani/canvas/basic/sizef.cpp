#include "sizef.h"
#include <utility>
namespace nani::canvas::basic
{
	SizeF::SizeF()
		: SizeF(0.0f, 0.0f)
	{

	}

	SizeF::SizeF(scalar _width, scalar _height)
		: width(_width)
		, height(_height)
	{

	}

	SizeF::SizeF(const SizeF& other)
		: SizeF(other.width, other.height)
	{

	}

	SizeF::~SizeF()
	{

	}

	SizeF& SizeF::operator=(const SizeF& other)
	{
		width = other.width;
		height = other.height;
		return *this;
	}

	SizeF& SizeF::operator+=(const SizeF& other)
	{
		width += other.width;
		height += other.height;
		return *this;
	}

	SizeF& SizeF::operator-=(const SizeF& other)
	{
		width -= other.width;
		height -= other.height;
		return *this;
	}

	SizeF& SizeF::Scale(scalar x, scalar y)
	{
		return ScaleX(x).ScaleY(y);
	}

	SizeF SizeF::Scaled(scalar x, scalar y) const
	{
		return SizeF(width * x, height * y);
	}

	SizeF& SizeF::ScaleX(scalar x)
	{
		width *= x;
		return *this;
	}

	SizeF& SizeF::ScaleY(scalar y)
	{
		height *= y;
		return *this;
	}

	SizeF& SizeF::Transpose()
	{
		std::swap(width, height);
		return *this;
	}

	SizeF SizeF::Transposed() const
	{
		return SizeF(height, width);
	}

	bool SizeF::IsValid() const
	{
		return width >= 0.0f && height >= 0.0f;
	}

}
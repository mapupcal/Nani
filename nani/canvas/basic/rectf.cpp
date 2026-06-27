#pragma once
#include "rectf.h"
#include <utility>
namespace nani::canvas::basic
{
	RectF::RectF()
		: RectF(0.0f, 0.0f, 0.0f, 0.0f)
	{

	}

	RectF::RectF(scalar _left, scalar _top, scalar _right, scalar _bottom)
		: left(_left)
		, top(_top)
		, right(_right)
		, bottom(_bottom)
	{

	}

	RectF::RectF(scalar x, scalar y, const SizeF& size)
		: RectF(x, y, x + size.width, y + size.height)
	{

	}

	RectF::RectF(const PointF& topLeft, const SizeF& size)
		: RectF(topLeft, size.width, size.height)
	{

	}

	RectF::RectF(const PointF& topLeft, scalar width, scalar height)
		: RectF(topLeft.x, topLeft.y, topLeft.x + width, topLeft.y + height)
	{

	}

	RectF::RectF(const PointF& pt1, const PointF& pt2)
		: RectF(std::min(pt1.x, pt2.x), std::min(pt1.y, pt2.y),
			std::max(pt1.x, pt2.x), std::max(pt1.y, pt2.y))
	{

	}

	RectF::RectF(const RectF& other)
		: RectF(other.left, other.top, other.right, other.bottom)
	{

	}

	RectF::RectF(const SizeF& size)
		: RectF(0.0f, 0.0f, size)
	{

	}

	RectF::~RectF()
	{

	}

	RectF& RectF::operator=(const RectF& other)
	{
		left = other.left;
		top = other.top;
		right = other.right;
		bottom = other.bottom;
		return *this;
	}

	RectF& RectF::operator&=(const RectF& other)
	{
		return Intersect(other);
	}

	RectF& RectF::operator|=(const RectF& other)
	{
		return Bound(other);
	}

	scalar RectF::X() const
	{
		return left;
	}

	scalar RectF::Y() const
	{
		return top;
	}

	PointF RectF::TopLeft() const
	{
		return PointF(left, top);
	}

	PointF RectF::BottomLeft() const
	{
		return PointF(left, bottom);
	}

	PointF RectF::TopRight() const
	{
		return PointF(right, top);
	}

	PointF RectF::BottomRight() const
	{
		return PointF(right, bottom);
	}

	PointF RectF::Center() const
	{
		return PointF((left + right) / 2.0f, (top + bottom) / 2.0f);
	}

	scalar RectF::Width() const
	{
		return right - left;
	}

	RectF& RectF::SetWidth(scalar width)
	{
		right = left + width;
		return *this;
	}

	scalar RectF::Height() const
	{
		return bottom - top;
	}

	RectF& RectF::SetHeight(scalar height)
	{
		bottom = top + height;
		return *this;
	}

	SizeF RectF::Size() const
	{
		return SizeF(Width(), Height());
	}

	RectF& RectF::SetSize(const SizeF& size)
	{
		return SetSize(size.width, size.height);
	}

	RectF& RectF::SetSize(scalar width, scalar height)
	{
		return SetWidth(width).SetHeight(height);
	}

	RectF& RectF::Transpose()
	{
		scalar width = Width();
		return SetWidth(Height()).SetHeight(width);
	}

	RectF RectF::Transposed() const
	{
		return RectF(TopLeft(), Size().Transposed());
	}

	bool RectF::IsValid() const
	{
		return Size().IsValid();
	}

	bool RectF::IsContains(const PointF& pt) const
	{
		return left <= pt.x && pt.x < right &&
			top <= pt.y && pt.y < bottom;
	}

	scalar RectF::Area() const
	{
		return Width() * Height();
	}

	RectF& RectF::Bound(const RectF& rc)
	{
		*this = Bounded(rc);
		return *this;
	}

	RectF RectF::Bounded(const RectF& rc) const
	{
		return RectF(
			std::min(left, rc.left), std::min(top, rc.top),
			std::max(right, rc.right), std::max(bottom, rc.bottom)
		);
	}

	RectF& RectF::Intersect(const RectF& rc)
	{
		*this = Intersected(rc);
		return *this;
	}

	RectF RectF::Intersected(const RectF& rc) const
	{
		return RectF(
			std::max(left, rc.left), std::max(top, rc.top),
			std::min(right, rc.right), std::min(bottom, rc.bottom)
		);
	}

	RectF& basic::RectF::MoveTo(const PointF& pos)
	{
		scalar width = Width();
		scalar height = Height();
		left = pos.x;
		top = pos.y;
		right = left + width;
		bottom = top + height;
		return *this;
	}
}
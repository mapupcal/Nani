#pragma once
#include "geometry_defs.h"
#include "pointf.h"
#include "sizef.h"
namespace nani::canvas::basic
{
	struct NANI_CANVAS_API RectF
	{
	public:
		RectF();
		RectF(scalar _left, scalar _top, scalar _right, scalar _bottom);
		RectF(const SizeF& size);
		RectF(scalar x, scalar y, const SizeF& size);
		RectF(const PointF& topLeft, const SizeF& size);
		RectF(const PointF& topLeft, scalar width, scalar height);
		RectF(const PointF& pt1, const PointF& pt2);
		RectF(const RectF& other);
		~RectF();

	public:
		RectF& operator=(const RectF& other);
		RectF& operator&=(const RectF& other);
		RectF& operator|=(const RectF& other);

	public:
		scalar X() const;
		scalar Y() const;

		PointF TopLeft() const;
		PointF BottomLeft() const;
		PointF TopRight() const;
		PointF BottomRight() const;
		PointF Center() const;

		scalar Width() const;
		RectF& SetWidth(scalar width);
		scalar Height() const;
		RectF& SetHeight(scalar height);
		SizeF Size() const;
		RectF& SetSize(const SizeF& size);
		RectF& SetSize(scalar width, scalar height);

		RectF& Transpose();
		RectF Transposed() const;
		bool IsValid() const;
		bool IsContains(const PointF& pt) const;
		scalar Area() const; 

		RectF& Bound(const RectF& rc);
		RectF Bounded(const RectF& rc) const;
		RectF& Intersect(const RectF& rc);
		RectF Intersected(const RectF& rc) const;

		RectF& MoveTo(const PointF& pos);

	public:
		scalar left;
		scalar top;
		scalar right;
		scalar bottom;
	};

	inline bool operator==(const RectF& lhs, const RectF& rhs)
	{
		return IsScalarEqual(lhs.left, rhs.left) &&
			IsScalarEqual(lhs.top, rhs.top) &&
			IsScalarEqual(lhs.right, rhs.right) &&
			IsScalarEqual(lhs.bottom, rhs.bottom);
	}

	inline bool operator!=(const RectF& lhs, const RectF& rhs)
	{
		return !(lhs == rhs);
	}

	inline RectF operator&(const RectF& lhs, const RectF& rhs)
	{
		return lhs.Intersected(rhs);
	}

	inline RectF operator|(const RectF& lhs, const RectF& rhs)
	{
		return lhs.Bounded(rhs);
	}
}
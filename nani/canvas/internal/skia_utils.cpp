#include "skia_utils.h"
#include <algorithm>

#ifdef NANI_OS_WIN
#include <ports/SkTypeface_win.h>
#endif // NANI_OS_WIN

using namespace nani::canvas::basic;

namespace nani::canvas::internal::skia_utils
{
	namespace
	{
		scalar InsetCorner(scalar radius, scalar insetX, scalar insetY)
		{
			const scalar inset = std::max(insetX, insetY);
			return std::max(0.0f, radius - inset);
		}
	}

	SkColor ToSkColor(const Color& color)
	{
		return SkColorSetARGB(color.a, color.r, color.g, color.b);
	}

	SkMatrix ToSkMatrix(const TransformF& transform)
	{
		return SkMatrix().setAll(
			transform.m11, transform.m12, transform.dx,
			transform.m21, transform.m22, transform.dy,
			0.0f, 0.0f, 1.0f
		);
	}

	SkRRect ToSkRRect(const basic::RectF& rect, const Radius& radius)
	{
		SkRect srect = SkRect::MakeLTRB(rect.left, rect.top, rect.right, rect.bottom);
		// Skia order: TL, TR, BR, BL
		SkVector radii[4] = {
			{radius.topLeft, radius.topLeft},
			{radius.topRight, radius.topRight},
			{radius.bottomRight, radius.bottomRight},
			{radius.bottomLeft, radius.bottomLeft}
		};
		SkRRect rrect;
		rrect.setRectRadii(srect, radii);
		return rrect;
	}

	Radius InsetBorderRadius(const Radius& radius, const MarginsF& inset)
	{
		Radius result;
		result.topLeft = InsetCorner(radius.topLeft, inset.left, inset.top);
		result.topRight = InsetCorner(radius.topRight, inset.right, inset.top);
		result.bottomRight = InsetCorner(radius.bottomRight, inset.right, inset.bottom);
		result.bottomLeft = InsetCorner(radius.bottomLeft, inset.left, inset.bottom);
		return result;
	}

	Radius OutsetBorderRadius(const Radius& radius, scalar outset)
	{
		Radius result;
		result.topLeft = radius.topLeft + outset;
		result.topRight = radius.topRight + outset;
		result.bottomRight = radius.bottomRight + outset;
		result.bottomLeft = radius.bottomLeft + outset;
		return result;
	}

	bool ContainsPoint(const RectF& rect, const Radius& radius, const PointF& point)
	{
		if (!rect.IsContains(point))
			return false;

		const bool hasRadius =
			radius.topLeft > 0.0f || radius.topRight > 0.0f ||
			radius.bottomLeft > 0.0f || radius.bottomRight > 0.0f;
		if (!hasRadius)
			return true;

		return SkPath::RRect(ToSkRRect(rect, radius)).contains(point.x, point.y);
	}

	sk_sp<SkFontMgr> CreateDefaultFontMgr()
	{
#ifdef NANI_OS_WIN
		return SkFontMgr_New_DirectWrite();
#endif
		NANI_ASSERT(false);
		NANI_MESSAGE("not impl.");
	}

	std::u8string GetFamilyName(sk_sp<SkTypeface> typeface)
	{
		SkString skName;
		typeface->getFamilyName(&skName);
		std::u8string family(skName.begin(), skName.end());
		return family;
	}
}

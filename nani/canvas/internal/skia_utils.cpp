#include "skia_utils.h"

#ifdef NANI_OS_WIN
#include <ports/SkTypeface_win.h>
#endif // NANI_OS_WIN

using namespace nani::canvas::basic;

namespace nani::canvas::internal::skia_utils
{
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

	SkRRect ToSkRRect(const basic::RectF& rect, const Raidus& radius)
	{
		SkRect srect = SkRect::MakeLTRB(rect.left, rect.top, rect.right, rect.bottom);
		SkVector radii[4] = {
			{radius.topLeft, radius.topLeft},
			{radius.topRight, radius.topRight},
			{radius.bottomLeft, radius.bottomLeft},
			{radius.bottomRight, radius.bottomRight}
		};
		SkRRect rrect;
		rrect.setRectRadii(srect, radii);
		return rrect;
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


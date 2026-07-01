#include "skia_utils.h"

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
}


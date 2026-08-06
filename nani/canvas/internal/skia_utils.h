#pragma once

#include "defs.h"
#include "computed_style.h"
#include "skia_defs.h"

#include "../basic/color.h"
#include "../basic/geometry_defs.h"
#include "../basic/marginsf.h"
#include "../basic/pointf.h"
#include "../basic/rectf.h"
#include "../basic/transformf.h"

namespace nani::canvas::internal::skia_utils
{
	SkColor ToSkColor(const basic::Color& color);
	SkMatrix ToSkMatrix(const basic::TransformF& transform);
	using Radius = ComputedStyle::VisualProperties::BorderRadius;
	SkRRect ToSkRRect(const basic::RectF& rect, const Radius& radius);

	// Inset / outset each corner radius by the bordering edges (CSS border-radius reduction).
	Radius InsetBorderRadius(const Radius& radius, const basic::MarginsF& inset);
	Radius OutsetBorderRadius(const Radius& radius, basic::scalar outset);

	// Point-in-rounded-rect, matching ToSkRRect circular corner semantics.
	bool ContainsPoint(
		const basic::RectF& rect,
		const Radius& radius,
		const basic::PointF& point);

	sk_sp<SkFontMgr> CreateDefaultFontMgr();

	SkString ToSkString(const std::u8string_view& text);
	std::u8string ToU8String(const SkString& text);
	std::u8string GetFamilyName(sk_sp<SkTypeface> typeface);

	// matchFamilyStyle(family, style), then Normal() if the styled face is missing.
	sk_sp<SkTypeface> MatchFamilyStyle(
		SkFontMgr* fontMgr,
		const std::u8string_view& family,
		const SkFontStyle& style);
}

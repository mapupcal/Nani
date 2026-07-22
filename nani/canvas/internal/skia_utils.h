#pragma once
#include "defs.h"
#include "../basic/geometry_defs.h"
#include "../basic/transformf.h"
#include "../basic/color.h"
#include "../basic/marginsf.h"
#include "computed_style.h"
#include <core/SkColor.h>
#include <core/SkMatrix.h>
#include <core/SkRRect.h>
#include <core/SkPath.h>
#include <core/SkFontMgr.h>
#include <core/SkTypeface.h>
#include <core/SkFont.h>

namespace nani::canvas::internal::skia_utils
{
	SkColor ToSkColor(const basic::Color& color);
	SkMatrix ToSkMatrix(const basic::TransformF& transform);
	using Raidus = ComputedStyle::VisualProperties::BorderRadius;
	SkRRect ToSkRRect(const basic::RectF& rect, const Raidus& radius);
	sk_sp<SkFontMgr> CreateDefaultFontMgr();
	std::u8string GetFamilyName(sk_sp<SkTypeface> typeface);
}

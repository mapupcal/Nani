#pragma once

#include "computed_style.h"

#include "../basic/color.h"
#include "../basic/rectf.h"
#include "../text/text_alignment.h"

namespace nani::canvas::internal::text_paint_utils
{
	inline basic::Color ResolveTextColor(const ComputedStyle* style)
	{
		if (!style)
			return basic::Colors::Black;

		const auto& visualProps = style->visualProps;
		if (visualProps.color.a != 0)
			return visualProps.color;

		if (visualProps.textDecoration.Color().a != 0)
			return visualProps.textDecoration.Color();

		return basic::Colors::Black;
	}

	inline float AlignedBlockTop(
		const basic::RectF& contentRect,
		float blockHeight,
		text::TextAlignment::Vertical align)
	{
		switch (align)
		{
		case text::TextAlignment::Vertical::Center:
			return contentRect.top + (contentRect.Height() - blockHeight) * 0.5f;
		case text::TextAlignment::Vertical::Bottom:
			return contentRect.bottom - blockHeight;
		case text::TextAlignment::Vertical::Top:
		default:
			return contentRect.top;
		}
	}
}

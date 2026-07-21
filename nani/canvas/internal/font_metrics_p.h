#pragma once
#include "../text/font_metrics.h"

namespace nani::canvas::internal
{
	using TextElideMode = text::TextElideMode;
	class FontMetricsPrivate
	{
	public:
		basic::single HorizontalAdvance(const std::u8string_view& text) const;
		basic::RectF BoundingRect(const std::u8string_view& text) const;
		basic::SizeF MeasureText(const std::u8string_view& text) const;
		const std::u8string ElidedText(
			const std::u8string_view& text,
			basic::single maxWidth,
			TextElideMode mode = TextElideMode::Right,
			const std::u8string_view& ellipsis = u8"…"
		) const;

	public:
		basic::single ascent = 0.0f;
		basic::single descent = 0.0f;
		basic::single leading = 0.0f;
		basic::single lineHeight = 0.0f;
	};
}
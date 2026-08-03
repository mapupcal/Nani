#pragma once
#include "text_defs.h"

namespace nani::canvas::text
{
	class NANI_CANVAS_API FontMetrics
	{
	public:
		explicit FontMetrics(const Font& font);
		~FontMetrics() = default;

	public:
		basic::single Ascent() const;
		basic::single Descent() const;
		basic::single Leading() const;
		basic::single LineHeight() const;
		basic::single XHeight() const;
		basic::single CapHeight() const;

		// Y offset from baseline (positive = below). Suitable for stroke-centered drawing.
		basic::single UnderlineOffset() const;
		basic::single UnderlineThickness() const;
		basic::single StrikeoutOffset() const;
		basic::single StrikeoutThickness() const;

		basic::single HorizontalAdvance(const std::u8string_view& text) const;
		basic::RectF BoundingRect(const std::u8string_view& text) const;
		basic::SizeF MeasureText(const std::u8string_view& text) const;
		const std::u8string ElidedText(
			const std::u8string_view& text,
			basic::single maxWidth,
			TextElideMode mode = TextElideMode::Right,
			const std::u8string_view& ellipsis = u8"…"
		) const;

	private:
		std::shared_ptr<SkFont> m_spSkFont;
	};
}

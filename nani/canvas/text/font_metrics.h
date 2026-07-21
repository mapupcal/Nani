#pragma once
#include "text_defs.h"

namespace nani::canvas::text
{
	enum class TextElideMode
	{
		None,
		Left,
		Middle,
		Right
	};

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
		std::shared_ptr<const internal::FontMetricsPrivate> m_private;
	};
}
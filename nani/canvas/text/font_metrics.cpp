#include "font_metrics.h"
#include "../internal/font_metrics_p.h"
#include "../internal/font_manager_p.h"
namespace nani::canvas::text
{
	FontMetrics::FontMetrics(const Font& font)
		: m_private(internal::FontManagerPrivate::Instance()->GetMetrics(font))
	{
		NANI_ASSERT(m_private);
	}

	basic::single FontMetrics::Ascent() const
	{
		return m_private->ascent;
	}

	basic::single FontMetrics::Descent() const
	{
		return m_private->descent;
	}

	basic::single FontMetrics::Leading() const
	{
		return m_private->leading;
	}

	basic::single FontMetrics::LineHeight() const
	{
		return m_private->lineHeight;
	}

	basic::single FontMetrics::HorizontalAdvance(const std::u8string_view& text) const
	{
		return m_private->HorizontalAdvance(text);
	}

	basic::RectF FontMetrics::BoundingRect(const std::u8string_view& text) const
	{
		return m_private->BoundingRect(text);
	}

	basic::SizeF FontMetrics::MeasureText(const std::u8string_view& text) const
	{
		return m_private->MeasureText(text);
	}

	const std::u8string FontMetrics::ElidedText(
		const std::u8string_view& text,
		basic::single maxWidth,
		TextElideMode mode,
		const std::u8string_view& ellipsis
	) const
	{
		return m_private->ElidedText(text, maxWidth, mode, ellipsis);
	}
}
#include "font_metrics_p.h"
namespace nani::canvas::internal
{
	basic::single FontMetricsPrivate::HorizontalAdvance(const std::u8string_view& text) const
	{
		return 0.0f;
	}

	basic::RectF FontMetricsPrivate::BoundingRect(const std::u8string_view& text) const
	{
		return basic::RectF();
	}

	basic::SizeF FontMetricsPrivate::MeasureText(const std::u8string_view& text) const
	{
		return basic::SizeF();
	}

	const std::u8string FontMetricsPrivate::ElidedText(
		const std::u8string_view& text,
		basic::single maxWidth,
		TextElideMode mode,
		const std::u8string_view& ellipsis
	) const
	{
		return std::u8string();
	}
}
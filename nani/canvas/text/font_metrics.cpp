#include "font_metrics.h"
#include "utf8.h"

#include <functional>
#include <span>

#include "internal/font_manager_p.h"
#include "internal/font_resolver.h"
#include "internal/skia_defs.h"

namespace nani::canvas::text
{
	namespace
	{
		std::span<const std::u8string> AsSpan(const std::vector<std::u8string>& families)
		{
			return std::span<const std::u8string>(families.data(), families.size());
		}
	}

	FontMetrics::FontMetrics(const Font& font)
		: m_font(font)
		, m_spSkFont(internal::FontManagerPrivate::Instance()->CreateSkFont(font))
	{
		NANI_ASSERT(m_spSkFont);
	}

	basic::single FontMetrics::Ascent() const
	{
		SkFontMetrics metrics;
		m_spSkFont->getMetrics(&metrics);
		return -metrics.fAscent;
	}

	basic::single FontMetrics::Descent() const
	{
		SkFontMetrics metrics;
		m_spSkFont->getMetrics(&metrics);
		return metrics.fDescent;
	}

	basic::single FontMetrics::Leading() const
	{
		SkFontMetrics metrics;
		m_spSkFont->getMetrics(&metrics);
		return metrics.fLeading;
	}

	basic::single FontMetrics::LineHeight() const
	{
		SkFontMetrics metrics;
		m_spSkFont->getMetrics(&metrics);
		return -metrics.fAscent + metrics.fDescent + metrics.fLeading;
	}

	basic::single FontMetrics::XHeight() const
	{
		SkFontMetrics metrics;
		m_spSkFont->getMetrics(&metrics);
		if (metrics.fXHeight < 0.0f)
			return -metrics.fXHeight;
		if (metrics.fXHeight > 0.0f)
			return metrics.fXHeight;
		return Ascent() * 0.5f;
	}

	basic::single FontMetrics::CapHeight() const
	{
		SkFontMetrics metrics;
		m_spSkFont->getMetrics(&metrics);
		if (metrics.fCapHeight < 0.0f)
			return -metrics.fCapHeight;
		if (metrics.fCapHeight > 0.0f)
			return metrics.fCapHeight;
		return Ascent() * 0.7f;
	}

	basic::single FontMetrics::UnderlineOffset() const
	{
		SkFontMetrics metrics;
		m_spSkFont->getMetrics(&metrics);
		SkScalar position = 0.0f;
		SkScalar thickness = 0.0f;
		if (metrics.hasUnderlinePosition(&position))
		{
			if (metrics.hasUnderlineThickness(&thickness) && thickness > 0.0f)
				return position + thickness * 0.5f;
			return position + UnderlineThickness() * 0.5f;
		}
		return std::max(UnderlineThickness(), Descent() * 0.25f);
	}

	basic::single FontMetrics::UnderlineThickness() const
	{
		SkFontMetrics metrics;
		m_spSkFont->getMetrics(&metrics);
		SkScalar thickness = 0.0f;
		if (metrics.hasUnderlineThickness(&thickness) && thickness > 0.0f)
			return thickness;
		return std::max(1.0f, m_spSkFont->getSize() / 14.0f);
	}

	basic::single FontMetrics::StrikeoutOffset() const
	{
		SkFontMetrics metrics;
		m_spSkFont->getMetrics(&metrics);
		SkScalar position = 0.0f;
		SkScalar thickness = 0.0f;
		if (metrics.hasStrikeoutPosition(&position))
		{
			// Skia: position is baseline -> bottom of stroke (typically negative).
			const float thick = (metrics.hasStrikeoutThickness(&thickness) && thickness > 0.0f)
				? thickness
				: StrikeoutThickness();
			return position - thick * 0.5f;
		}

		const float xHeight = XHeight();
		return -xHeight * 0.5f;
	}

	basic::single FontMetrics::StrikeoutThickness() const
	{
		SkFontMetrics metrics;
		m_spSkFont->getMetrics(&metrics);
		SkScalar thickness = 0.0f;
		if (metrics.hasStrikeoutThickness(&thickness) && thickness > 0.0f)
			return thickness;
		return std::max(1.0f, m_spSkFont->getSize() / 14.0f);
	}

	basic::single FontMetrics::HorizontalAdvance(const std::u8string_view& text) const
	{
		if (text.empty() || !m_spSkFont)
			return 0.0f;

		auto* mgr = internal::FontManagerPrivate::Instance();
		return internal::font_resolver::Measure(
			*m_spSkFont,
			mgr->FontMgr(),
			AsSpan(m_font.Families()),
			AsSpan(mgr->FallbackFamilies()),
			text);
	}

	basic::RectF FontMetrics::BoundingRect(const std::u8string_view& text) const
	{
		if (text.empty() || !m_spSkFont)
			return basic::RectF(0, 0, 0, 0);

		const float width = HorizontalAdvance(text);
		return basic::RectF(0.0f, -Ascent(), width, Descent());
	}

	basic::SizeF FontMetrics::MeasureText(const std::u8string_view& text) const
	{
		return BoundingRect(text).Size();
	}

	void FontMetrics::DrawText(
		SkCanvas* canvas,
		const std::u8string_view& text,
		basic::single x,
		basic::single y,
		const SkPaint& paint) const
	{
		if (!canvas || text.empty() || !m_spSkFont)
			return;

		SkFont drawFont = *m_spSkFont;
		drawFont.setEdging(SkFont::Edging::kAntiAlias);
		drawFont.setSubpixel(true);

		auto* mgr = internal::FontManagerPrivate::Instance();
		internal::font_resolver::Draw(
			canvas,
			drawFont,
			mgr->FontMgr(),
			AsSpan(m_font.Families()),
			AsSpan(mgr->FallbackFamilies()),
			text,
			x,
			y,
			paint);
	}

	namespace
	{
		using MeasureFn = std::function<float(const char*, size_t)>;

		std::u8string ElideTextEnd(
			const MeasureFn& measure,
			const std::u8string_view& text,
			float availableWidth,
			const std::u8string_view& ellipsis)
		{
			const char* utf8Data = reinterpret_cast<const char*>(text.data());
			size_t byteLength = text.size();

			if (byteLength == 0 || availableWidth <= 0)
				return std::u8string(ellipsis);

			size_t lo = 0, hi = byteLength;
			size_t bestFit = 0;

			while (lo <= hi && lo < byteLength)
			{
				size_t mid = lo + (hi - lo) / 2;
				size_t charStart = utf8::AlignBoundary(utf8Data, mid, byteLength);
				size_t prefixLen = (charStart < byteLength)
					? utf8::NextIndex(utf8Data, charStart, byteLength)
					: byteLength;

				const float width = measure(utf8Data, prefixLen);
				if (width <= availableWidth)
				{
					bestFit = prefixLen;
					lo = prefixLen;
				}
				else if (charStart == 0)
				{
					break;
				}
				else
				{
					hi = charStart - 1;
				}
			}

			std::u8string result;
			if (bestFit > 0)
				result.append(reinterpret_cast<const char8_t*>(utf8Data), bestFit);
			result.append(ellipsis);
			return result;
		}

		std::u8string ElideTextStart(
			const MeasureFn& measure,
			const std::u8string_view& text,
			float availableWidth,
			const std::u8string_view& ellipsis)
		{
			const char* utf8Data = reinterpret_cast<const char*>(text.data());
			size_t byteLength = text.size();

			if (byteLength == 0 || availableWidth <= 0)
				return std::u8string(ellipsis);

			size_t lo = 0, hi = byteLength;
			size_t bestStart = byteLength;

			while (lo <= hi && lo < byteLength)
			{
				size_t mid = lo + (hi - lo) / 2;
				size_t boundary = utf8::AlignBoundary(utf8Data, mid, byteLength);

				if (boundary >= byteLength)
				{
					if (byteLength == 0)
						break;
					hi = byteLength - 1;
					continue;
				}

				const float width = measure(utf8Data + boundary, byteLength - boundary);
				if (width <= availableWidth)
				{
					bestStart = boundary;
					if (boundary == 0)
						break;
					hi = boundary - 1;
				}
				else
				{
					lo = utf8::NextIndex(utf8Data, boundary, byteLength);
				}
			}

			std::u8string result;
			result.append(ellipsis);
			if (bestStart < byteLength)
			{
				result.append(reinterpret_cast<const char8_t*>(utf8Data + bestStart),
					byteLength - bestStart);
			}
			return result;
		}

		std::u8string ElideTextMiddle(
			const MeasureFn& measure,
			const std::u8string_view& text,
			float availableWidth,
			const std::u8string_view& ellipsis)
		{
			const char* utf8Data = reinterpret_cast<const char*>(text.data());
			size_t byteLength = text.size();

			if (byteLength == 0 || availableWidth <= 0)
				return std::u8string(ellipsis);

			if (byteLength <= 6)
				return ElideTextEnd(measure, text, availableWidth, ellipsis);

			size_t halfPos = byteLength / 2;
			size_t splitPoint = utf8::AlignBoundary(utf8Data, halfPos, byteLength);
			if (splitPoint == 0 || splitPoint >= byteLength)
				return ElideTextEnd(measure, text, availableWidth, ellipsis);

			size_t leftBytes = splitPoint;
			size_t rightBytes = byteLength - splitPoint;
			float leftWidth = measure(utf8Data, leftBytes);
			float rightWidth = measure(utf8Data + splitPoint, rightBytes);

			const int maxIterations = 100;
			int iterations = 0;

			while (iterations < maxIterations &&
				(leftBytes > 0 || rightBytes > 0) &&
				(leftWidth + rightWidth > availableWidth))
			{
				iterations++;

				if (leftWidth >= rightWidth && leftBytes > 0)
				{
					size_t newLeft = utf8::PrevIndex(utf8Data, leftBytes);
					if (newLeft < leftBytes && newLeft > 0)
					{
						leftBytes = newLeft;
						leftWidth = measure(utf8Data, leftBytes);
					}
					else
					{
						leftBytes = 0;
						leftWidth = 0;
					}
				}
				else if (rightBytes > 0)
				{
					size_t rightStart = byteLength - rightBytes;
					size_t newRightStart = utf8::NextIndex(utf8Data, rightStart, byteLength);
					if (newRightStart > rightStart && newRightStart < byteLength)
					{
						rightBytes = byteLength - newRightStart;
						rightWidth = measure(utf8Data + newRightStart, rightBytes);
					}
					else
					{
						rightBytes = 0;
						rightWidth = 0;
					}
				}
			}

			if (leftWidth + rightWidth > availableWidth)
				return ElideTextEnd(measure, text, availableWidth, ellipsis);

			std::u8string result;
			if (leftBytes > 0)
				result.append(reinterpret_cast<const char8_t*>(utf8Data), leftBytes);
			result.append(ellipsis);
			if (rightBytes > 0)
			{
				result.append(reinterpret_cast<const char8_t*>(utf8Data + byteLength - rightBytes),
					rightBytes);
			}
			return result;
		}

		bool IsAsciiSpace(char ch)
		{
			return ch == ' ' || ch == '\t';
		}

		std::vector<std::u8string> WrapHardLine(
			const MeasureFn& measure,
			const std::u8string_view& hardLine,
			basic::single maxWidth)
		{
			std::vector<std::u8string> lines;
			if (hardLine.empty())
			{
				lines.emplace_back();
				return lines;
			}

			const char* data = reinterpret_cast<const char*>(hardLine.data());
			const size_t length = hardLine.size();
			const float fullWidth = measure(data, length);
			if (fullWidth <= maxWidth)
			{
				lines.emplace_back(hardLine);
				return lines;
			}

			size_t lineStart = 0;
			while (lineStart < length)
			{
				size_t pos = lineStart;
				size_t lastFit = lineStart;
				size_t lastBreak = std::string_view::npos;

				while (pos < length)
				{
					const size_t next = utf8::NextIndex(data, pos, length);
					const float width = measure(data + lineStart, next - lineStart);
					if (width > maxWidth)
						break;

					lastFit = next;
					if (IsAsciiSpace(data[pos]))
						lastBreak = pos;
					pos = next;
				}

				size_t breakAt = lastFit;
				size_t nextStart = lastFit;
				if (lastFit == lineStart)
				{
					breakAt = utf8::NextIndex(data, lineStart, length);
					nextStart = breakAt;
				}
				else if (pos < length && lastBreak != std::string_view::npos && lastBreak > lineStart)
				{
					breakAt = lastBreak;
					nextStart = lastBreak;
					while (nextStart < length && IsAsciiSpace(data[nextStart]))
						++nextStart;
				}

				lines.emplace_back(
					reinterpret_cast<const char8_t*>(data + lineStart),
					breakAt - lineStart);

				if (nextStart <= lineStart)
					nextStart = utf8::NextIndex(data, lineStart, length);
				lineStart = nextStart;
			}

			if (lines.empty())
				lines.emplace_back();
			return lines;
		}
	}

	const std::u8string FontMetrics::ElidedText(
		const std::u8string_view& text,
		basic::single maxWidth,
		TextElideMode mode,
		const std::u8string_view& ellipsis
	) const
	{
		if (text.empty() || maxWidth <= 0)
			return std::u8string();

		if (mode == TextElideMode::None)
			return std::u8string(text);

		const float fullWidth = HorizontalAdvance(text);
		if (fullWidth <= maxWidth)
			return std::u8string(text);

		const float ellipsisWidth = HorizontalAdvance(ellipsis);
		if (ellipsisWidth >= maxWidth)
			return ellipsis.empty() ? std::u8string() : std::u8string(ellipsis);

		const float availableWidth = maxWidth - ellipsisWidth;
		const auto measure = [this](const char* data, size_t length) -> float
		{
			return HorizontalAdvance(std::u8string_view(
				reinterpret_cast<const char8_t*>(data),
				length));
		};

		switch (mode)
		{
		case TextElideMode::Right:
			return ElideTextEnd(measure, text, availableWidth, ellipsis);
		case TextElideMode::Middle:
			return ElideTextMiddle(measure, text, availableWidth, ellipsis);
		case TextElideMode::Left:
			return ElideTextStart(measure, text, availableWidth, ellipsis);
		default:
			return std::u8string(text);
		}
	}

	std::vector<std::u8string> FontMetrics::LayoutLines(
		const std::u8string_view& text,
		basic::single maxWidth,
		bool wrap) const
	{
		std::vector<std::u8string> lines;
		if (text.empty())
			return lines;

		const auto measure = [this](const char* data, size_t length) -> float
		{
			return HorizontalAdvance(std::u8string_view(
				reinterpret_cast<const char8_t*>(data),
				length));
		};

		const bool softWrap = wrap && maxWidth > 0.0f;
		size_t start = 0;
		for (size_t i = 0; i <= text.size(); ++i)
		{
			const bool atEnd = i == text.size();
			const bool atBreak = !atEnd && text[i] == u8'\n';
			if (!atEnd && !atBreak)
				continue;

			size_t lineEnd = i;
			if (atBreak && lineEnd > start && text[lineEnd - 1] == u8'\r')
				--lineEnd;

			const std::u8string_view hardLine = text.substr(start, lineEnd - start);
			if (softWrap)
			{
				auto wrapped = WrapHardLine(measure, hardLine, maxWidth);
				lines.insert(lines.end(), wrapped.begin(), wrapped.end());
			}
			else
			{
				lines.emplace_back(hardLine);
			}

			if (atEnd)
				break;
			start = i + 1;
		}

		return lines;
	}
}

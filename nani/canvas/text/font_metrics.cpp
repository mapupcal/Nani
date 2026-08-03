#include "font_metrics.h"
#include "internal/font_manager_p.h"
#include <core/SkFont.h>
#include <core/SkFontMetrics.h>
#include <core/SkFontTypes.h>
#include <algorithm>

namespace nani::canvas::text
{
	FontMetrics::FontMetrics(const Font& font)
		: m_spSkFont(internal::FontManagerPrivate::Instance()->CreateSkFont(font))
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
		if (text.empty())
			return 0.0f;

		const char* utf8Data = reinterpret_cast<const char*>(text.data());
		size_t byteLength = text.size();

		return m_spSkFont->measureText(utf8Data, byteLength, SkTextEncoding::kUTF8);
	}

	basic::RectF FontMetrics::BoundingRect(const std::u8string_view& text) const
	{
		if (text.empty())
			return basic::RectF(0, 0, 0, 0);

		const char* utf8Data = reinterpret_cast<const char*>(text.data());
		size_t byteLength = text.size();

		SkRect bounds;
		m_spSkFont->measureText(utf8Data, byteLength, SkTextEncoding::kUTF8, &bounds);

		return basic::RectF(bounds.left(), bounds.top(), bounds.right(), bounds.bottom());
	}

	basic::SizeF FontMetrics::MeasureText(const std::u8string_view& text) const
	{
		return BoundingRect(text).Size();
	}

	namespace
	{
		// Helper: Check if a byte is a UTF-8 continuation byte
		inline bool IsUtf8ContinuationByte(char c)
		{
			return (c & 0xC0) == 0x80;
		}

		// Helper: Find a valid UTF-8 character boundary
		// Returns the nearest valid boundary at or before the given position
		size_t FindUtf8Boundary(const char* data, size_t pos, size_t length)
		{
			if (pos >= length)
			{
				return length;
			}

			// Move forward to find a valid boundary
			size_t boundary = pos;
			while (boundary < length && IsUtf8ContinuationByte(data[boundary]))
			{
				boundary++;
			}

			// If we went past the end or couldn't find a boundary,
			// try moving backward instead
			if (boundary >= length)
			{
				boundary = pos;
				while (boundary > 0 && IsUtf8ContinuationByte(data[boundary]))
				{
					boundary--;
				}
			}

			return boundary;
		}

		// Helper: Find the start of the previous character
		size_t PrevCharStart(const char* data, size_t pos)
		{
			if (pos == 0)
			{
				return 0;
			}

			pos--;
			while (pos > 0 && IsUtf8ContinuationByte(data[pos]))
			{
				pos--;
			}
			return pos;
		}

		// Helper: Find the start of the next character
		size_t NextCharStart(const char* data, size_t pos, size_t length)
		{
			if (pos >= length)
			{
				return length;
			}

			pos++;
			while (pos < length && IsUtf8ContinuationByte(data[pos]))
			{
				pos++;
			}
			return pos;
		}

		// Elide text at the end (Right)
		std::u8string ElideTextEnd(
			const std::shared_ptr<SkFont>& skFont,
			const std::u8string_view& text,
			SkScalar availableWidth,
			const std::u8string_view& ellipsis)
		{
			const char* utf8Data = reinterpret_cast<const char*>(text.data());
			size_t byteLength = text.size();

			if (byteLength == 0 || availableWidth <= 0)
			{
				return std::u8string(ellipsis);
			}

			// Binary search for the maximum bytes that fit
			size_t lo = 0, hi = byteLength;
			size_t bestFit = 0;

			while (lo <= hi && lo < byteLength)
			{
				size_t mid = lo + (hi - lo) / 2;
				size_t boundary = FindUtf8Boundary(utf8Data, mid, byteLength);

				if (boundary == 0)
				{
					break;
				}

				SkScalar width = skFont->measureText(utf8Data, boundary, SkTextEncoding::kUTF8);

				if (width <= availableWidth)
				{
					bestFit = boundary;
					lo = boundary + 1;
				}
				else
				{
					hi = boundary - 1;
				}
			}

			std::u8string result;
			if (bestFit > 0)
			{
				result.append(reinterpret_cast<const char8_t*>(utf8Data), bestFit);
			}
			result.append(ellipsis);
			return result;
		}

		// Elide text at the beginning (Left)
		std::u8string ElideTextStart(
			const std::shared_ptr<SkFont>& skFont,
			const std::u8string_view& text,
			SkScalar availableWidth,
			const std::u8string_view& ellipsis)
		{
			const char* utf8Data = reinterpret_cast<const char*>(text.data());
			size_t byteLength = text.size();

			if (byteLength == 0 || availableWidth <= 0)
			{
				return std::u8string(ellipsis);
			}

			// Measure ellipsis
			const char* ellipsisData = reinterpret_cast<const char*>(ellipsis.data());
			SkScalar ellipsisWidth = skFont->measureText(ellipsisData, ellipsis.size(),
				SkTextEncoding::kUTF8);

			if (ellipsisWidth >= availableWidth)
			{
				return std::u8string(ellipsis);
			}

			// Binary search for the best start position
			size_t lo = 0, hi = byteLength;
			size_t bestStart = byteLength;

			while (lo <= hi && lo < byteLength)
			{
				size_t mid = lo + (hi - lo) / 2;
				size_t boundary = FindUtf8Boundary(utf8Data, mid, byteLength);

				if (boundary >= byteLength)
				{
					hi = boundary - 1;
					continue;
				}

				SkScalar width = skFont->measureText(utf8Data + boundary,
					byteLength - boundary,
					SkTextEncoding::kUTF8);

				if (width <= availableWidth)
				{
					bestStart = boundary;
					hi = boundary - 1;  // Try to include more text
				}
				else
				{
					lo = boundary + 1;  // Include less text
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

		// Elide text in the middle (Middle)
		std::u8string ElideTextMiddle(
			const std::shared_ptr<SkFont>& skFont,
			const std::u8string_view& text,
			SkScalar availableWidth,
			const std::u8string_view& ellipsis)
		{
			const char* utf8Data = reinterpret_cast<const char*>(text.data());
			size_t byteLength = text.size();

			if (byteLength == 0 || availableWidth <= 0)
			{
				return std::u8string(ellipsis);
			}

			// If text is too short, fallback to end elision
			if (byteLength <= 6)
			{
				return ElideTextEnd(skFont, text, availableWidth, ellipsis);
			}

			// Measure ellipsis
			const char* ellipsisData = reinterpret_cast<const char*>(ellipsis.data());
			SkScalar ellipsisWidth = skFont->measureText(ellipsisData, ellipsis.size(),
				SkTextEncoding::kUTF8);

			if (ellipsisWidth >= availableWidth)
			{
				return std::u8string(ellipsis);
			}

			// Start with half the text
			size_t halfPos = byteLength / 2;
			size_t splitPoint = FindUtf8Boundary(utf8Data, halfPos, byteLength);

			if (splitPoint == 0 || splitPoint >= byteLength)
			{
				return ElideTextEnd(skFont, text, availableWidth, ellipsis);
			}

			// Try to balance left and right
			size_t leftBytes = splitPoint;
			size_t rightBytes = byteLength - splitPoint;
			SkScalar leftWidth = skFont->measureText(utf8Data, leftBytes, SkTextEncoding::kUTF8);
			SkScalar rightWidth = skFont->measureText(utf8Data + splitPoint, rightBytes,
				SkTextEncoding::kUTF8);

			// Adjust to fit within available width
			const int maxIterations = 100;
			int iterations = 0;

			while (iterations < maxIterations &&
				(leftBytes > 0 || rightBytes > 0) &&
				(leftWidth + rightWidth > availableWidth))
			{
				iterations++;

				// Remove from the longer side
				if (leftWidth >= rightWidth && leftBytes > 0)
				{
					// Remove last character from left
					size_t newLeft = PrevCharStart(utf8Data, leftBytes);
					if (newLeft < leftBytes && newLeft > 0)
					{
						leftBytes = newLeft;
						leftWidth = skFont->measureText(utf8Data, leftBytes, SkTextEncoding::kUTF8);
					}
					else
					{
						leftBytes = 0;
						leftWidth = 0;
					}
				}
				else if (rightBytes > 0)
				{
					// Remove first character from right
					size_t rightStart = byteLength - rightBytes;
					size_t newRightStart = NextCharStart(utf8Data, rightStart, byteLength);
					if (newRightStart > rightStart && newRightStart < byteLength)
					{
						rightBytes = byteLength - newRightStart;
						rightWidth = skFont->measureText(utf8Data + newRightStart,
							rightBytes, SkTextEncoding::kUTF8);
					}
					else
					{
						rightBytes = 0;
						rightWidth = 0;
					}
				}
			}

			// Fallback to end elision if still too wide
			if (leftWidth + rightWidth > availableWidth)
			{
				return ElideTextEnd(skFont, text, availableWidth, ellipsis);
			}

			// Construct result
			std::u8string result;
			if (leftBytes > 0)
			{
				result.append(reinterpret_cast<const char8_t*>(utf8Data), leftBytes);
			}
			result.append(ellipsis);
			if (rightBytes > 0)
			{
				result.append(reinterpret_cast<const char8_t*>(utf8Data + byteLength - rightBytes),
					rightBytes);
			}
			return result;
		}

	}

	const std::u8string FontMetrics::ElidedText(
		const std::u8string_view& text,
		basic::single maxWidth,
		TextElideMode mode,
		const std::u8string_view& ellipsis
	) const
	{
		// Empty text or invalid width
		if (text.empty() || maxWidth <= 0)
		{
			return std::u8string();
		}

		// No elision
		if (mode == TextElideMode::None)
		{
			return std::u8string(text);
		}

		// Measure full text
		const char* utf8Data = reinterpret_cast<const char*>(text.data());
		size_t byteLength = text.size();
		SkScalar fullWidth = m_spSkFont->measureText(utf8Data, byteLength,
			SkTextEncoding::kUTF8);

		// Text already fits
		if (fullWidth <= maxWidth)
		{
			return std::u8string(text);
		}

		// Measure ellipsis
		const char* ellipsisData = reinterpret_cast<const char*>(ellipsis.data());
		SkScalar ellipsisWidth = m_spSkFont->measureText(ellipsisData, ellipsis.size(),
			SkTextEncoding::kUTF8);

		// Ellipsis itself is too wide
		if (ellipsisWidth >= maxWidth)
		{
			return ellipsis.empty() ? std::u8string() : std::u8string(ellipsis);
		}

		// Available width for text
		SkScalar availableWidth = maxWidth - ellipsisWidth;

		// Dispatch based on mode
		switch (mode)
		{
		case TextElideMode::Right:
			return ElideTextEnd(m_spSkFont, text, availableWidth, ellipsis);
		case TextElideMode::Middle:
			return ElideTextMiddle(m_spSkFont, text, availableWidth, ellipsis);
		case TextElideMode::Left:
			return ElideTextStart(m_spSkFont, text, availableWidth, ellipsis);
		default:
			return std::u8string(text);
		}
	}
}

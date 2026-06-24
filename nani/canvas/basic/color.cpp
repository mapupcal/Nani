#include "color.h"
namespace
{
	using namespace nani::canvas::basic;

	dword ColorsEnumToRgba(Colors color)
	{
		switch (color)
		{
		case Colors::Black:       return 0x000000FF; // R=0, G=0, B=0, A=255
		case Colors::White:       return 0xFFFFFFFF; // R=255, G=255, B=255, A=255
		case Colors::Red:         return 0xFF0000FF; // R=255, G=0, B=0, A=255
		case Colors::Blue:        return 0x0000FFFF; // R=0, G=0, B=255, A=255
		case Colors::Green:       return 0x00FF00FF; // R=0, G=255, B=0, A=255
		case Colors::Gray:        return 0x808080FF; // R=128, G=128, B=128, A=255
		case Colors::Yellow:      return 0xFFFF00FF; // R=255, G=255, B=0, A=255
		case Colors::Cyan:        return 0x00FFFFFF; // R=0, G=255, B=255, A=255
		case Colors::Magenta:     return 0xFF00FFFF; // R=255, G=0, B=255, A=255
		case Colors::Transparent: return 0x00000000; // R=0, G=0, B=0, A=0
		default:                  return 0x000000FF; // fallback to black
		}
	}

	dword HexStringToRgba(const std::string_view& hex)
	{
		// Supported formats:
		// - "RRGGBB" (6 hex digits) -> Alpha = 0xFF
		// - "RRGGBBAA" (8 hex digits) -> Alpha included
		// - "#RRGGBB" or "#RRGGBBAA" with leading #
		std::string cleaned;
		cleaned.reserve(8);

		// Remove leading '#' if present
		size_t start = 0;
		if (!hex.empty() && hex[0] == '#')
		{
			start = 1;
		}

		// Extract only hex characters
		for (size_t i = start; i < hex.size(); ++i) {
			char c = hex[i];
			if (std::isxdigit(static_cast<unsigned char>(c)))
				cleaned += c;
			else
				break; // Stop at the first non-hex character
		}

		// Validate length
		if (cleaned.length() != 6 && cleaned.length() != 8)
		{
			NANI_MESSAGE("Invalid hex color format. Expected 6 or 8 hex digits.");
			return 0x000000FF;
		}

		// Convert hex string to integer
		dword value = 0;
		for (char c : cleaned)
		{
			value <<= 4;
			if (c >= '0' && c <= '9')
				value |= (c - '0');
			else if (c >= 'a' && c <= 'f')
				value |= (c - 'a' + 10);
			else if (c >= 'A' && c <= 'F')
				value |= (c - 'A' + 10);
			else
				NANI_ASSERT(false); // Should not happen due to previous validation
		}

		// If no alpha provided, set alpha to 0xFF (fully opaque)
		if (cleaned.length() == 6)
			value = (value << 8) | 0xFF;

		// Return as RGBA (assuming format is RRGGBBAA or RRGGBB + AA)
		// Convert from RRGGBBAA to AARRGGBB if needed by your system
		return value;
	}

	std::string DoubleWordToHexString(dword value)
	{
		const char hex_chars[] = "0123456789ABCDEF";
		std::string result;
		result.reserve(9); // '#' + 8 hex digits

		result += '#';
		for (int i = 24; i >= 0; i -= 8)
		{
			unsigned char byte = (value >> i) & 0xFF;
			result += hex_chars[byte >> 4];
			result += hex_chars[byte & 0x0F];
		}

		return result;
	}
}

namespace nani::canvas::basic
{
	Color::Color(dword rgba)
		: r((rgba >> 24) & 0xFF)
		, g((rgba >> 16) & 0xFF)
		, b((rgba >> 8) & 0xFF)
		, a((rgba) & 0xFF)
	{

	}

	Color::Color(Colors color)
		: Color(ColorsEnumToRgba(color))
	{

	}

	Color::Color(const std::string_view & hex)
		: Color(HexStringToRgba(hex))
	{

	}

	Color::Color(uint8 r_, uint8 g_, uint8 b_, uint8 a_)
		: r(r_)
		, g(g_)
		, b(b_)
		, a(a_)
	{

	}

	dword Color::Rgba() const
	{
		return (static_cast<dword>(r) << 24)
			| (static_cast<dword>(g) << 16)
			| (static_cast<dword>(b) << 8)
			| (static_cast<dword>(a));
	}

	dword Color::Argb() const
	{
		return (static_cast<dword>(a) << 24)
			| (static_cast<dword>(r) << 16)
			| (static_cast<dword>(g) << 8)
			| (static_cast<dword>(b));
	}

	std::string Color::ToString(Format format) const
	{
		if (format == Format::HexRgba)
		{
			return DoubleWordToHexString(Rgba());
		}
		else if (format == Format::HexArgb)
		{
			return DoubleWordToHexString(Argb());
		}
		else
		{
			NANI_ASSERT(false);
			NANI_MESSAGE("Invalid color format.");
		}
		return std::string();
	}

}
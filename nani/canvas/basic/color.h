#pragma once
#include "basic_defs.h"
namespace nani::canvas::basic
{
	enum class Colors
	{
		Black,
		White,
		Red,
		Blue,
		Green,
		Gray,
		Yellow,
		Cyan,
		Magenta,
		Transparent
	};

	struct NANI_API Color
	{
	public:
		enum Format
		{
			HexRgba,
			HexArgb
		};

	public:
		Color() = default;
		~Color() = default;

		Color(dword rgba);
		Color(Colors color);
		Color(const std::string_view& hex);
		Color(uint8 r, uint8 g, uint8 b, uint8 a = 255);

	public:
		dword Rgba() const;
		dword Argb() const;
		std::string ToString(Format format = HexRgba) const;

	public:
		uint8 r = 0;
		uint8 g = 0;
		uint8 b = 0;
		uint8 a = 255;
	};
}
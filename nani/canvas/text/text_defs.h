#pragma once
#include "../defs.h"
#include "../basic/basic_defs.h"
#include "../basic/rectf.h"
#include "../basic/color.h"
namespace nani::canvas::text
{
	enum class TextElideMode
	{
		None,
		Left,
		Middle,
		Right
	};

	enum class TextWrapMode
	{
		NoWrap,
		Wrap
	};

	enum class FontWeight : basic::dword
	{
		Thin = 100,
		ExtraLight = 200,
		Light = 300,
		Normal = 400,
		Medium = 500,
		SemiBold = 600,
		Bold = 700,
		ExtraBold = 800,
		Black = 900
	};

	enum class FontStyle : basic::byte
	{
		Normal,
		Italic,
		Oblique
	};

	enum class DecorationLine : basic::byte
	{
		None		= 0,
		Underline	= 1 << 0,
		Overline	= 1 << 1,
		LineThrough = 1 << 2
	};

	enum class DecorationStyle : basic::byte
	{
		Solid,
		Double,
		Dotted,
		Dashed,
		Wavy
	};

	inline DecorationLine operator|(DecorationLine lhs, DecorationLine rhs)
	{
		return static_cast<DecorationLine>(
			static_cast<basic::byte>(lhs) | static_cast<basic::byte>(rhs));
	}

	inline DecorationLine operator&(DecorationLine lhs, DecorationLine rhs)
	{
		return static_cast<DecorationLine>(
			static_cast<basic::byte>(lhs) & static_cast<basic::byte>(rhs));
	}

	class Font;
	class FontManager;
	class FontMetrics;
	class TextDecoration;
	class TextAlignment;
}

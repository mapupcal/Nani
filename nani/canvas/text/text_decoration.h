#pragma once
#include "text_defs.h"

namespace nani::canvas::text
{
	class NANI_CANVAS_API TextDecoration
	{
	public:
		enum class Line : basic::byte
		{
			None		= 0,
			Underline	= 1 << 0,
			Overline	= 1 << 1,
			LineThrough = 1 << 2
		};

		enum class Style : basic::byte
		{
			Solid,
			Double,
			Dotted,
			Dashed,
			Wavy
		};

		using DecorationLine = Line;
		using DecorationStyle = Style;

	public:
		TextDecoration() = default;
		TextDecoration(const TextDecoration& other) = default;
		~TextDecoration() = default;

	public:
		TextDecoration& operator=(const TextDecoration& other) = default;

	public:
		basic::Color Color() const;
		void SetColor(basic::Color color);

		DecorationLine Lines() const;
		void SetLines(DecorationLine lines);

		DecorationStyle Style() const;
		void SetStyle(DecorationStyle style);

	private:
		basic::Color m_color = basic::Colors::Black;
		DecorationLine m_lines = DecorationLine::None;
		DecorationStyle m_style = DecorationStyle::Solid;
	};

	inline bool operator==(const TextDecoration& lhs, const TextDecoration& rhs)
	{
		return (lhs.Color() == rhs.Color()) && (lhs.Lines() == rhs.Lines()) && (lhs.Style() == rhs.Style());
	}

	inline bool operator!=(const TextDecoration& lhs, const TextDecoration& rhs)
	{
		return !(lhs == rhs);
	}
}
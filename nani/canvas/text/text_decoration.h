#pragma once
#include "text_defs.h"

namespace nani::canvas::text
{
	class NANI_CANVAS_API TextDecoration
	{
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

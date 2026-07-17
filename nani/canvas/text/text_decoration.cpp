#include "text_decoration.h"
namespace nani::canvas::text
{
	basic::Color TextDecoration::Color() const
	{
		return m_color;
	}

	void TextDecoration::SetColor(basic::Color color)
	{
		m_color = color;
	}

	TextDecoration::DecorationLine TextDecoration::Lines() const
	{
		return m_lines;
	}

	void TextDecoration::SetLines(DecorationLine lines)
	{
		m_lines = lines;
	}

	TextDecoration::DecorationStyle TextDecoration::Style() const
	{
		return m_style;
	}

	void TextDecoration::SetStyle(DecorationStyle style)
	{
		m_style = style;
	}
}

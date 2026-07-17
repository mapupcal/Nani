#include "font.h"
namespace nani::canvas::text
{
	Font::Font()
	{

	}

	const std::u8string_view Font::Family() const
	{
		return m_family;
	}

	void Font::SetFamily(const std::u8string_view& family)
	{
		m_family = family;
	}

	basic::single Font::Size() const
	{
		return m_size;
	}

	void Font::SetSize(basic::single size)
	{
		m_size = size;
	}

	Font::FontWeight Font::Weight() const
	{
		return m_weight;
	}

	void Font::SetWeight(FontWeight weight)
	{
		m_weight = weight;
	}

	Font::FontStyle Font::Style() const
	{
		return m_style;
	}

	void Font::SetStyle(FontStyle style)
	{
		m_style = style;
	}
}
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

	FontWeight Font::Weight() const
	{
		return m_weight;
	}

	void Font::SetWeight(FontWeight weight)
	{
		m_weight = weight;
	}

	FontStyle Font::Style() const
	{
		return m_style;
	}

	void Font::SetStyle(FontStyle style)
	{
		m_style = style;
	}

	size_t Font::Hash() const
	{
		size_t h1 = std::hash<std::u8string>()(m_family);
		size_t h2 = std::hash<float>()(m_size);
		size_t h3 = std::hash<int>()(static_cast<int>(m_weight));
		size_t h4 = std::hash<int>()(static_cast<int>(m_style));
		return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
	}
}

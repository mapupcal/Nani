#include "font.h"
namespace nani::canvas::text
{
	Font::Font()
	{

	}

	const std::u8string_view Font::Family() const
	{
		if (m_families.empty())
			return {};
		return m_families.front();
	}

	void Font::SetFamily(const std::u8string_view& family)
	{
		m_families.clear();
		if (!family.empty())
			m_families.emplace_back(family);
	}

	const std::vector<std::u8string>& Font::Families() const
	{
		return m_families;
	}

	void Font::SetFamilies(std::vector<std::u8string> families)
	{
		m_families = std::move(families);
		// Drop empty entries to keep equality/hash stable.
		m_families.erase(
			std::remove_if(
				m_families.begin(),
				m_families.end(),
				[](const std::u8string& name) { return name.empty(); }),
			m_families.end());
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
		size_t h = 0;
		for (const auto& family : m_families)
			h ^= std::hash<std::u8string>()(family) + 0x9e3779b9 + (h << 6) + (h >> 2);
		h ^= (std::hash<float>()(m_size) << 1);
		h ^= (std::hash<int>()(static_cast<int>(m_weight)) << 2);
		h ^= (std::hash<int>()(static_cast<int>(m_style)) << 3);
		return h;
	}
}

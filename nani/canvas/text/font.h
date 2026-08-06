#pragma once
#include "text_defs.h"

namespace nani::canvas::text
{
	class NANI_CANVAS_API Font
	{
	public:
		Font();
		Font(const Font& other) = default;
		~Font() = default;

	public:
		Font& operator=(const Font& other) = default;

	public:
		// Preferred family (first entry). Empty when Families() is empty.
		const std::u8string_view Family() const;
		void SetFamily(const std::u8string_view& family);

		const std::vector<std::u8string>& Families() const;
		void SetFamilies(std::vector<std::u8string> families);

		basic::single Size() const;
		void SetSize(basic::single size);

		FontWeight Weight() const;
		void SetWeight(FontWeight weight);

		FontStyle Style() const;
		void SetStyle(FontStyle style);

		size_t Hash() const;

	private:
		std::vector<std::u8string> m_families;
		basic::single m_size = 12.0f;
		FontWeight m_weight = FontWeight::Normal;
		FontStyle m_style = FontStyle::Normal;
	};

	inline bool operator==(const Font& lhs, const Font& rhs)
	{
		return lhs.Families() == rhs.Families() && lhs.Size() == rhs.Size() &&
			lhs.Weight() == rhs.Weight() && lhs.Style() == rhs.Style();
	}

	inline bool operator!=(const Font& lhs, const Font& rhs)
	{
		return !(lhs == rhs);
	}
}

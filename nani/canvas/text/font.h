#pragma once
#include "text_defs.h"

namespace nani::canvas::text
{
	class NANI_CANVAS_API Font
	{
	public:
		enum class Weight : basic::dword
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

		enum class Style : basic::byte
		{
			Normal,
			Italic,
			Oblique
		};

		using FontWeight = Weight;
		using FontStyle = Style;

	public:
		Font();
		Font(const Font& other) = default;
		~Font() = default;

	public:
		Font& operator=(const Font& other) = default;

	public:
		const std::u8string_view Family() const;
		void SetFamily(const std::u8string_view& family);

		basic::single Size() const;
		void SetSize(basic::single size);

		FontWeight Weight() const;
		void SetWeight(FontWeight weight);

		FontStyle Style() const;
		void SetStyle(FontStyle style);

	private:
		std::u8string m_family;
		basic::single m_size = 12.0f;
		FontWeight m_weight = FontWeight::Normal;
		FontStyle m_style = FontStyle::Normal;
	};

	inline bool operator==(const Font& lhs, const Font& rhs)
	{
		return lhs.Family() == rhs.Family() && lhs.Size() == rhs.Size() &&
			lhs.Weight() == rhs.Weight() && lhs.Style() == rhs.Style();
	}

	inline bool operator!=(const Font& lhs, const Font& rhs)
	{
		return !(lhs == rhs);
	}
}
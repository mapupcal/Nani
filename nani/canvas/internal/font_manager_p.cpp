#include "font_manager_p.h"
#include "skia_utils.h"
#include <core/SkFontMgr.h>
#include <core/SkTypes.h>
#include <core/SkTypeface.h>
#include <algorithm>
#include "text/font.h"
namespace
{
	using namespace nani::canvas::text;
	// Helper: Convert Font::Weight to SkFontStyle::Weight
	SkFontStyle::Weight ConvertWeight(Font::FontWeight weight)
	{
		using Weight = Font::FontWeight;
		switch (weight)
		{
		case Weight::Thin:
			return SkFontStyle::kThin_Weight;
		case Weight::ExtraLight:
			return SkFontStyle::kExtraLight_Weight;
		case Weight::Light:
			return SkFontStyle::kLight_Weight;
		case Weight::Normal:
			return SkFontStyle::kNormal_Weight;
		case Weight::Medium:
			return SkFontStyle::kMedium_Weight;
		case Weight::SemiBold:
			return SkFontStyle::kSemiBold_Weight;
		case Weight::Bold:
			return SkFontStyle::kBold_Weight;
		case Weight::ExtraBold:
			return SkFontStyle::kExtraBold_Weight;
		case Weight::Black:
			return SkFontStyle::kBlack_Weight;
		default:
			return SkFontStyle::kNormal_Weight;
		}
	}

	// Helper: Convert Font::Style to SkFontStyle::Slant
	SkFontStyle::Slant ConvertStyle(Font::FontStyle style)
	{
		using Style = Font::FontStyle;

		switch (style)
		{
		case Style::Normal:
			return SkFontStyle::kUpright_Slant;
		case Style::Italic:
			return SkFontStyle::kItalic_Slant;
		case Style::Oblique:
			return SkFontStyle::kOblique_Slant;
		default:
			return SkFontStyle::kUpright_Slant;
		}
	}

	// Helper: Create std::shared_ptr<SkFont>
	std::shared_ptr<SkFont> MakeSharedSkFont(
		sk_sp<SkTypeface> typeface,
		float size)
	{
		if (!typeface)
		{
			return nullptr;
		}

		return std::make_shared<SkFont>(typeface, size);
	}

	// Helper: Convert std::u8string_view to SkString
	SkString U8StringToSkString(const std::u8string_view& u8str)
	{
		const char* data = reinterpret_cast<const char*>(u8str.data());
		return SkString(data, u8str.size());
	}

}

namespace nani::canvas::internal
{
	FontManagerPrivate* FontManagerPrivate::Instance()
	{
		static FontManagerPrivate ins;
		return &ins;
	}

	std::vector<std::u8string> FontManagerPrivate::DefaultFamilies() const
	{
		std::vector<std::u8string> families;

		if (!m_spSkFontMgr)
		{
			return families;
		}

		// Get the number of font families
		int familyCount = m_spSkFontMgr->countFamilies();
		if (familyCount <= 0)
		{
			return families;
		}

		// Reserve space for performance
		families.reserve(familyCount);

		// Iterate through all families and collect their names
		for (int i = 0; i < familyCount; ++i)
		{
			SkString familyName;
			m_spSkFontMgr->getFamilyName(i, &familyName);

			if (!familyName.isEmpty())
			{
				// Convert SkString to u8string
				std::u8string u8Name(reinterpret_cast<const char8_t*>(familyName.c_str()),
					familyName.size());
				families.push_back(std::move(u8Name));
			}
		}

		return families;
	}

	std::shared_ptr<SkFont> FontManagerPrivate::CreateSkFont(const text::Font& font)
	{
		if (!m_spSkFontMgr)
		{
			return nullptr;
		}

		// Check cache
		auto it = m_fontCache.find(font);
		if (it != m_fontCache.end())
		{
			std::shared_ptr<SkFont> cachedFont = it->second.lock();
			if (cachedFont)
			{
				return cachedFont;
			}
			else
			{
				// Weak pointer expired, remove from cache
				m_fontCache.erase(it);
			}
		}

		// Convert font properties to Skia types
		SkFontStyle::Weight skWeight = ConvertWeight(font.Weight());
		SkFontStyle::Slant skSlant = ConvertStyle(font.Style());
		SkFontStyle style(skWeight, SkFontStyle::kNormal_Width, skSlant);

		sk_sp<SkTypeface> typeface;
		std::u8string_view family = font.Family();

		// Try to find the font by family name with the specified style
		if (!family.empty())
		{
			SkString skFamilyName = U8StringToSkString(family);

			// Try to match the family with the style
			typeface = m_spSkFontMgr->matchFamilyStyle(skFamilyName.c_str(), style);

			// If not found, try with normal style
			if (!typeface)
			{
				typeface = m_spSkFontMgr->matchFamilyStyle(
					skFamilyName.c_str(),
					SkFontStyle::Normal());
			}
		}

		// If still not found, fallback to default font with specified style
		if (!typeface)
		{
			typeface = m_spSkFontMgr->matchFamilyStyle(nullptr, style);
		}

		// Last resort: legacy default
		if (!typeface)
		{
			typeface = m_spSkFontMgr->legacyMakeTypeface(
				nullptr,
				SkFontStyle::Normal());
		}

		if (!typeface)
		{
			return nullptr;
		}

		// Create SkFont
		std::shared_ptr<SkFont> skFont = MakeSharedSkFont(typeface, font.Size());

		if (skFont)
		{
			// Store in cache
			m_fontCache[font] = skFont;
		}

		return skFont;
	}

	void FontManagerPrivate::ClearCache()
	{
		m_fontCache.clear();
	}

	FontManagerPrivate::FontManagerPrivate()
	{
		m_spSkFontMgr = skia_utils::CreateDefaultFontMgr();
	}

	FontManagerPrivate::~FontManagerPrivate()
	{

	}
}

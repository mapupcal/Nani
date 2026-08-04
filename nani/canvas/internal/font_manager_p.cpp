#include "font_manager_p.h"
#include "skia_utils.h"
#include "text/font.h"

namespace
{
	using namespace nani::canvas::text;
	SkFontStyle::Weight ConvertWeight(FontWeight weight)
	{
		switch (weight)
		{
		case FontWeight::Thin:
			return SkFontStyle::kThin_Weight;
		case FontWeight::ExtraLight:
			return SkFontStyle::kExtraLight_Weight;
		case FontWeight::Light:
			return SkFontStyle::kLight_Weight;
		case FontWeight::Normal:
			return SkFontStyle::kNormal_Weight;
		case FontWeight::Medium:
			return SkFontStyle::kMedium_Weight;
		case FontWeight::SemiBold:
			return SkFontStyle::kSemiBold_Weight;
		case FontWeight::Bold:
			return SkFontStyle::kBold_Weight;
		case FontWeight::ExtraBold:
			return SkFontStyle::kExtraBold_Weight;
		case FontWeight::Black:
			return SkFontStyle::kBlack_Weight;
		default:
			return SkFontStyle::kNormal_Weight;
		}
	}

	SkFontStyle::Slant ConvertStyle(FontStyle style)
	{
		switch (style)
		{
		case FontStyle::Normal:
			return SkFontStyle::kUpright_Slant;
		case FontStyle::Italic:
			return SkFontStyle::kItalic_Slant;
		case FontStyle::Oblique:
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

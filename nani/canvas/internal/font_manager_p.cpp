#include "font_manager_p.h"
#include "skia_utils.h"
#include "text/font.h"

namespace
{
	using namespace nani::canvas::text;
	using namespace nani::canvas::internal;

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

	std::shared_ptr<SkFont> MakeSharedSkFont(
		sk_sp<SkTypeface> typeface,
		float size)
	{
		if (!typeface)
			return nullptr;

		return std::make_shared<SkFont>(typeface, size);
	}

	std::vector<std::u8string> PlatformFallbackFamilies()
	{
		std::vector<std::u8string> families;
#ifdef NANI_OS_WIN
		families = {
			u8"Microsoft YaHei UI",
			u8"Microsoft YaHei",
			u8"Segoe UI Emoji",
			u8"SimSun",
		};
#endif
		return families;
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
			return families;

		int familyCount = m_spSkFontMgr->countFamilies();
		if (familyCount <= 0)
			return families;

		families.reserve(familyCount);
		for (int i = 0; i < familyCount; ++i)
		{
			SkString familyName;
			m_spSkFontMgr->getFamilyName(i, &familyName);

			if (!familyName.isEmpty())
				families.push_back(skia_utils::ToU8String(familyName));
		}

		return families;
	}

	const std::vector<std::u8string>& FontManagerPrivate::FallbackFamilies() const
	{
		return m_fallbackFamilies;
	}

	std::shared_ptr<SkFont> FontManagerPrivate::CreateSkFont(const text::Font& font)
	{
		if (!m_spSkFontMgr)
			return nullptr;

		auto it = m_fontCache.find(font);
		if (it != m_fontCache.end())
		{
			std::shared_ptr<SkFont> cachedFont = it->second.lock();
			if (cachedFont)
				return cachedFont;
			m_fontCache.erase(it);
		}

		SkFontStyle::Weight skWeight = ConvertWeight(font.Weight());
		SkFontStyle::Slant skSlant = ConvertStyle(font.Style());
		SkFontStyle style(skWeight, SkFontStyle::kNormal_Width, skSlant);

		sk_sp<SkTypeface> typeface;
		for (const auto& family : font.Families())
		{
			typeface = skia_utils::MatchFamilyStyle(m_spSkFontMgr.get(), family, style);
			if (typeface)
				break;
		}

		if (!typeface)
		{
			for (const auto& family : m_fallbackFamilies)
			{
				typeface = skia_utils::MatchFamilyStyle(m_spSkFontMgr.get(), family, style);
				if (typeface)
					break;
			}
		}

		if (!typeface)
			typeface = m_spSkFontMgr->matchFamilyStyle(nullptr, style);

		if (!typeface)
		{
			typeface = m_spSkFontMgr->legacyMakeTypeface(
				nullptr,
				SkFontStyle::Normal());
		}

		if (!typeface)
			return nullptr;

		std::shared_ptr<SkFont> skFont = MakeSharedSkFont(typeface, font.Size());
		if (skFont)
			m_fontCache[font] = skFont;

		return skFont;
	}

	void FontManagerPrivate::ClearCache()
	{
		m_fontCache.clear();
	}

	SkFontMgr* FontManagerPrivate::FontMgr() const
	{
		return m_spSkFontMgr.get();
	}

	FontManagerPrivate::FontManagerPrivate()
	{
		m_spSkFontMgr = skia_utils::CreateDefaultFontMgr();
		m_fallbackFamilies = PlatformFallbackFamilies();
	}

	FontManagerPrivate::~FontManagerPrivate()
	{
	}
}

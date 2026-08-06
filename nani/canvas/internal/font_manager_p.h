#pragma once

#include "defs.h"
#include "skia_defs.h"

#include "../text/font.h"

#include <unordered_map>

namespace nani::canvas::internal
{
	class FontManagerPrivate
	{
	public:
		static FontManagerPrivate* Instance();

	public:
		std::vector<std::u8string> DefaultFamilies() const;
		const std::vector<std::u8string>& FallbackFamilies() const;
		void ClearCache();

		std::shared_ptr<SkFont> CreateSkFont(const text::Font& font);
		SkFontMgr* FontMgr() const;

	private:
		FontManagerPrivate();
		~FontManagerPrivate();

	private:
		struct FontHash
		{
			size_t operator()(const text::Font& font) const
			{
				return font.Hash();
			}
		};

	private:
		sk_sp<SkFontMgr> m_spSkFontMgr;
		std::vector<std::u8string> m_fallbackFamilies;
		std::unordered_map<text::Font, std::weak_ptr<SkFont>, FontHash> m_fontCache;
	};
}

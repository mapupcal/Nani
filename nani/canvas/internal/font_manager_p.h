#pragma once
#include "defs.h"
#include <core/SkFontMgr.h>

namespace nani::canvas::internal
{
	class FontManagerPrivate
	{
	public:
		static FontManagerPrivate* Instance();

	public:
		std::vector<std::u8string> DefaultFamilies() const;
		std::vector<std::u8string> AvailableFamilies() const;
		std::vector<std::u8string> RegisteredFamilies() const;
		bool RegisterFont(const std::u8string_view& fontFilePath, std::u8string& outFamily);
		bool UnRegisterFont(const std::u8string_view& fontFilePath);
		void ClearCache();

		std::shared_ptr<const FontMetricsPrivate> GetMetrics(const text::Font& font);

	private:
		FontManagerPrivate();
		~FontManagerPrivate();

	private:
		sk_sp<SkFontMgr> m_spSkFontMgr;

		std::vector<std::u8string> m_defaultFamilies;
		std::map<std::u8string, sk_sp<SkTypeface>> m_registeredTypefaces;
	};
}
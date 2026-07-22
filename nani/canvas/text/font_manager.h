#pragma once
#include "text_defs.h"

namespace nani::canvas::text
{
	class NANI_CANVAS_API FontManager
	{
	public:
		FontManager() = default;
		~FontManager() = default;

	public:
		std::vector<std::u8string> DefaultFamilies() const;
		std::vector<std::u8string> AvailableFamilies() const;

		std::vector<std::u8string> RegisteredFamilies() const;
		bool RegisterFont(const std::u8string_view& fontFilePath, std::u8string& outFamily);
		bool UnRegisterFont(const std::u8string_view& fontFilePath);

		void ClearCache();
	};
}
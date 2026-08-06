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
		// Platform default glyph fallback chain (after Font.Families()).
		std::vector<std::u8string> FallbackFamilies() const;
		void ClearCache();
	};
}

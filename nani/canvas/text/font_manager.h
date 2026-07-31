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
		void ClearCache();
	};
}

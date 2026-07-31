#include "font_manager.h"
#include "../internal/font_manager_p.h"
namespace nani::canvas::text
{
	std::vector<std::u8string> FontManager::DefaultFamilies() const
	{
		return internal::FontManagerPrivate::Instance()->DefaultFamilies();
	}

	void FontManager::ClearCache()
	{
		internal::FontManagerPrivate::Instance()->ClearCache();
	}
}

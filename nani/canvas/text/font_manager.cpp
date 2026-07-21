#include "font_manager.h"
#include "../internal/font_manager_p.h"
namespace nani::canvas::text
{
	std::vector<std::u8string> FontManager::DefaultFamilies() const
	{
		return internal::FontManagerPrivate::Instance()->DefaultFamilies();
	}

	std::vector<std::u8string> FontManager::AvailableFamilies() const
	{
		return internal::FontManagerPrivate::Instance()->AvailableFamilies();
	}

	bool FontManager::RegisterFont(const std::u8string_view& fontFilePath)
	{
		return internal::FontManagerPrivate::Instance()->RegisterFont(fontFilePath);
	}

	bool FontManager::UnRegisterFont(const std::u8string_view& fontFilePath)
	{
		return internal::FontManagerPrivate::Instance()->UnRegisterFont(fontFilePath);
	}

	void FontManager::ClearCache()
	{
		internal::FontManagerPrivate::Instance()->ClearCache();
	}
}
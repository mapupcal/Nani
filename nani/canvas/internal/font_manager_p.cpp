#include "font_manager_p.h"
#include "font_metrics_p.h"
#include "../text/font.h"

namespace nani::canvas::internal
{
	FontManagerPrivate* FontManagerPrivate::Instance()
	{
		static FontManagerPrivate ins;
		return &ins;
	}

	std::vector<std::u8string> FontManagerPrivate::DefaultFamilies() const
	{
		return {};
	}

	std::vector<std::u8string> FontManagerPrivate::AvailableFamilies() const
	{
		return {};
	}

	bool FontManagerPrivate::RegisterFont(const std::u8string_view& fontFilePath)
	{
		return false;
	}

	bool FontManagerPrivate::UnRegisterFont(const std::u8string_view& fontFilePath)
	{
		return false;
	}

	void FontManagerPrivate::ClearCache()
	{

	}

	std::shared_ptr<const FontMetricsPrivate> FontManagerPrivate::GetMetrics(const text::Font& font)
	{
		return std::make_shared<FontMetricsPrivate>();
	}
}
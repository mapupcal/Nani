#include "font_manager_p.h"
#include "font_metrics_p.h"
#include "../text/font.h"
#include "skia_utils.h"

namespace nani::canvas::internal
{
	FontManagerPrivate* FontManagerPrivate::Instance()
	{
		static FontManagerPrivate ins;
		return &ins;
	}

	std::vector<std::u8string> FontManagerPrivate::DefaultFamilies() const
	{
		return m_defaultFamilies;
	}

	std::vector<std::u8string> FontManagerPrivate::AvailableFamilies() const
	{
		auto defaults = DefaultFamilies();
		auto registers = RegisteredFamilies();

		std::vector<std::u8string> families = defaults;
		families.reserve(defaults.size() + registers.size());
		families.insert(families.end(), registers.cbegin(), registers.cend());
		return families;
	}

	std::vector<std::u8string> FontManagerPrivate::RegisteredFamilies() const
	{
		std::vector<std::u8string> families;
		families.reserve(m_registeredTypefaces.size());

		for (const auto& [path, typeface] : m_registeredTypefaces)
			families.push_back(skia_utils::GetFamilyName(typeface));

		return families;
	}

	bool FontManagerPrivate::RegisterFont(const std::u8string_view& fontFilePath, std::u8string& outFamily)
	{	
		std::u8string fontPath(fontFilePath);
		auto iter = m_registeredTypefaces.find(fontPath);
		if (iter != m_registeredTypefaces.cend() && iter->second)
		{
			outFamily = skia_utils::GetFamilyName(iter->second);
			return true;
		}

		auto typeface = m_spSkFontMgr->makeFromFile(reinterpret_cast<const char*>(fontPath.c_str()));
		if (!typeface)
			return false;

		m_registeredTypefaces.insert_or_assign(fontPath, typeface);
		outFamily = skia_utils::GetFamilyName(typeface);
		return true;
	}

	bool FontManagerPrivate::UnRegisterFont(const std::u8string_view& fontFilePath)
	{
		std::u8string fontPath(fontFilePath);
		auto iter = m_registeredTypefaces.find(fontPath);
		if (iter == m_registeredTypefaces.cend() || !iter->second)
			return false;
		m_registeredTypefaces.erase(iter);
		return true;
	}

	void FontManagerPrivate::ClearCache()
	{

	}

	std::shared_ptr<const FontMetricsPrivate> FontManagerPrivate::GetMetrics(const text::Font& font)
	{
		return std::make_shared<FontMetricsPrivate>();
	}

	FontManagerPrivate::FontManagerPrivate()
	{
		m_spSkFontMgr = skia_utils::CreateDefaultFontMgr();
	}

	FontManagerPrivate::~FontManagerPrivate()
	{

	}
}
#pragma once
#include "elements/elements_defs.h"
namespace nani::canvas
{
	class NANI_CANVAS_API Styles
	{
	public:
		Styles();
		~Styles();

	public:
		void LoadFromFile(const std::u8string_view& filePath);
		void LoadFromXML(const std::string& utf8XMLData);

	private:
		friend class visuals::View;
		friend class visuals::Visual;
		void LoadXMLDocuemnt(std::any pXmlDocument);
		std::shared_ptr<internal::ComputedStyle> Compute(elements::Element* element);
		void Override(const std::u8string_view& id, const internal::ComputedStyle& style);

	private:
		std::map<std::u8string, std::shared_ptr<internal::ComputedStyle>> m_mapComputedStyles;
		std::map<std::u8string, std::shared_ptr<internal::ComputedStyleBuilder>> m_mapComputedStyleBuilders;
		std::map<std::u8string, std::set<std::u8string>> m_mapInherits;
	};
}
#include "styles.h"
#include "elements/element.h"
#include "elements/element_states.h"
#include "internal/computed_style.h"
#include "internal/computed_style_builder.h"
#include <pugixml.hpp>

using namespace nani::canvas::internal;
using namespace nani::canvas::elements;

namespace
{
	std::u8string to_u8string(const std::string_view& str)
	{
		return std::u8string(str.cbegin(), str.cend());
	}

	std::string to_string(const std::u8string_view& str)
	{
		return std::string(str.cbegin(), str.cend());
	}
}

namespace nani::canvas
{
	Styles::Styles()
	{

	}

	Styles::~Styles()
	{

	}

	void Styles::LoadFromFile(const std::u8string_view& filePath)
	{
		pugi::xml_document doc;

		std::string utf8Path(filePath.begin(), filePath.end());
		pugi::xml_parse_result result = doc.load_file(utf8Path.c_str());

		if (!result)
		{
			NANI_ASSERT(false);
			NANI_MESSAGE(result.description());
			return;
		}

		LoadXMLDocuemnt(&doc);
	}

	void Styles::LoadFromXML(const std::string& utf8XMLData)
	{
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_string(utf8XMLData.c_str());
		if (!result)
		{
			NANI_ASSERT(false);
			NANI_MESSAGE(result.description());
			return;
		}

		LoadXMLDocuemnt(&doc);
	}

	void Styles::LoadXMLDocuemnt(std::any pXmlDocument)
	{
		pugi::xml_document* pDoc = std::any_cast<pugi::xml_document*>(pXmlDocument);
		if (!pDoc)
			return;

		pugi::xml_document& doc = *pDoc;
		auto lstStyleXMLNode = doc.child("Styles").children("Style");

		for (const auto& styleNode : lstStyleXMLNode)
		{
			std::u8string styleClass = to_u8string(styleNode.attribute("class").as_string());
			if (styleClass.empty())
				continue;

			std::shared_ptr<ComputedStyleBuilder> builder = std::make_shared<ComputedStyleBuilder>();
			builder->Load(styleNode);

			std::u8string state = to_u8string(styleNode.attribute("state").as_string());
			if (!state.empty())
			{
				m_mapStateStyles[styleClass].insert(state);
				m_mapComputedStyleBuilders.insert_or_assign(styleClass + u8"-" + state, builder);
				continue;
			}

			std::u8string inherit = to_u8string(styleNode.attribute("inherit").as_string());
			if (!inherit.empty())
				m_mapInheritStyle.insert_or_assign(styleClass, inherit);

			m_mapComputedStyleBuilders.insert_or_assign(styleClass, builder);
		}

		for (const auto& pair : m_mapStateStyles)
		{
			const std::u8string& styleClass = pair.first;
			for (const auto& state : pair.second)
			{
				auto iterBase = m_mapComputedStyleBuilders.find(styleClass);
				if (iterBase == m_mapComputedStyleBuilders.cend())
				{
					NANI_MESSAGE(std::format("state style : {} not found for state : {}", to_string(styleClass), to_string(state)));
					continue;
				}
				auto iterState = m_mapComputedStyleBuilders.find(styleClass + u8"-" + state);
				iterState->second->Inherit(iterBase->second.get());
			}
		}

		std::map<std::u8string, std::set<std::u8string>> mapDerivedStyles;
		for (const auto& pair : m_mapInheritStyle)
			mapDerivedStyles[pair.second].insert(pair.first);

		for (const auto& pair : mapDerivedStyles)
		{
			const std::u8string& inherit = pair.first;
			for (const auto& styleClass : pair.second)
			{
				auto iterInherit = m_mapComputedStyleBuilders.find(inherit);
				if (iterInherit == m_mapComputedStyleBuilders.cend())
				{
					NANI_MESSAGE(std::format("inherit style : {} not found for style : {}", to_string(inherit), to_string(styleClass)));
					continue;
				}
				auto iterStyle = m_mapComputedStyleBuilders.find(styleClass);
				iterStyle->second->Inherit(iterInherit->second.get());
			}
		}

		m_mapComputedStyles.clear();
	}

	std::shared_ptr<ComputedStyle> Styles::Compute(const std::u8string_view& styleClass, const std::u8string_view& state)
	{
		if(state.empty())
		{
			if (auto iter = m_mapComputedStyles.find(std::u8string(styleClass)); iter != m_mapComputedStyles.cend())
				return iter->second;

			if (auto iter = m_mapComputedStyleBuilders.find(std::u8string(styleClass)); iter != m_mapComputedStyleBuilders.cend())
			{
				std::shared_ptr<ComputedStyle> computedStyle = std::make_shared<ComputedStyle>(iter->second->Compute());
				m_mapComputedStyles.insert_or_assign(std::u8string(styleClass), computedStyle);
				return computedStyle;
			}

			std::shared_ptr<ComputedStyle> computedStyle = std::make_shared<ComputedStyle>();
			m_mapComputedStyles.insert_or_assign(std::u8string(styleClass), computedStyle);
			return computedStyle;
		}

		std::u8string stateStyleClass = std::u8string(styleClass) + u8"-" + std::u8string(state);
		if (auto iter = m_mapComputedStyles.find(stateStyleClass); iter != m_mapComputedStyles.cend())
			return iter->second;

		if (auto iter = m_mapComputedStyleBuilders.find(stateStyleClass); iter != m_mapComputedStyleBuilders.cend())
		{
			std::shared_ptr<ComputedStyle> computedStyle = std::make_shared<ComputedStyle>(iter->second->Compute());
			m_mapComputedStyles.insert_or_assign(stateStyleClass, computedStyle);
			return computedStyle;
		}

		auto iterInherit = m_mapInheritStyle.find(std::u8string(styleClass));
		while (iterInherit != m_mapInheritStyle.cend())
		{
			std::u8string inheritStyleClass = iterInherit->second;
			std::u8string inheritStateStyleClass = inheritStyleClass + u8"-" + std::u8string(state);
			if (auto iter = m_mapComputedStyleBuilders.find(inheritStateStyleClass); iter != m_mapComputedStyleBuilders.cend())
			{
				std::shared_ptr<ComputedStyleBuilder> tempStateStyleBuilder = std::make_shared<ComputedStyleBuilder>();
				tempStateStyleBuilder->CopyFrom(*iter->second);
				tempStateStyleBuilder->Inherit(m_mapComputedStyleBuilders[std::u8string(styleClass)].get());
				std::shared_ptr<ComputedStyle> computedStyle = std::make_shared<ComputedStyle>(tempStateStyleBuilder->Compute());
				m_mapComputedStyles.insert_or_assign(inheritStateStyleClass, computedStyle);
				return computedStyle;
			}
			iterInherit = m_mapInheritStyle.find(inheritStyleClass);
		}

		// If no specific state style is found, fall back to the base style class
		return Compute(styleClass);
	}

	std::shared_ptr<ComputedStyle> Styles::Compute(Element* element)
	{
		if (!element)
			return nullptr;

		const std::u8string_view styleClass = element->StyleClass();
		const std::u8string_view stateProps = element->States()->GetStateProps();
		return Compute(styleClass, stateProps);
	}

	void Styles::Override(const std::u8string_view& id, const ComputedStyle& style)
	{
		m_mapComputedStyles.insert_or_assign(std::u8string(id), std::make_shared<ComputedStyle>(style));
	}
}
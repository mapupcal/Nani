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
	std::u8string to_u8string(const std::string& str)
	{
		return std::u8string(reinterpret_cast<const char8_t*>(str.data()), str.size());
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

		struct SrcStyleInfo
		{
			std::u8string styleClass;
			std::u8string inheritStyleClass;
			std::shared_ptr<ComputedStyleBuilder> builder;
		};

		// Pass 1: load all builders, record inherit relationships.
		std::vector<SrcStyleInfo> loadedInfos;
		for (const auto& styleNode : lstStyleXMLNode)
		{
			std::u8string styleClass = to_u8string(styleNode.attribute("class").as_string());
			if (styleClass.empty())
				continue;

			std::shared_ptr<ComputedStyleBuilder> builder = std::make_shared<ComputedStyleBuilder>();
			builder->Load(styleNode);

			std::u8string inheritStyleClass = to_u8string(styleNode.attribute("inherit").as_string());
			m_mapComputedStyleBuilders.insert_or_assign(styleClass, builder);
			loadedInfos.push_back({ std::move(styleClass), std::move(inheritStyleClass), builder });
		}

		// Pass 2: wire up inherit and update old inheritors.
		for (const auto& info : loadedInfos)
		{
			if (!info.inheritStyleClass.empty())
			{
				// Wire new builder's inherit.
				auto iterBase = m_mapComputedStyleBuilders.find(info.inheritStyleClass);
				if (iterBase != m_mapComputedStyleBuilders.cend())
					info.builder->Inherit(iterBase->second.get());

				// Track: styleClass -> parent's class, for later hot-reload lookup.
				m_mapInherits[info.inheritStyleClass].insert(info.styleClass);
			}

			// Hot reload: update old inheritors of this class to point to the new builder.
			auto inheritsIter = m_mapInherits.find(info.styleClass);
			if (inheritsIter != m_mapInherits.cend())
			{
				for (const std::u8string& oldInheritorClass : inheritsIter->second)
				{
					auto oldInheritorIter = m_mapComputedStyleBuilders.find(oldInheritorClass);
					if (oldInheritorIter == m_mapComputedStyleBuilders.cend())
						continue;
					oldInheritorIter->second->Inherit(info.builder.get());
				}
			}
		}

		// refresh all computed styles.
		m_mapComputedStyles.clear();
	}

	std::shared_ptr<ComputedStyle> Styles::Compute(const std::u8string_view& styleClass)
	{
		auto iter = m_mapComputedStyles.find(std::u8string(styleClass));
		if (iter != m_mapComputedStyles.cend())
			return iter->second;

		auto iterBuilder = m_mapComputedStyleBuilders.find(std::u8string(styleClass));
		if (iterBuilder != m_mapComputedStyleBuilders.cend())
			return std::make_shared<ComputedStyle>(iterBuilder->second->Compute());

		std::shared_ptr<ComputedStyle> computedStyle = std::make_shared<ComputedStyle>();
		m_mapComputedStyles.emplace(std::u8string(styleClass), computedStyle);
		return computedStyle;
	}

	std::shared_ptr<ComputedStyle> Styles::Compute(Element* element)
	{
		if (!element)
			return nullptr;

		const std::u8string_view styleClass = element->StyleClass();
		const std::u8string_view stateProps = element->States()->GetStateProps();

		std::u8string computeStyleId;
		computeStyleId.reserve(styleClass.size() + stateProps.size());
		computeStyleId.append(styleClass);
		computeStyleId.append(stateProps);

		// style with states.
		auto iter = m_mapComputedStyles.find(computeStyleId);
		if (iter != m_mapComputedStyles.cend())
			return iter->second;

		auto iterBuilder = m_mapComputedStyleBuilders.find(computeStyleId);
		if (iterBuilder != m_mapComputedStyleBuilders.cend())
			return std::make_shared<ComputedStyle>(iterBuilder->second->Compute());

		// fallback to style without states.
		iter = m_mapComputedStyles.find(std::u8string(styleClass));
		if (iter != m_mapComputedStyles.cend())
			return iter->second;

		iterBuilder = m_mapComputedStyleBuilders.find(std::u8string(styleClass));
		if (iterBuilder != m_mapComputedStyleBuilders.cend())
			return std::make_shared<ComputedStyle>(iterBuilder->second->Compute());

		std::shared_ptr<ComputedStyle> computedStyle = std::make_shared<ComputedStyle>();
		m_mapComputedStyles.emplace(computeStyleId, computedStyle);
		return computedStyle;
	}

	void Styles::Override(const std::u8string_view& id, const ComputedStyle& style)
	{
		m_mapComputedStyles.insert_or_assign(std::u8string(id), std::make_shared<ComputedStyle>(style));
	}
}
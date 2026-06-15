#include "styles.h"
#include "element.h"
#include "element_states.h"

namespace nani::canvas::elements
{
	ComputedStyle::ComputedStyle()
	{

	}

	ComputedStyle::~ComputedStyle()
	{

	}

	Styles::Styles()
	{

	}

	Styles::~Styles()
	{
		for (auto pair : m_mapComputedStyles)
			delete pair.second;
	}

	ComputedStyle* Styles::Compute(Element* element)
	{
		if (!element)
			return nullptr;

		const std::u8string_view styleClass = element->SylteClass();
		const std::u8string_view stateProps = element->States()->GetStateProps();

		std::u8string computeStyleId(styleClass.size() + stateProps.size(), u8'\0');
		computeStyleId.append(styleClass);
		computeStyleId.append(stateProps);

		auto iter = m_mapComputedStyles.find(computeStyleId);
		if (iter != m_mapComputedStyles.cend())
			return iter->second;

		ComputedStyle* computedSytle = new ComputedStyle();
		//TODO: Compute Style
		m_mapComputedStyles.emplace(computeStyleId, computedSytle);
		return computedSytle;
	}
}
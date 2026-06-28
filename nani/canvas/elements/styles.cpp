#include "styles.h"
#include "element.h"
#include "element_states.h"
#include "../internal/computed_style.h"
using namespace nani::canvas::internal;

namespace nani::canvas::elements
{
	Styles::Styles()
	{

	}

	Styles::~Styles()
	{

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

		// fallback to style without states.
		iter = m_mapComputedStyles.find(std::u8string(styleClass));
		if (iter != m_mapComputedStyles.cend())
			return iter->second;

		std::shared_ptr<ComputedStyle> computedStyle = std::make_shared<ComputedStyle>();
		//TODO: Compute Style
		m_mapComputedStyles.emplace(computeStyleId, computedStyle);
		return computedStyle;
	}

	void Styles::Override(const std::u8string_view& id, const ComputedStyle& style)
	{
		m_mapComputedStyles.insert_or_assign(std::u8string(id), std::make_shared<ComputedStyle>(style));
	}
}
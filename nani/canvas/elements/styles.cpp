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

	bool operator==(const ComputedStyle::LayoutProperties& lhs, const ComputedStyle::LayoutProperties& rhs)
	{
		return lhs.style == rhs.style;
	}

	bool operator==(const ComputedStyle::VisualProperties::BorderRadius& lhs, const ComputedStyle::VisualProperties::BorderRadius& rhs)
	{
		return basic::IsScalarEqual(lhs.topLeft, rhs.topLeft) &&
			basic::IsScalarEqual(lhs.topRight, rhs.topRight) &&
			basic::IsScalarEqual(lhs.bottomLeft, rhs.bottomLeft) &&
			basic::IsScalarEqual(lhs.bottomRight, rhs.bottomRight);
	}

	bool operator==(const ComputedStyle::VisualProperties::Shadow& lhs, const ComputedStyle::VisualProperties::Shadow& rhs)
	{
		return (lhs.color == rhs.color) && 
			basic::IsScalarEqual(lhs.offsetX, rhs.offsetX) &&
			basic::IsScalarEqual(lhs.offsetY, rhs.offsetY) &&
			basic::IsScalarEqual(lhs.blur, rhs.blur) &&
			basic::IsScalarEqual(lhs.spread, rhs.spread);
	}

	bool operator==(const ComputedStyle::VisualProperties& lhs, const ComputedStyle::VisualProperties& rhs)
	{
		return (lhs.color == rhs.color) && 
			(lhs.borderColor == rhs.borderColor) &&
			(lhs.backgroundColor == rhs.backgroundColor) &&
			basic::IsScalarEqual(lhs.opacity, rhs.opacity) &&
			(lhs.radius == rhs.radius) &&
			(lhs.shadow == rhs.shadow);
	}

	const ComputedStyle::DiffResult ComputedStyle::Diff(const ComputedStyle* other) const
	{
		if (!other)
			return { .layoutChanged = true, .visualChanged = true };
		return { .layoutChanged = !(layoutProps == other->layoutProps), .visualChanged = !(visualProps == other->visualProps) };
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
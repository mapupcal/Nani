#pragma once
#include "computed_style.h"
#include <pugixml.hpp>
#include <yoga/style/Style.h>

#define ComputeStyleBuilderOptionalProperty(value_class, value_name)		\
	std::optional<value_class> value_name;									\
	std::optional<value_class> Compute##value_name() const					\
	{																		\
		if (!value_name.has_value() && m_inheritBuilder)					\
			return m_inheritBuilder->Compute##value_name();					\
		return value_name;													\
	}																		\

namespace nani::canvas::internal
{
	class ComputedStyleBuilder
	{
	public:
		void Load(const pugi::xml_node& styleNode);
		void Inherit(const ComputedStyleBuilder* inheritBuilder);
		ComputedStyle Compute() const;

	public:
		ComputeStyleBuilderOptionalProperty(facebook::yoga::FlexDirection, FlexDirection);
		ComputeStyleBuilderOptionalProperty(facebook::yoga::StyleLength, Width);
		ComputeStyleBuilderOptionalProperty(facebook::yoga::StyleLength, Height);
		ComputeStyleBuilderOptionalProperty(facebook::yoga::StyleLength, MinWidth);
		ComputeStyleBuilderOptionalProperty(facebook::yoga::StyleLength, MinHeight);
		ComputeStyleBuilderOptionalProperty(facebook::yoga::StyleLength, MaxWidth);
		ComputeStyleBuilderOptionalProperty(facebook::yoga::StyleLength, MaxHeight);
	private:
		const ComputedStyleBuilder* m_inheritBuilder = nullptr;
	};
}
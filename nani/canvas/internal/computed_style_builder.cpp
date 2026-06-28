#include "computed_style_builder.h"
#include "../basic/basic_defs.h"

namespace
{
	using namespace nani::canvas::basic;
	using namespace nani::canvas::internal;
	using namespace facebook::yoga;

	std::string GetStyleNodeValue(const pugi::xml_node& node, const std::string_view& name)
	{
		pugi::xml_node childNode = node.child(name);
		if (childNode.empty())
			return std::string();

		pugi::xml_attribute attribute = childNode.attribute("value");
		if (attribute.empty())
			return std::string();

		return attribute.as_string();
	}

	std::optional<StyleLength> AsYogaStyleLength(const std::string_view& str)
	{
		if(str.empty())
			return std::optional<StyleLength>();

		bool percent = false;
		std::string_view str_ = str;
		if (str_.ends_with("%"))
		{
			percent = true;
			str_ = str_.substr(0, str_.length() - 1);
		}

		scalar value;
		auto [ptr, ec] = std::from_chars(str_.data(), str_.data() + str_.size(), value);
		if (ec != std::errc())
			return std::optional<StyleLength>();

		if (percent)
			return StyleLength::percent(value);
		return StyleLength::points(value);
	}

	std::optional<FlexDirection> AsYogaFlexDirection(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<FlexDirection>();

		if (str == "row")
			return FlexDirection::Row;
		else if (str == "column")
			return FlexDirection::Column;
		else if (str == "row-reverse")
			return FlexDirection::RowReverse;
		else if (str == "column-reverse")
			return FlexDirection::ColumnReverse;

		return std::optional<FlexDirection>();
	}
}

namespace nani::canvas::internal
{
	void ComputedStyleBuilder::Load(const pugi::xml_node& styleNode)
	{
		FlexDirection = AsYogaFlexDirection(GetStyleNodeValue(styleNode, "FlexDirection"));
		Width = AsYogaStyleLength(GetStyleNodeValue(styleNode, "Width"));
		Height = AsYogaStyleLength(GetStyleNodeValue(styleNode, "Height"));
		MinWidth = AsYogaStyleLength(GetStyleNodeValue(styleNode, "MinWidth"));
		MinHeight = AsYogaStyleLength(GetStyleNodeValue(styleNode, "MinHeight"));
		MaxWidth = AsYogaStyleLength(GetStyleNodeValue(styleNode, "MaxWidth"));
		MaxHeight = AsYogaStyleLength(GetStyleNodeValue(styleNode, "MaxHeight"));
	}

	void ComputedStyleBuilder::Inherit(const ComputedStyleBuilder * inheritBuilder)
	{
		m_inheritBuilder = inheritBuilder;
	}

	ComputedStyle ComputedStyleBuilder::Compute() const
	{
		ComputedStyle computedStyle;
		Style& styleRef = computedStyle.layoutProps.style;

		if (auto v = ComputeFlexDirection(); v.has_value())
			styleRef.setFlexDirection(v.value());

		if (auto v = ComputeWidth(); v.has_value())
			styleRef.setDimension(Dimension::Width, v.value());

		if (auto v = ComputeWidth(); v.has_value())
			styleRef.setDimension(Dimension::Height, v.value());

		if (auto v = ComputeMinWidth(); v.has_value())
			styleRef.setMinDimension(Dimension::Width, v.value());

		if (auto v = ComputeMinHeight(); v.has_value())
			styleRef.setMinDimension(Dimension::Height, v.value());

		if (auto v = ComputeMaxWidth(); v.has_value())
			styleRef.setMaxDimension(Dimension::Width, v.value());

		if (auto v = ComputeMaxHeight(); v.has_value())
			styleRef.setMaxDimension(Dimension::Height, v.value());

		return computedStyle;
	}
}
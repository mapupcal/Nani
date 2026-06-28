#include "computed_style_builder.h"
#include "../basic/basic_defs.h"

namespace
{
	using namespace nani::canvas::basic;
	using namespace nani::canvas::internal;
	using namespace facebook::yoga;

	struct ParseLengthResult
	{
		bool ok = false;
		bool percent = false;
		scalar value = 0;
	};
	ParseLengthResult _ParseLength(const std::string_view& str)
	{
		ParseLengthResult result;
		std::string_view str_ = str;
		if (str_.ends_with("%"))
		{
			result.percent = true;
			str_ = str_.substr(0, str_.length() - 1);
		}

		scalar value;
		auto [ptr, ec] = std::from_chars(str_.data(), str_.data() + str_.size(), value);
		if (ec != std::errc())
			return result;

		result.ok = true;
		result.value = value;
		return result;
	}

	void _SetStyleDimention(ComputedStyle& style, Dimension dimention, const std::string_view& str)
	{
		auto result = _ParseLength(str);
		if (!result.ok)
			return;
		if (result.percent)
			style.layoutProps.style.setDimension(dimention, Style::Length::percent(result.value));
		else
			style.layoutProps.style.setDimension(dimention, Style::Length::points(result.value));
	}


	std::optional<std::string> GetStyleNodeValue(const pugi::xml_node& node, const std::string_view& name)
	{
		pugi::xml_node childNode = node.child(name);
		if (childNode.empty())
			return std::optional<std::string>();

		pugi::xml_attribute attribute = childNode.attribute("value");
		if (attribute.empty())
			return std::optional<std::string>();

		return attribute.as_string();
	}

	void SetStyleFlexDirection(ComputedStyle& style, const std::string_view& str)
	{
		if (str == "row")
			style.layoutProps.style.setFlexDirection(FlexDirection::Row);
		else if (str == "column")
			style.layoutProps.style.setFlexDirection(FlexDirection::Column);
		else if (str == "row-reverse")
			style.layoutProps.style.setFlexDirection(FlexDirection::RowReverse);
		else if (str == "column-reverse")
			style.layoutProps.style.setFlexDirection(FlexDirection::ColumnReverse);
		else
			NANI_ASSERT(false);
	}

	void SetStyleWidth(ComputedStyle& style, const std::string_view& str)
	{
		_SetStyleDimention(style, Dimension::Width, str);
	}

	void SetStyleHeight(ComputedStyle& style, const std::string_view& str)
	{
		_SetStyleDimention(style, Dimension::Height, str);
	}
}

namespace nani::canvas::internal
{
	void ComputedStyleBuilder::Load(const pugi::xml_node& styleNode)
	{
		FlexDirection = GetStyleNodeValue(styleNode, "FlexDirection");
		Width = GetStyleNodeValue(styleNode, "Width");
		Height = GetStyleNodeValue(styleNode, "Height");
	}

	void ComputedStyleBuilder::Inherit(const ComputedStyleBuilder * inheritBuilder)
	{
		m_inheritBuilder = inheritBuilder;
	}

	ComputedStyle ComputedStyleBuilder::Compute() const
	{
		ComputedStyle style;
		if (auto value = ComputeFlexDirection(); value.has_value())
			SetStyleFlexDirection(style, value.value());
		if (auto value = ComputeWidth(); value.has_value())
			SetStyleWidth(style, value.value());
		if (auto value = ComputeHeight(); value.has_value())
			SetStyleHeight(style, value.value());
		return style;
	}

	std::optional<std::string> ComputedStyleBuilder::ComputeFlexDirection() const
	{
		if (!FlexDirection.has_value() && m_inheritBuilder)
			return m_inheritBuilder->ComputeFlexDirection();

		return FlexDirection;
	}

	std::optional<std::string> ComputedStyleBuilder::ComputeWidth() const
	{
		if (!Width.has_value() && m_inheritBuilder)
			return m_inheritBuilder->ComputeWidth();

		return Width;

	}

	std::optional<std::string> ComputedStyleBuilder::ComputeHeight() const
	{
		if (!Height.has_value() && m_inheritBuilder)
			return m_inheritBuilder->ComputeHeight();

		return Height;
	}
}
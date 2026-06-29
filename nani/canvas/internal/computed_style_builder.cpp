#include "computed_style_builder.h"
#include "../basic/basic_defs.h"

namespace
{
	using namespace nani::canvas::basic;
	using namespace nani::canvas::internal;
	using namespace facebook::yoga;

	std::string GetNodeValue(const pugi::xml_node& node, const std::string_view& name)
	{
		pugi::xml_attribute attribute = node.attribute(name);
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

	std::optional<Direction> AsYogaDirection(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<Direction>();

		if (str == "ltr")
			return Direction::LTR;
		else if (str == "rtl")
			return Direction::RTL;

		return std::optional<Direction>();
	}

	std::optional<scalar> AsScalar(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<scalar>();

		scalar value;
		auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
		if (ec == std::errc())
			return value;

		return std::optional<scalar>();
	}

	std::optional<Color> AsColor(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<Color>();

		Color color(str);
		return color;
	}

	using SetEdgesMemberFuncType = decltype(&Style::setMargin);
	void SetYogaStyleEdges(Style& style, const ComputedStyleBuilder::Edges& edges, SetEdgesMemberFuncType setEdgeFunc)
	{
		if (auto v = edges.Left; v.has_value())
			(style.*setEdgeFunc)(Edge::Left, v.value());
		if (auto v = edges.Top; v.has_value())
			(style.*setEdgeFunc)(Edge::Top, v.value());
		if (auto v = edges.Right; v.has_value())
			(style.*setEdgeFunc)(Edge::Right, v.value());
		if (auto v = edges.Bottom; v.has_value())
			(style.*setEdgeFunc)(Edge::Bottom, v.value());
	}
}

namespace nani::canvas::internal
{
	void ComputedStyleBuilder::Load(const pugi::xml_node& styleNode)
	{
		LoadImpl(styleNode);
	}

	void ComputedStyleBuilder::Inherit(const ComputedStyleBuilder * inheritBuilder)
	{
		m_inheritBuilder = inheritBuilder;
	}

	ComputedStyle ComputedStyleBuilder::Compute() const
	{
		ComputedStyle computedStyle;
		ComputedStyle::LayoutProperties& layoutPropsRef = computedStyle.layoutProps;
		ComputedStyle::VisualProperties& visualPropsRef = computedStyle.visualProps;

		if (auto v = ComputeDirection(); v.has_value())
			layoutPropsRef.style.setDirection(v.value());

		if (auto v = ComputeFlexDirection(); v.has_value())
			layoutPropsRef.style.setFlexDirection(v.value());

		if (auto v = ComputeWidth(); v.has_value())
			layoutPropsRef.style.setDimension(Dimension::Width, v.value());

		if (auto v = ComputeHeight(); v.has_value())
			layoutPropsRef.style.setDimension(Dimension::Height, v.value());

		if (auto v = ComputeMinWidth(); v.has_value())
			layoutPropsRef.style.setMinDimension(Dimension::Width, v.value());

		if (auto v = ComputeMinHeight(); v.has_value())
			layoutPropsRef.style.setMinDimension(Dimension::Height, v.value());

		if (auto v = ComputeMaxWidth(); v.has_value())
			layoutPropsRef.style.setMaxDimension(Dimension::Width, v.value());

		if (auto v = ComputeMaxHeight(); v.has_value())
			layoutPropsRef.style.setMaxDimension(Dimension::Height, v.value());

		if (auto v = RowGap; v.has_value())
			layoutPropsRef.style.setGap(Gutter::Row, v.value());

		if (auto v = ColumnGap; v.has_value())
			layoutPropsRef.style.setGap(Gutter::Column, v.value());

		if (auto v = Margins; v.has_value())
			SetYogaStyleEdges(layoutPropsRef.style, v.value(), &Style::setMargin);

		if (auto v = Paddings; v.has_value())
			SetYogaStyleEdges(layoutPropsRef.style, v.value(), &Style::setPadding);

		if (auto v = Borders; v.has_value())
			SetYogaStyleEdges(layoutPropsRef.style, v.value(), &Style::setBorder);

		if (auto v = ComputeColor(); v.has_value())
			visualPropsRef.color = v.value();

		if (auto v = ComputeBackgroundColor(); v.has_value())
			visualPropsRef.backgroundColor = v.value();

		if (auto v = ComputeBorderColor(); v.has_value())
			visualPropsRef.borderColor = v.value();

		if (auto v = ComputeTransform(); v.has_value())
			visualPropsRef.transform = v.value();

		if (auto v = ComputeOpacity(); v.has_value())
			visualPropsRef.opacity = v.value();

		if (auto v = ComputeRadius(); v.has_value())
			visualPropsRef.radius = v.value();

		if (auto v = ComputeShadow(); v.has_value())
			visualPropsRef.shadow = v.value();

		return computedStyle;
	}

	void ComputedStyleBuilder::LoadImpl(const pugi::xml_node& styleNode)
	{
		if (styleNode.empty())
			return;

		LoadStyleNode(styleNode);

		auto children = styleNode.children();
		for (const auto& child : children)
		{
			std::string name = child.name();
			if (name == "Dimension")
				LoadDimensionNode(child);
			else if (name == "Colors")
				LoadColorsNode(child);
			else if (name == "Radius")
				LoadRadiusNode(child);
			else if (name == "Transform")
				LoadTransformNode(child);
			else if (name == "Shadow")
				LoadShadowNode(child);
			else if (name == "Gaps")
				LoadGapsNode(child);
			else if (name == "Margins")
				LoadEdgesNode(child, Margins);
			else if (name == "Paddings")
				LoadEdgesNode(child, Paddings);
			else if (name == "Borders")
				LoadEdgesNode(child, Borders);
		}
	}

	void ComputedStyleBuilder::LoadStyleNode(const pugi::xml_node& node)
	{
		if (node.empty())
			return;
		auto attributes = node.attributes();
		for (const auto& attribute : attributes)
		{
			std::string name = attribute.name();
			if (name == "flex")
				FlexDirection = AsYogaFlexDirection(attribute.value());
			else if (name == "direction")
				Direction = AsYogaDirection(attribute.value());
		}
	}

	void ComputedStyleBuilder::LoadDimensionNode(const pugi::xml_node& node)
	{
		if (node.empty())
			return;
		auto attributes = node.attributes();
		for (const auto& attribute : attributes)
		{
			std::string name = attribute.name();
			if (name == "width")
				Width = AsYogaStyleLength(attribute.value());
			else if (name == "height")
				Height = AsYogaStyleLength(attribute.value());
			else if (name == "minWidth")
				MinWidth = AsYogaStyleLength(attribute.value());
			else if (name == "minHeight")
				MinHeight = AsYogaStyleLength(attribute.value());
			else if (name == "maxWidth")
				MaxWidth = AsYogaStyleLength(attribute.value());
			else if (name == "maxHeight")
				MaxHeight = AsYogaStyleLength(attribute.value());
		}
	}

	void ComputedStyleBuilder::LoadColorsNode(const pugi::xml_node& node)
	{
		if (node.empty())
			return;
		auto attributes = node.attributes();
		for (const auto& attribute : attributes)
		{
			std::string name = attribute.name();
			if (name == "color")
				Color = AsColor(attribute.value());
			else if (name == "background")
				BackgroundColor = AsColor(attribute.value());
			else if (name == "border")
				BorderColor = AsColor(attribute.value());
			else if (name == "opacity")
				Opacity = AsScalar(attribute.value());
		}
	}

	void ComputedStyleBuilder::LoadRadiusNode(const pugi::xml_node& node)
	{
		if (node.empty())
			return;
		using BorderRadius = ComputedStyle::VisualProperties::BorderRadius;
		BorderRadius radius;
		auto attributes = node.attributes();
		for (const auto& attribute : attributes)
		{
			std::string name = attribute.name();
			if (name == "radius")
			{
				scalar fRadius = AsScalar(attribute.value()).value_or(0.0f);
				radius.topLeft = fRadius;
				radius.topRight = fRadius;
				radius.bottomLeft = fRadius;
				radius.bottomRight = fRadius;
				break;
			}
			else if (name == "tl")
				radius.topLeft = AsScalar(attribute.value()).value_or(0.0f);
			else if (name == "tr")
				radius.topRight = AsScalar(attribute.value()).value_or(0.0f);
			else if (name == "bl")
				radius.bottomLeft = AsScalar(attribute.value()).value_or(0.0f);
			else if (name == "br")
				radius.bottomRight = AsScalar(attribute.value()).value_or(0.0f);
		}

		Radius = radius;
	}

	void ComputedStyleBuilder::LoadTransformNode(const pugi::xml_node& node)
	{
		if (node.empty())
			return;

		TransformF transform;
		auto children = node.children();
		for (const auto& child : children)
		{
			std::string name = child.name();
			if (name == "Translation")
			{
				auto x = AsScalar(GetNodeValue(child, "x"));
				auto y = AsScalar(GetNodeValue(child, "y"));
				transform.Apply(TransformF::Translation(x.value_or(0.0f), y.value_or(0.0f)));
			}
			else if (name == "Rotation")
			{
				auto a = AsScalar(GetNodeValue(child, "a"));
				transform.Apply(TransformF::Rotation(Radian2Degree(a.value_or(0.0f))));
			}
			else if (name == "Scaling")
			{
				auto x = AsScalar(GetNodeValue(child, "x"));
				auto y = AsScalar(GetNodeValue(child, "y"));
				transform.Apply(TransformF::Scaling(x.value_or(0.0f), y.value_or(0.0f)));
			}
			else if (name == "Shearing")
			{
				auto x = AsScalar(GetNodeValue(child, "x"));
				auto y = AsScalar(GetNodeValue(child, "y"));
				transform.Apply(TransformF::Shearing(x.value_or(0.0f), y.value_or(0.0f)));
			}
		}
		 Transform = transform;
	}

	void ComputedStyleBuilder::LoadShadowNode(const pugi::xml_node& node)
	{
		if (node.empty())
			return;

		ComputedStyle::VisualProperties::Shadow shadow;
		auto attributes = node.attributes();
		for (const auto& attribute : attributes)
		{
			std::string name = attribute.name();
			if (name == "color")
				shadow.color = AsColor(attribute.value()).value_or(basic::Color());
			else if (name == "x")
				shadow.offsetX = AsScalar(attribute.value()).value_or(0.0f);
			else if (name == "y")
				shadow.offsetY = AsScalar(attribute.value()).value_or(0.0f);
			else if (name == "b")
				shadow.blur = AsScalar(attribute.value()).value_or(0.0f);
			else if (name == "s")
				shadow.spread = AsScalar(attribute.value()).value_or(0.0f);
		}

		Shadow = shadow;
	}

	void ComputedStyleBuilder::LoadGapsNode(const pugi::xml_node& node)
	{
		if (node.empty())
			return;

		auto attributes = node.attributes();
		for (const auto& attribute : attributes)
		{
			std::string name = attribute.name();
			if (name == "gap")
			{
				auto gap = AsYogaStyleLength(attribute.value());
				RowGap = gap;
				ColumnGap = gap;
				break;
			}
			else if (name == "row")
				RowGap = AsYogaStyleLength(attribute.value());
			else if (name == "column")
				ColumnGap = AsYogaStyleLength(attribute.value());
		}
	}

	void ComputedStyleBuilder::LoadEdgesNode(const pugi::xml_node& node, std::optional<Edges>& OptionalEdges)
	{
		if (node.empty())
			return;

		Edges edges;
		auto attributes = node.attributes();
		for (const auto& attribute : attributes)
		{
			std::string name = attribute.name();
			if (name == "value")
			{
				auto length = AsYogaStyleLength(attribute.value());
				edges.Left = length;
				edges.Top = length;
				edges.Right = length;
				edges.Bottom = length;
				break;
			}
			else if (name == "l")
				edges.Left = AsYogaStyleLength(attribute.value());
			else if (name == "t")
				edges.Top = AsYogaStyleLength(attribute.value());
			else if (name == "r")
				edges.Right = AsYogaStyleLength(attribute.value());
			else if (name == "b")
				edges.Bottom = AsYogaStyleLength(attribute.value());
		}

		OptionalEdges = edges;
	}
}
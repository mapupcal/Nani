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

	std::optional<TransformF> AsTransform(const pugi::xml_node& node)
	{
		if (node.empty())
			return std::optional<TransformF>();

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
		return transform;
	}

	std::optional<ComputedStyle::VisualProperties::BorderRadius> AsBorderRadius(const pugi::xml_node& node)
	{
		using BorderRadius = ComputedStyle::VisualProperties::BorderRadius;
		if (node.empty())
			return std::optional<BorderRadius>();

		// radius
		std::optional<scalar> radius = AsScalar(GetNodeValue(node, "radius"));
		if (radius.has_value())
		{
			scalar fRadius = radius.value();
			return BorderRadius {
				.topLeft = fRadius,
				.topRight = fRadius,
				.bottomLeft = fRadius,
				.bottomRight = fRadius
			};
		}
		// tl, tr, bl, br
		std::optional<scalar> tl = AsScalar(GetNodeValue(node, "tl"));
		std::optional<scalar> tr = AsScalar(GetNodeValue(node, "tr"));
		std::optional<scalar> bl = AsScalar(GetNodeValue(node, "bl"));
		std::optional<scalar> br = AsScalar(GetNodeValue(node, "br"));

		return BorderRadius {
			.topLeft = tl.value_or(0.0f),
			.topRight = tr.value_or(0.0f),
			.bottomLeft = bl.value_or(0.0f),
			.bottomRight = br.value_or(0.0f)
		};
	}

	std::optional<ComputedStyle::VisualProperties::Shadow> AsShadow(const pugi::xml_node& node)
	{
		using Shadow = ComputedStyle::VisualProperties::Shadow;
		if (node.empty())
			return std::optional<Shadow>();

		auto color = AsColor(GetNodeValue(node, "color"));
		auto x = AsScalar(GetNodeValue(node, "x"));
		auto y = AsScalar(GetNodeValue(node, "y"));
		auto b = AsScalar(GetNodeValue(node, "b"));
		auto s = AsScalar(GetNodeValue(node, "s"));

		return Shadow{
			.color = color.value_or(Color()),
			.offsetX = x.value_or(0.0f),
			.offsetY = y.value_or(0.0f),
			.blur = b.value_or(0.0f),
			.spread = s.value_or(0.0f)
		};
	}
}

namespace nani::canvas::internal
{
	void ComputedStyleBuilder::Load(const pugi::xml_node& styleNode)
	{
		FlexDirection = AsYogaFlexDirection(GetNodeValue(styleNode, "flex"));
		Direction = AsYogaDirection(GetNodeValue(styleNode, "direction"));

		pugi::xml_node dimensionNode = styleNode.child("Dimension");
		if (!dimensionNode.empty())
		{
			Width = AsYogaStyleLength(GetNodeValue(dimensionNode, "width"));
			Height = AsYogaStyleLength(GetNodeValue(dimensionNode, "height"));
			MinWidth = AsYogaStyleLength(GetNodeValue(dimensionNode, "minWidth"));
			MinHeight = AsYogaStyleLength(GetNodeValue(dimensionNode, "minHeight"));
			MaxWidth = AsYogaStyleLength(GetNodeValue(dimensionNode, "maxWidth"));
			MaxHeight = AsYogaStyleLength(GetNodeValue(dimensionNode, "maxHeight"));
		}

		pugi::xml_node colorsNode = styleNode.child("Colors");
		if (!colorsNode.empty())
		{
			Color = AsColor(GetNodeValue(colorsNode, "color"));
			BackgroundColor = AsColor(GetNodeValue(colorsNode, "background"));
			BorderColor = AsColor(GetNodeValue(colorsNode, "border"));
			Opacity = AsScalar(GetNodeValue(colorsNode, "opacity"));
		}

		Radius = AsBorderRadius(styleNode.child("Radius"));
		Shadow = AsShadow(styleNode.child("Shadow"));
		Transform = AsTransform(styleNode.child("Transform"));
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
}
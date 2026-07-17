#include "computed_style_builder.h"
#include "../basic/basic_defs.h"

namespace
{
	using namespace nani::canvas::basic;
	using namespace nani::canvas::internal;
	using namespace nani::canvas::text;
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

	std::optional<FloatOptional> AsYogaFloatOptional(const std::string_view& str)
	{
		auto scalar = AsScalar(str);
		if (!scalar.has_value())
			return std::optional<FloatOptional>();
		return FloatOptional(scalar.value());
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

	std::optional<PositionType> AsYogaPositionType(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<PositionType>();

		if (str == "absolute")
			return PositionType::Absolute;
		else if (str == "static")
			return PositionType::Static;
		else if (str == "relative")
			return PositionType::Relative;

		return std::optional<PositionType>();
	}

	std::optional<Justify> AsYogaJustify(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<Justify>();

		if (str == "start")
			return Justify::FlexStart;
		else if (str == "center")
			return Justify::Center;
		else if (str == "end")
			return Justify::FlexEnd;
		else if (str == "space-between")
			return Justify::SpaceBetween;
		else if (str == "space-around")
			return Justify::SpaceAround;
		else if (str == "space-evenly")
			return Justify::SpaceEvenly;

		return std::optional<Justify>();
	}

	std::optional<Align> AsYogaAlign(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<Align>();

		if (str == "auto")
			return Align::Auto;
		else if (str == "start")
			return Align::FlexStart;
		else if (str == "center")
			return Align::Center;
		else if (str == "end")
			return Align::FlexEnd;
		else if (str == "stretch")
			return Align::Stretch;
		else if (str == "baseline")
			return Align::Baseline;
		else if (str == "space-between")
			return Align::SpaceBetween;
		else if (str == "space-around")
			return Align::SpaceAround;
		else if (str == "space-evenly")
			return Align::SpaceEvenly;

		return std::optional<Align>();
	}

	std::optional<Wrap> AsYogaWrap(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<Wrap>();

		if (str == "nowrap")
			return Wrap::NoWrap;
		else if (str == "wrap")
			return Wrap::Wrap;
		else if (str == "wrap-reverse")
			return Wrap::WrapReverse;

		return std::optional<Wrap>();
	}

	std::optional<Overflow> AsYogaOverflow(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<Overflow>();

		if (str == "visible")
			return Overflow::Visible;
		else if (str == "hidden")
			return Overflow::Hidden;
		else if (str == "scroll")
			return Overflow::Scroll;

		return std::optional<Overflow>();
	}

	std::string Trim(const std::string_view& str) {
		auto start = str.find_first_not_of(" \t\n\r\f\v");
		if (start == std::string_view::npos) {
			return {};
		}
		auto end = str.find_last_not_of(" \t\n\r\f\v");
		return std::string(str.substr(start, end - start + 1));
	}

	using TransformOrigin = ComputedStyle::VisualProperties::TransformOrigin;
	TransformOrigin ParseTransformOrigin(const std::string_view& str)
	{
		TransformOrigin origin;
		if (str.empty())
			return origin;

		if (str == "center")
			return origin;
		else if(str == "tl")
			return { StyleLength::percent(0.0f), StyleLength::percent(0.0f) };
		else if(str == "tr")
			return { StyleLength::percent(100.0f), StyleLength::percent(0.0f) };
		else if (str == "bl")
			return { StyleLength::percent(0.0f), StyleLength::percent(100.0f) };
		else if (str == "br")
			return { StyleLength::percent(100.0f), StyleLength::percent(100.0f) };
		else if (auto pos = str.find(','); pos != std::string_view::npos)
		{
			std::string_view xStr = str.substr(0, pos);
			std::string_view yStr = str.substr(pos + 1);
			if (auto x = AsYogaStyleLength(Trim(xStr)); x.has_value())
				origin.x = x.value();
			if (auto y = AsYogaStyleLength(Trim(yStr)); y.has_value())
				origin.y = y.value();
		}
		return origin;
	}

	std::optional<Font::FontStyle> AsFontStyle(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<Font::FontStyle>();
		if (str == "normal")
			return Font::Style::Normal;
		else if (str == "italic")
			return Font::Style::Italic;
		else if (str == "oblique")
			return Font::Style::Oblique;
		return std::optional<Font::FontStyle>();
	}

	std::optional<Font::FontWeight> AsFontWeight(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<Font::FontWeight>();
		if (str == "thin")
			return Font::Weight::Thin;
		else if (str == "extralight")
			return Font::Weight::ExtraLight;
		else if (str == "light")
			return Font::Weight::Light;
		else if (str == "normal")
			return Font::Weight::Normal;
		else if (str == "medium")
			return Font::Weight::Medium;
		else if (str == "semibold")
			return Font::Weight::SemiBold;
		else if (str == "bold")
			return Font::Weight::Bold;
		else if (str == "extrabold")
			return Font::Weight::ExtraBold;
		else if (str == "black")
			return Font::Weight::Black;
		return std::optional<Font::FontWeight>();
	}

	using DecorationLine = TextDecoration::DecorationLine;
	std::optional<DecorationLine> AsDecorationLine(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<DecorationLine>();
		DecorationLine lines = DecorationLine::None;
		std::string_view remaining = str;
		while (!remaining.empty())
		{
			auto pos = remaining.find(',');
			std::string_view lineStr = (pos == std::string_view::npos) ? remaining : remaining.substr(0, pos);
			std::string line = Trim(lineStr);
			if (line == "underline")
				lines = static_cast<DecorationLine>(static_cast<byte>(lines) | static_cast<byte>(DecorationLine::Underline));
			else if (line == "overline")
				lines = static_cast<DecorationLine>(static_cast<byte>(lines) | static_cast<byte>(DecorationLine::Overline));
			else if (line == "linethrough")
				lines = static_cast<DecorationLine>(static_cast<byte>(lines) | static_cast<byte>(DecorationLine::LineThrough));
			if (pos == std::string_view::npos)
				break;
			remaining = remaining.substr(pos + 1);
		}
		return lines;
	}

	using DecorationStyle = TextDecoration::DecorationStyle;
	std::optional<DecorationStyle> AsDecorationStyle(const std::string_view& str)
	{
		if (str.empty())
			return std::optional<DecorationStyle>();
		if (str == "solid")
			return DecorationStyle::Solid;
		else if (str == "double")
			return DecorationStyle::Double;
		else if (str == "dotted")
			return DecorationStyle::Dotted;
		else if (str == "dashed")
			return DecorationStyle::Dashed;
		else if (str == "wavy")
			return DecorationStyle::Wavy;
		return std::optional<DecorationStyle>();
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

		if (auto v = ComputeJustify(); v.has_value())
			layoutPropsRef.style.setJustifyContent(v.value());

		if (auto v = ComputeAlignContent(); v.has_value())
			layoutPropsRef.style.setAlignContent(v.value());

		if (auto v = ComputeAlignItems(); v.has_value())
			layoutPropsRef.style.setAlignItems(v.value());

		if (auto v = ComputeAlignSelf(); v.has_value())
			layoutPropsRef.style.setAlignSelf(v.value());

		if (auto v = ComputeFlexWrap(); v.has_value())
			layoutPropsRef.style.setFlexWrap(v.value());

		if (auto v = ComputeOverflow(); v.has_value())
			layoutPropsRef.style.setOverflow(v.value());

		if (auto v = ComputeFlex(); v.has_value())
			layoutPropsRef.style.setFlex(v.value());

		if (auto v = ComputeFlexGrow(); v.has_value())
			layoutPropsRef.style.setFlexGrow(v.value());

		if (auto v = ComputeFlexShrink(); v.has_value())
			layoutPropsRef.style.setFlexShrink(v.value());

		if (auto v = ComputeFlexBasis(); v.has_value())
			layoutPropsRef.style.setFlexBasis(v.value());

		if (auto v = ComputeAspectRatio(); v.has_value())
			layoutPropsRef.style.setAspectRatio(v.value());

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

		if (auto v = ComputeRowGap(); v.has_value())
			layoutPropsRef.style.setGap(Gutter::Row, v.value());

		if (auto v = ComputeColumnGap(); v.has_value())
			layoutPropsRef.style.setGap(Gutter::Column, v.value());

		if (auto v = ComputeMargins(); v.has_value())
			SetYogaStyleEdges(layoutPropsRef.style, v.value(), &Style::setMargin);

		if (auto v = ComputePaddings(); v.has_value())
			SetYogaStyleEdges(layoutPropsRef.style, v.value(), &Style::setPadding);

		if (auto v = ComputeBorders(); v.has_value())
			SetYogaStyleEdges(layoutPropsRef.style, v.value(), &Style::setBorder);

		if (auto v = ComputePositionType(); v.has_value())
			layoutPropsRef.style.setPositionType(v.value());

		if (auto v = ComputePositions(); v.has_value())
			SetYogaStyleEdges(layoutPropsRef.style, v.value(), &Style::setPosition);

		if (auto v = ComputeFont(); v.has_value())
			visualPropsRef.font = v.value();

		if (auto v = ComputeTextDecoration(); v.has_value())
			visualPropsRef.textDecoration = v.value();

		if (auto v = ComputeColor(); v.has_value())
			visualPropsRef.color = v.value();

		if (auto v = ComputeBackgroundColor(); v.has_value())
			visualPropsRef.backgroundColor = v.value();

		if (auto v = ComputeBorderColor(); v.has_value())
			visualPropsRef.borderColor = v.value();

		if (auto v = ComputeTransform(); v.has_value())
			visualPropsRef.transform = v.value();

		if (auto v = ComputeTransformOrigin(); v.has_value())
			visualPropsRef.transformOrigin = v.value();

		if (auto v = ComputeOpacity(); v.has_value())
			visualPropsRef.opacity = v.value();

		if (auto v = ComputeRadius(); v.has_value())
			visualPropsRef.radius = v.value();

		if (auto v = ComputeShadow(); v.has_value())
			visualPropsRef.shadow = v.value();

		return computedStyle;
	}

	void ComputedStyleBuilder::CopyFrom(const ComputedStyleBuilder& other)
	{
		*this = other;
	}

	void ComputedStyleBuilder::LoadImpl(const pugi::xml_node& styleNode)
	{
		if (styleNode.empty())
			return;

		auto children = styleNode.children();
		for (const auto& child : children)
		{
			std::string name = child.name();
			if (name == "FlexBox")
				LoadFlexBoxNode(child);
			else if (name == "Dimension")
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
			else if (name == "Positions")
				LoadPositionsNode(child);
			else if (name == "Font")
				LoadFontNode(child);
			else if (name == "TextDecoration")
				LoadTextDecorationNode(child);
		}
	}

	void ComputedStyleBuilder::LoadFlexBoxNode(const pugi::xml_node& node)
	{
		if (node.empty())
			return;
		auto attributes = node.attributes();
		for (const auto& attribute : attributes)
		{
			std::string name = attribute.name();
			if (name == "flex")
				Flex = AsYogaFloatOptional(attribute.value());
			else if (name == "flexDirection")
				FlexDirection = AsYogaFlexDirection(attribute.value());
			else if (name == "direction")
				Direction = AsYogaDirection(attribute.value());
			else if (name == "grow")
				FlexGrow = AsYogaFloatOptional(attribute.value());
			else if (name == "shrink")
				FlexShrink = AsYogaFloatOptional(attribute.value());
			else if (name == "basis")
				FlexBasis = AsYogaStyleLength(attribute.value());
			else if (name == "wrap")
				FlexWrap = AsYogaWrap(attribute.value());
			else if (name == "justify")
				Justify = AsYogaJustify(attribute.value());
			else if (name == "alignContent")
				AlignContent = AsYogaAlign(attribute.value());
			else if (name == "alignItems")
				AlignItems = AsYogaAlign(attribute.value());
			else if (name == "alignSelf")
				AlignSelf = AsYogaAlign(attribute.value());
			else if (name == "overflow")
				Overflow = AsYogaOverflow(attribute.value());
			else if (name == "aspectRatio")
				AspectRatio = AsYogaFloatOptional(attribute.value());
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

		TransformOrigin = ParseTransformOrigin(GetNodeValue(node, "origin"));

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
				transform.Apply(TransformF::Rotation(Degree2Radian(a.value_or(0.0f))));
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

	void ComputedStyleBuilder::LoadPositionsNode(const pugi::xml_node& node)
	{
		if (node.empty())
			return;
		PositionType = AsYogaPositionType(GetNodeValue(node, "type"));
		LoadEdgesNode(node, Positions);
	}

	void ComputedStyleBuilder::LoadFontNode(const pugi::xml_node& node)
	{
		if (node.empty())
			return;

		text::Font font;
		auto attributes = node.attributes();
		for (const auto& attribute : attributes)
		{
			std::string name = attribute.name();
			if (name == "family")
			{
				std::string family = attribute.value();
				font.SetFamily(std::u8string(family.cbegin(), family.cend()));
			}
			else if (name == "size")
			{
				auto size = AsScalar(attribute.value());
				if (size.has_value())
					font.SetSize(size.value());
			}
			else if (name == "style")
			{
				auto style = AsFontStyle(attribute.value());
				if (style.has_value())
					font.SetStyle(style.value());
			}
			else if (name == "weight")
			{
				auto weight = AsFontWeight(attribute.value());
				if (weight.has_value())
					font.SetWeight(weight.value());
			}
		}
		Font = font;
	}

	void ComputedStyleBuilder::LoadTextDecorationNode(const pugi::xml_node& node)
	{
		if (node.empty())
			return;

		text::TextDecoration textDecoration;
		auto attributes = node.attributes();
		for (const auto& attribute : attributes)
		{
			std::string name = attribute.name();
			if (name == "line")
			{
				auto line = AsDecorationLine(attribute.value());
				if (line.has_value())
					textDecoration.SetLines(line.value());
			}
			else if (name == "style")
			{
				auto style = AsDecorationStyle(attribute.value());
				if (style.has_value())
					textDecoration.SetStyle(style.value());
			}
			else if (name == "color")
			{
				auto color = AsColor(attribute.value());
				if (color.has_value())
					textDecoration.SetColor(color.value());
			}
		}

		TextDecoration = textDecoration;
	}
}
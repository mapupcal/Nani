#pragma once
#include "computed_style.h"
#include <pugixml.hpp>
#include <yoga/style/Style.h>

#define ComputeStyleBuilderInheritProperty(value_class, value_name)			\
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
		void CopyFrom(const ComputedStyleBuilder& other);

	public:
		//Layout
		ComputeStyleBuilderInheritProperty(facebook::yoga::Direction, Direction);
		ComputeStyleBuilderInheritProperty(facebook::yoga::FlexDirection, FlexDirection);
		ComputeStyleBuilderInheritProperty(facebook::yoga::FloatOptional, Flex);
		ComputeStyleBuilderInheritProperty(facebook::yoga::FloatOptional, FlexGrow);
		ComputeStyleBuilderInheritProperty(facebook::yoga::FloatOptional, FlexShrink);
		ComputeStyleBuilderInheritProperty(facebook::yoga::FloatOptional, AspectRatio);
		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, FlexBasis);
		ComputeStyleBuilderInheritProperty(facebook::yoga::Wrap, FlexWrap);
		ComputeStyleBuilderInheritProperty(facebook::yoga::Justify, Justify);
		ComputeStyleBuilderInheritProperty(facebook::yoga::Align, AlignContent);
		ComputeStyleBuilderInheritProperty(facebook::yoga::Align, AlignItems);
		ComputeStyleBuilderInheritProperty(facebook::yoga::Align, AlignSelf);
		ComputeStyleBuilderInheritProperty(facebook::yoga::Overflow, Overflow);

		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, Width);
		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, Height);
		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, MinWidth);
		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, MinHeight);
		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, MaxWidth);
		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, MaxHeight);

		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, RowGap);
		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, ColumnGap);
		struct Edges
		{
			std::optional<facebook::yoga::StyleLength> Left;
			std::optional<facebook::yoga::StyleLength> Top;
			std::optional<facebook::yoga::StyleLength> Right;
			std::optional<facebook::yoga::StyleLength> Bottom;
		};
		ComputeStyleBuilderInheritProperty(Edges, Margins);
		ComputeStyleBuilderInheritProperty(Edges, Paddings);
		ComputeStyleBuilderInheritProperty(Edges, Borders);

		ComputeStyleBuilderInheritProperty(facebook::yoga::PositionType, PositionType);
		ComputeStyleBuilderInheritProperty(Edges, Positions);
		//Visual
		ComputeStyleBuilderInheritProperty(basic::Color, Color);
		ComputeStyleBuilderInheritProperty(basic::Color, BackgroundColor);
		ComputeStyleBuilderInheritProperty(basic::Color, BorderColor);
		ComputeStyleBuilderInheritProperty(ComputedStyle::VisualProperties::TransformOrigin, TransformOrigin);
		ComputeStyleBuilderInheritProperty(basic::TransformF, Transform);
		ComputeStyleBuilderInheritProperty(basic::scalar, Opacity);
		ComputeStyleBuilderInheritProperty(ComputedStyle::VisualProperties::BorderRadius, Radius);
		ComputeStyleBuilderInheritProperty(ComputedStyle::VisualProperties::Shadow, Shadow);

	private:
		void LoadImpl(const pugi::xml_node& styleNode);
		void LoadFlexBoxNode(const pugi::xml_node& node);
		void LoadDimensionNode(const pugi::xml_node& node);
		void LoadColorsNode(const pugi::xml_node& node);
		void LoadRadiusNode(const pugi::xml_node& node);
		void LoadTransformNode(const pugi::xml_node& node);
		void LoadShadowNode(const pugi::xml_node& node);
		void LoadGapsNode(const pugi::xml_node& node);
		void LoadEdgesNode(const pugi::xml_node& node, std::optional<Edges>& OptionalEdges);
		void LoadPositionsNode(const pugi::xml_node& node);

	private:
		const ComputedStyleBuilder* m_inheritBuilder = nullptr;
	};
}
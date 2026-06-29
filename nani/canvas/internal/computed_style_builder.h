#pragma once
#include "computed_style.h"
#include <pugixml.hpp>
#include <yoga/style/Style.h>

#define ComputeStyleBuilderProperty(value_class, value_name)				\
	std::optional<value_class> value_name									\

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

	public:
		//Layout
		ComputeStyleBuilderInheritProperty(facebook::yoga::Direction, Direction);
		ComputeStyleBuilderInheritProperty(facebook::yoga::FlexDirection, FlexDirection);

		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, Width);
		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, Height);
		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, MinWidth);
		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, MinHeight);
		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, MaxWidth);
		ComputeStyleBuilderInheritProperty(facebook::yoga::StyleLength, MaxHeight);

		ComputeStyleBuilderProperty(facebook::yoga::StyleLength, RowGap);
		ComputeStyleBuilderProperty(facebook::yoga::StyleLength, ColumnGap);
		struct Edges
		{
			std::optional<facebook::yoga::StyleLength> Left;
			std::optional<facebook::yoga::StyleLength> Top;
			std::optional<facebook::yoga::StyleLength> Right;
			std::optional<facebook::yoga::StyleLength> Bottom;
		};
		ComputeStyleBuilderProperty(Edges, Margins);
		ComputeStyleBuilderProperty(Edges, Paddings);
		ComputeStyleBuilderProperty(Edges, Borders);
		//Visual
		ComputeStyleBuilderInheritProperty(basic::Color, Color);
		ComputeStyleBuilderInheritProperty(basic::Color, BackgroundColor);
		ComputeStyleBuilderInheritProperty(basic::Color, BorderColor);
		ComputeStyleBuilderInheritProperty(basic::TransformF, Transform);
		ComputeStyleBuilderInheritProperty(basic::scalar, Opacity);
		ComputeStyleBuilderInheritProperty(ComputedStyle::VisualProperties::BorderRadius, Radius);
		ComputeStyleBuilderInheritProperty(ComputedStyle::VisualProperties::Shadow, Shadow);

	private:
		void LoadImpl(const pugi::xml_node& styleNode);
		void LoadStyleNode(const pugi::xml_node& node);
		void LoadDimensionNode(const pugi::xml_node& node);
		void LoadColorsNode(const pugi::xml_node& node);
		void LoadRadiusNode(const pugi::xml_node& node);
		void LoadTransformNode(const pugi::xml_node& node);
		void LoadShadowNode(const pugi::xml_node& node);
		void LoadGapsNode(const pugi::xml_node& node);
		void LoadEdgesNode(const pugi::xml_node& node, std::optional<Edges>& OptionalEdges);

	private:
		const ComputedStyleBuilder* m_inheritBuilder = nullptr;
	};
}
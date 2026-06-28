#pragma once
#include "computed_style.h"
#include <pugixml.hpp>

namespace nani::canvas::internal
{
	class ComputedStyleBuilder
	{
	public:
		void Load(const pugi::xml_node& styleNode);
		void Inherit(const ComputedStyleBuilder* inheritBuilder);
		ComputedStyle Compute() const;

	public:
		std::optional<std::string> FlexDirection;
		std::optional<std::string> Width;
		std::optional<std::string> Height;

		std::optional<std::string> ComputeFlexDirection() const;
		std::optional<std::string> ComputeWidth() const;
		std::optional<std::string> ComputeHeight() const;

	private:
		const ComputedStyleBuilder* m_inheritBuilder = nullptr;
	};
}
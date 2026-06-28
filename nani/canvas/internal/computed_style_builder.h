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

	private:
		const ComputedStyleBuilder* m_inheritBuilder = nullptr;
	};
}
#include "computed_style_builder.h"

namespace nani::canvas::internal
{
	void ComputedStyleBuilder::Load(const pugi::xml_node& styleNode)
	{

	}

	void ComputedStyleBuilder::Inherit(const ComputedStyleBuilder * inheritBuilder)
	{
		m_inheritBuilder = inheritBuilder;
	}

	ComputedStyle ComputedStyleBuilder::Compute() const
	{
		return ComputedStyle();
	}
}
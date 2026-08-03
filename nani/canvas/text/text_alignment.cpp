#include "text_alignment.h"

namespace nani::canvas::text
{
	TextAlignment::Horizontal TextAlignment::HorizontalAlign() const
	{
		return m_horizontal;
	}

	void TextAlignment::SetHorizontal(Horizontal align)
	{
		m_horizontal = align;
	}

	TextAlignment::Vertical TextAlignment::VerticalAlign() const
	{
		return m_vertical;
	}

	void TextAlignment::SetVertical(Vertical align)
	{
		m_vertical = align;
	}
}

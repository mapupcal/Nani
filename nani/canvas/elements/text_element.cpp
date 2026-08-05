#include "text_element.h"
#include "../visuals/text_visual.h"
#include "../events/event.h"

using namespace nani::canvas::visuals;
using namespace nani::canvas::events;

namespace nani::canvas::elements
{
	TextElement::TextElement(Element* parent, const std::u8string_view& text)
		: Element(parent)
	{
		SetStyleClass(u8"DefaultText");
		m_text = text;
	}

	void TextElement::SetText(const std::u8string_view& text)
	{
		if (m_text == text)
			return;

		m_text = text;
		ElementTextChangedEvent event(this);
		FireEvent(&event);
	}

	const std::u8string_view TextElement::Text() const
	{
		return m_text;
	}

	void TextElement::SetElideMode(text::TextElideMode mode)
	{
		if (m_elideMode == mode)
			return;

		m_elideMode = mode;
		ElementTextChangedEvent event(this);
		FireEvent(&event);
	}

	text::TextElideMode TextElement::ElideMode() const
	{
		return m_elideMode;
	}

	void TextElement::SetWrapMode(text::TextWrapMode mode)
	{
		if (m_wrapMode == mode)
			return;

		m_wrapMode = mode;
		ElementTextChangedEvent event(this);
		FireEvent(&event);
	}

	text::TextWrapMode TextElement::WrapMode() const
	{
		return m_wrapMode;
	}

	std::shared_ptr<Visual> TextElement::CreateVisual(View* view, Visual* visualParent)
	{
		return std::make_shared<TextVisual>(view, this, visualParent);
	}
}

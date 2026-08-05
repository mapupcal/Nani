#pragma once
#include "elements_defs.h"
#include "element.h"
#include "../text/text_defs.h"

namespace nani::canvas::elements
{
	class NANI_CANVAS_API TextElement : public Element
	{
	public:
		TextElement(Element* parent, const std::u8string_view& text = {});
		TextElement(const TextElement&) = delete;
		~TextElement() override = default;

	public:
		void SetText(const std::u8string_view& text);
		const std::u8string_view Text() const;

		void SetElideMode(text::TextElideMode mode);
		text::TextElideMode ElideMode() const;

		void SetWrapMode(text::TextWrapMode mode);
		text::TextWrapMode WrapMode() const;

		std::shared_ptr<visuals::Visual> CreateVisual(
			visuals::View* view,
			visuals::Visual* visualParent) override;

	private:
		std::u8string m_text;
		text::TextElideMode m_elideMode = text::TextElideMode::Right;
		text::TextWrapMode m_wrapMode = text::TextWrapMode::NoWrap;
	};
}

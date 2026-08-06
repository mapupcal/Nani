#pragma once
#include "elements_defs.h"
#include "element.h"

namespace nani::canvas::elements
{
	class NANI_CANVAS_API InputTextElement : public Element
	{
	public:
		explicit InputTextElement(Element* parent, const std::u8string_view& text = {});
		InputTextElement(const InputTextElement&) = delete;
		~InputTextElement() override = default;

	public:
		void SetText(const std::u8string_view& text);
		const std::u8string_view Text() const;

		void SetCaretIndex(size_t index);
		size_t CaretIndex() const;

		const std::u8string_view PreeditText() const;
		bool IsComposing() const;
		void EndComposition();

		std::shared_ptr<visuals::Visual> CreateVisual(
			visuals::View* view,
			visuals::Visual* visualParent) override;

	protected:
		void OnEvent(events::Event* e) override;

	private:
		void NotifyTextChanged();
		void ClearCompositionState(bool notify);
		void InsertUtf8(const std::u8string_view& utf8);
		void DeleteBackward();
		void DeleteForward();
		void MoveCaretLeft();
		void MoveCaretRight();
		size_t ClampCaret(size_t index) const;

	private:
		std::u8string m_text;
		std::u8string m_preedit;
		size_t m_caret = 0;
		bool m_composing = false;
	};
}

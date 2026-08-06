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

		void SetMultiLine(bool multiLine);
		bool IsMultiLine() const;

		void SetPasswordMode(bool password);
		bool IsPasswordMode() const;
		void SetPasswordVisible(bool visible);
		bool IsPasswordVisible() const;
		void SetPasswordEcho(const std::u8string_view& echo);
		const std::u8string_view PasswordEcho() const;

		void SetCaretIndex(size_t index);
		size_t CaretIndex() const;

		void SetSelection(size_t anchor, size_t caret);
		size_t AnchorIndex() const;
		size_t SelectionStart() const;
		size_t SelectionEnd() const;
		bool HasSelection() const;
		void ClearSelection();
		void SelectAll();

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
		void DeleteSelection();
		void DeleteBackward();
		void DeleteForward();
		void MoveCaretLeft(bool extend);
		void MoveCaretRight(bool extend);
		void MoveCaretTo(size_t index, bool extend);
		size_t ClampCaret(size_t index) const;
		static bool HasShift(events::Modifier modifier);
		static bool HasCtrl(events::Modifier modifier);

	private:
		std::u8string m_text;
		std::u8string m_preedit;
		size_t m_anchor = 0;
		size_t m_caret = 0;
		bool m_composing = false;
		bool m_multiLine = false;
		bool m_passwordMode = false;
		bool m_passwordVisible = false;
		std::u8string m_passwordEcho;
	};
}

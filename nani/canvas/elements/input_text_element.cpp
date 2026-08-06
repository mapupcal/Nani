#include "input_text_element.h"
#include "../visuals/input_text_visual.h"
#include "../events/event.h"
#include "../text/utf8.h"

using namespace nani::canvas::visuals;
using namespace nani::canvas::events;
using namespace nani::canvas::text;

namespace nani::canvas::elements
{
	InputTextElement::InputTextElement(Element* parent, const std::u8string_view& text)
		: Element(parent)
		, m_text(text)
		, m_anchor(text.size())
		, m_caret(text.size())
	{
		SetStyleClass(u8"DefaultInputText");
	}

	void InputTextElement::SetText(const std::u8string_view& text)
	{
		if (m_text == text && m_preedit.empty() && !m_composing)
		{
			const size_t clamped = ClampCaret(m_caret);
			if (m_caret == clamped && m_anchor == clamped)
				return;
		}

		m_text = text;
		m_preedit.clear();
		m_composing = false;
		m_caret = ClampCaret(m_caret);
		m_anchor = m_caret;
		NotifyTextChanged();
	}

	const std::u8string_view InputTextElement::Text() const
	{
		return m_text;
	}

	void InputTextElement::SetCaretIndex(size_t index)
	{
		SetSelection(index, index);
	}

	size_t InputTextElement::CaretIndex() const
	{
		return m_caret;
	}

	void InputTextElement::SetSelection(size_t anchor, size_t caret)
	{
		const size_t clampedAnchor = ClampCaret(anchor);
		const size_t clampedCaret = ClampCaret(caret);
		if (m_anchor == clampedAnchor && m_caret == clampedCaret)
			return;

		m_anchor = clampedAnchor;
		m_caret = clampedCaret;
		NotifyTextChanged();
	}

	size_t InputTextElement::AnchorIndex() const
	{
		return m_anchor;
	}

	size_t InputTextElement::SelectionStart() const
	{
		return std::min(m_anchor, m_caret);
	}

	size_t InputTextElement::SelectionEnd() const
	{
		return std::max(m_anchor, m_caret);
	}

	bool InputTextElement::HasSelection() const
	{
		return m_anchor != m_caret;
	}

	void InputTextElement::ClearSelection()
	{
		if (!HasSelection())
			return;
		m_anchor = m_caret;
		NotifyTextChanged();
	}

	void InputTextElement::SelectAll()
	{
		SetSelection(0, m_text.size());
	}

	const std::u8string_view InputTextElement::PreeditText() const
	{
		return m_preedit;
	}

	bool InputTextElement::IsComposing() const
	{
		return m_composing;
	}

	void InputTextElement::EndComposition()
	{
		ClearCompositionState(true);
	}

	std::shared_ptr<Visual> InputTextElement::CreateVisual(View* view, Visual* visualParent)
	{
		return std::make_shared<InputTextVisual>(view, this, visualParent);
	}

	bool InputTextElement::HasShift(Modifier modifier)
	{
		return (modifier & Modifier::Shift) != Modifier::None;
	}

	void InputTextElement::OnEvent(Event* e)
	{
		switch (e->type)
		{
		case Type::Char:
		{
			auto* charEvent = static_cast<CharEvent*>(e);
			if (charEvent->codepoint < 0x20 && charEvent->codepoint != U'\t')
				return;

			ClearCompositionState(false);
			InsertUtf8(utf8::Encode(charEvent->codepoint));
			return;
		}
		case Type::KeyPress:
		{
			if (m_composing)
				return;

			auto* keyEvent = static_cast<KeyPressEvent*>(e);
			const bool extend = HasShift(keyEvent->modifier);
			const bool ctrl = (keyEvent->modifier & Modifier::Ctrl) != Modifier::None;

			if (ctrl && (keyEvent->key == Key::A))
			{
				SelectAll();
				return;
			}

			switch (keyEvent->key)
			{
			case Key::Backspace:
				DeleteBackward();
				return;
			case Key::Delete:
				DeleteForward();
				return;
			case Key::Left:
				MoveCaretLeft(extend);
				return;
			case Key::Right:
				MoveCaretRight(extend);
				return;
			case Key::Home:
				MoveCaretTo(0, extend);
				return;
			case Key::End:
				MoveCaretTo(m_text.size(), extend);
				return;
			default:
				break;
			}
			break;
		}
		case Type::ImeCompositionStart:
		{
			ClearSelection();
			m_composing = true;
			return;
		}
		case Type::ImeCompositionUpdate:
		{
			auto* update = static_cast<ImeCompositionUpdateEvent*>(e);
			ClearSelection();
			m_composing = true;
			if (m_preedit == update->preedit)
				return;
			m_preedit = update->preedit;
			NotifyTextChanged();
			return;
		}
		case Type::ImeCompositionEnd:
		{
			EndComposition();
			return;
		}
		default:
			break;
		}

		Element::OnEvent(e);
	}

	void InputTextElement::NotifyTextChanged()
	{
		ElementTextChangedEvent event(this);
		FireEvent(&event);
	}

	void InputTextElement::ClearCompositionState(bool notify)
	{
		if (!m_composing && m_preedit.empty())
			return;

		m_composing = false;
		m_preedit.clear();
		if (notify)
			NotifyTextChanged();
	}

	void InputTextElement::InsertUtf8(const std::u8string_view& utf8)
	{
		if (utf8.empty())
			return;

		DeleteSelection();
		m_text.insert(m_caret, utf8);
		m_caret += utf8.size();
		m_anchor = m_caret;
		NotifyTextChanged();
	}

	void InputTextElement::DeleteSelection()
	{
		if (!HasSelection())
			return;

		const size_t start = SelectionStart();
		const size_t end = SelectionEnd();
		m_text.erase(start, end - start);
		m_anchor = start;
		m_caret = start;
	}

	void InputTextElement::DeleteBackward()
	{
		if (HasSelection())
		{
			DeleteSelection();
			NotifyTextChanged();
			return;
		}

		if (m_caret == 0)
			return;

		const size_t prev = utf8::PrevIndex(m_text, m_caret);
		m_text.erase(prev, m_caret - prev);
		m_caret = prev;
		m_anchor = m_caret;
		NotifyTextChanged();
	}

	void InputTextElement::DeleteForward()
	{
		if (HasSelection())
		{
			DeleteSelection();
			NotifyTextChanged();
			return;
		}

		if (m_caret >= m_text.size())
			return;

		const size_t next = utf8::NextIndex(m_text, m_caret);
		m_text.erase(m_caret, next - m_caret);
		m_anchor = m_caret;
		NotifyTextChanged();
	}

	void InputTextElement::MoveCaretLeft(bool extend)
	{
		if (!extend && HasSelection())
		{
			SetCaretIndex(SelectionStart());
			return;
		}

		MoveCaretTo(utf8::PrevIndex(m_text, m_caret), extend);
	}

	void InputTextElement::MoveCaretRight(bool extend)
	{
		if (!extend && HasSelection())
		{
			SetCaretIndex(SelectionEnd());
			return;
		}

		MoveCaretTo(utf8::NextIndex(m_text, m_caret), extend);
	}

	void InputTextElement::MoveCaretTo(size_t index, bool extend)
	{
		const size_t clamped = ClampCaret(index);
		if (extend)
			SetSelection(m_anchor, clamped);
		else
			SetSelection(clamped, clamped);
	}

	size_t InputTextElement::ClampCaret(size_t index) const
	{
		return utf8::AlignBoundary(m_text, std::min(index, m_text.size()));
	}
}

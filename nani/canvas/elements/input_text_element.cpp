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
		, m_caret(text.size())
	{
		SetStyleClass(u8"DefaultInputText");
	}

	void InputTextElement::SetText(const std::u8string_view& text)
	{
		if (m_text == text && m_preedit.empty() && !m_composing)
			return;

		m_text = text;
		m_preedit.clear();
		m_composing = false;
		m_caret = ClampCaret(m_caret);
		NotifyTextChanged();
	}

	const std::u8string_view InputTextElement::Text() const
	{
		return m_text;
	}

	void InputTextElement::SetCaretIndex(size_t index)
	{
		const size_t clamped = ClampCaret(index);
		if (m_caret == clamped)
			return;

		m_caret = clamped;
		NotifyTextChanged();
	}

	size_t InputTextElement::CaretIndex() const
	{
		return m_caret;
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
			switch (keyEvent->key)
			{
			case Key::Backspace:
				DeleteBackward();
				return;
			case Key::Delete:
				DeleteForward();
				return;
			case Key::Left:
				MoveCaretLeft();
				return;
			case Key::Right:
				MoveCaretRight();
				return;
			case Key::Home:
				SetCaretIndex(0);
				return;
			case Key::End:
				SetCaretIndex(m_text.size());
				return;
			default:
				break;
			}
			break;
		}
		case Type::ImeCompositionStart:
		{
			m_composing = true;
			return;
		}
		case Type::ImeCompositionUpdate:
		{
			auto* update = static_cast<ImeCompositionUpdateEvent*>(e);
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

		m_text.insert(m_caret, utf8);
		m_caret += utf8.size();
		NotifyTextChanged();
	}

	void InputTextElement::DeleteBackward()
	{
		if (m_caret == 0)
			return;

		const size_t prev = utf8::PrevIndex(m_text, m_caret);
		m_text.erase(prev, m_caret - prev);
		m_caret = prev;
		NotifyTextChanged();
	}

	void InputTextElement::DeleteForward()
	{
		if (m_caret >= m_text.size())
			return;

		const size_t next = utf8::NextIndex(m_text, m_caret);
		m_text.erase(m_caret, next - m_caret);
		NotifyTextChanged();
	}

	void InputTextElement::MoveCaretLeft()
	{
		SetCaretIndex(utf8::PrevIndex(m_text, m_caret));
	}

	void InputTextElement::MoveCaretRight()
	{
		SetCaretIndex(utf8::NextIndex(m_text, m_caret));
	}

	size_t InputTextElement::ClampCaret(size_t index) const
	{
		return utf8::AlignBoundary(m_text, std::min(index, m_text.size()));
	}
}

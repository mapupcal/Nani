#pragma once
#include "visuals_defs.h"
#include "visual.h"
#include "../timer.h"

namespace nani::canvas::elements
{
	class InputTextElement;
}

namespace nani::canvas::visuals
{
	class NANI_CANVAS_API InputTextVisual : public Visual
	{
	public:
		InputTextVisual(visuals::View* view, elements::InputTextElement* element, Visual* parent);
		~InputTextVisual() override;

	public:
		void BuildVisuals() override;
		void PaintOverride(SkCanvas* canvas, const basic::RectF& dirtyRect) override;
		bool Filter(events::EventTarget* target, events::Event* e) override;

		basic::single ScrollOffset() const;

	private:
		elements::InputTextElement* InputText() const;
		basic::RectF LocalContentRect() const;
		void SyncImeCaretRect();
		void SyncCaretBlink(bool focused);
		void ResetCaretBlink();
		void OnBlinkTimeout();
		void EnsureCaretVisible();
		size_t CaretIndexAtLocalX(basic::single localX) const;
		void HandleMousePress(events::MousePressEvent* e);
		void HandleMouseMove(events::MouseMoveEvent* e);
		void HandleMouseRelease(events::MouseReleaseEvent* e);

	private:
		Timer m_blinkTimer;
		basic::single m_scrollX = 0.0f;
		bool m_caretVisible = true;
		bool m_dragging = false;
	};
}

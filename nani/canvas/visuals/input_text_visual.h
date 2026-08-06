#pragma once
#include "visuals_defs.h"
#include "visual.h"

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
		~InputTextVisual() override = default;

	public:
		void BuildVisuals() override;
		void PaintOverride(SkCanvas* canvas, const basic::RectF& dirtyRect) override;
		bool Filter(events::EventTarget* target, events::Event* e) override;

	private:
		elements::InputTextElement* InputText() const;
		basic::RectF LocalContentRect() const;
		void SyncImeCaretRect();
	};
}

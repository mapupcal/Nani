#include "input_text_visual.h"
#include "view.h"
#include "../elements/input_text_element.h"
#include "../elements/element_states.h"
#include "../events/event.h"
#include "../styles.h"
#include "../window.h"
#include "../text/font_metrics.h"
#include "../text/text_alignment.h"
#include "../text/utf8.h"
#include "../internal/computed_style.h"
#include "../internal/skia_defs.h"
#include "../internal/skia_utils.h"
#include "../internal/text_paint_utils.h"
#include "../internal/yoga_defs.h"
#include "../internal/yoga_utils.h"

using namespace nani::canvas::elements;
using namespace nani::canvas::events;
using namespace nani::canvas::basic;
using namespace nani::canvas::text;
using namespace nani::canvas::internal;

namespace nani::canvas::visuals
{
	namespace
	{
		constexpr dword kBlinkIntervalMs = 530;
		const Color kDefaultSelectionBackground(0x3B, 0x82, 0xF6, 0x80);

		struct InputTextLayout
		{
			RectF contentRect;
			float textX = 0.0f;
			float baselineY = 0.0f;
			float ascent = 0.0f;
			float descent = 0.0f;
			float prefixWidth = 0.0f;
			float preeditWidth = 0.0f;
			std::u8string_view prefix;
			std::u8string_view preedit;
			std::u8string_view suffix;
		};

		bool ResolveInputTextLayout(
			InputTextElement* input,
			const ComputedStyle* style,
			const RectF& contentRect,
			basic::single scrollX,
			InputTextLayout& out)
		{
			if (!input || !style)
				return false;
			if (contentRect.Width() <= 0.0f || contentRect.Height() <= 0.0f)
				return false;

			FontMetrics metrics(style->visualProps.font);
			const std::u8string_view text = input->Text();
			const size_t caret = input->CaretIndex();

			out.contentRect = contentRect;
			out.prefix = text.substr(0, caret);
			out.preedit = input->PreeditText();
			out.suffix = text.substr(caret);
			out.prefixWidth = metrics.HorizontalAdvance(out.prefix);
			out.preeditWidth = metrics.HorizontalAdvance(out.preedit);
			out.ascent = metrics.Ascent();
			out.descent = metrics.Descent();

			const float lineHeight = metrics.LineHeight();
			const float textTop = text_paint_utils::AlignedBlockTop(
				contentRect,
				lineHeight,
				style->visualProps.textAlignment.VerticalAlign());

			out.textX = contentRect.left - scrollX;
			out.baselineY = textTop + out.ascent;
			return true;
		}

		YGSize MeasureInputTextVisual(
			YGNodeConstRef node,
			float width,
			YGMeasureMode widthMode,
			float height,
			YGMeasureMode heightMode)
		{
			auto* visual = static_cast<InputTextVisual*>(YGNodeGetContext(node));
			auto* input = visual ? static_cast<InputTextElement*>(visual->Element()) : nullptr;
			if (!input)
				return { 0.0f, 0.0f };

			Styles* styles = input->GetStyles();
			if (!styles)
				return { 0.0f, 0.0f };

			std::shared_ptr<ComputedStyle> style = styles->Compute(
				input->StyleClass(),
				input->States()->GetStateProps());
			if (!style)
				return { 0.0f, 0.0f };

			FontMetrics metrics(style->visualProps.font);
			std::u8string display(input->Text());
			display.insert(input->CaretIndex(), input->PreeditText());

			const float contentWidth = metrics.HorizontalAdvance(display);
			const float contentHeight = metrics.LineHeight();
			return {
				yoga_utils::ResolveMeasuredSize(contentWidth, widthMode, width),
				yoga_utils::ResolveMeasuredSize(contentHeight, heightMode, height)
			};
		}

		PointF MapLocalToRoot(Visual* visual, PointF local)
		{
			for (Visual* current = visual; current; )
			{
				local = current->Transform().ApplyTo(local);
				Visual* parent = current->Parent();
				if (!parent)
					break;
				local += current->LayoutRect().TopLeft();
				current = parent;
			}
			return local;
		}

		Color ResolveSelectionBackground(const ComputedStyle* style)
		{
			if (style && style->visualProps.selectionBackground.a != 0)
				return style->visualProps.selectionBackground;
			return kDefaultSelectionBackground;
		}

		Color ResolveSelectionTextColor(const ComputedStyle* style)
		{
			if (style && style->visualProps.selectionColor.a != 0)
				return style->visualProps.selectionColor;
			return text_paint_utils::ResolveTextColor(style);
		}
	}

	InputTextVisual::InputTextVisual(visuals::View* view, InputTextElement* element, Visual* parent)
		: Visual(view, element, parent)
	{
		m_blinkTimer.SetInterval(kBlinkIntervalMs);
		m_blinkTimer.SetCallback([this] { OnBlinkTimeout(); });
	}

	InputTextVisual::~InputTextVisual()
	{
		m_blinkTimer.Stop();
	}

	InputTextElement* InputTextVisual::InputText() const
	{
		return static_cast<InputTextElement*>(Element());
	}

	basic::single InputTextVisual::ScrollOffset() const
	{
		return m_scrollX;
	}

	RectF InputTextVisual::LocalContentRect() const
	{
		// Paint is in local space with origin at the border-box top-left.
		// ContentRect() keeps Yoga parent offsets, so rebuild from a local layout rect.
		return yoga_utils::LocalContentRect(LayoutRect(), YogaNode());
	}

	void InputTextVisual::EnsureCaretVisible()
	{
		auto* input = InputText();
		const ComputedStyle* style = GetComputedStyle();
		const RectF contentRect = LocalContentRect();
		if (!input || !style || contentRect.Width() <= 0.0f)
			return;

		FontMetrics metrics(style->visualProps.font);
		std::u8string display(input->Text());
		display.insert(input->CaretIndex(), input->PreeditText());

		const float contentWidth = contentRect.Width();
		const float textWidth = metrics.HorizontalAdvance(display);
		const float caretX =
			metrics.HorizontalAdvance(input->Text().substr(0, input->CaretIndex())) +
			metrics.HorizontalAdvance(input->PreeditText());
		const float maxScroll = std::max(0.0f, textWidth - contentWidth);

		m_scrollX = std::clamp(m_scrollX, 0.0f, maxScroll);
		if (caretX - m_scrollX > contentWidth)
			m_scrollX = caretX - contentWidth;
		else if (caretX - m_scrollX < 0.0f)
			m_scrollX = caretX;
		m_scrollX = std::clamp(m_scrollX, 0.0f, maxScroll);
	}

	void InputTextVisual::BuildVisuals()
	{
		Visual::BuildVisuals();
		YGNodeRef yogaNode = YogaNode();
		if (!yogaNode)
			return;

		YGNodeSetContext(yogaNode, this);
		YGNodeSetMeasureFunc(yogaNode, MeasureInputTextVisual);
		YGNodeMarkDirty(yogaNode);
	}

	void InputTextVisual::PaintOverride(SkCanvas* canvas, const basic::RectF& dirtyRect)
	{
		Visual::PaintOverride(canvas, dirtyRect);

		auto* input = InputText();
		const ComputedStyle* style = GetComputedStyle();
		EnsureCaretVisible();

		InputTextLayout layout;
		if (!ResolveInputTextLayout(input, style, LocalContentRect(), m_scrollX, layout))
			return;

		canvas->save();
		canvas->clipRect(SkRect::MakeLTRB(
			layout.contentRect.left,
			layout.contentRect.top,
			layout.contentRect.right,
			layout.contentRect.bottom));

		FontMetrics metrics(style->visualProps.font);
		const Color textColor = text_paint_utils::ResolveTextColor(style);

		SkPaint paint;
		paint.setAntiAlias(true);
		paint.setStyle(SkPaint::kFill_Style);

		auto drawUtf8 = [&](const std::u8string_view& chunk, float x, const Color& color)
		{
			if (chunk.empty())
				return;
			paint.setColor(skia_utils::ToSkColor(color));
			metrics.DrawText(canvas, chunk, x, layout.baselineY, paint);
		};

		if (input->HasSelection() && input->PreeditText().empty())
		{
			const std::u8string_view text = input->Text();
			const size_t selStart = input->SelectionStart();
			const size_t selEnd = input->SelectionEnd();
			const auto before = text.substr(0, selStart);
			const auto selected = text.substr(selStart, selEnd - selStart);
			const auto after = text.substr(selEnd);

			const float beforeWidth = metrics.HorizontalAdvance(before);
			const float selectedWidth = metrics.HorizontalAdvance(selected);
			const float top = layout.baselineY - layout.ascent;
			const float bottom = layout.baselineY + layout.descent;

			SkPaint selectionPaint;
			selectionPaint.setAntiAlias(true);
			selectionPaint.setStyle(SkPaint::kFill_Style);
			selectionPaint.setColor(skia_utils::ToSkColor(ResolveSelectionBackground(style)));
			canvas->drawRect(
				SkRect::MakeLTRB(
					layout.textX + beforeWidth,
					top,
					layout.textX + beforeWidth + selectedWidth,
					bottom),
				selectionPaint);

			const Color selectedTextColor = ResolveSelectionTextColor(style);
			drawUtf8(before, layout.textX, textColor);
			drawUtf8(selected, layout.textX + beforeWidth, selectedTextColor);
			drawUtf8(after, layout.textX + beforeWidth + selectedWidth, textColor);
		}
		else
		{
			drawUtf8(layout.prefix, layout.textX, textColor);
			drawUtf8(layout.preedit, layout.textX + layout.prefixWidth, textColor);
			drawUtf8(layout.suffix, layout.textX + layout.prefixWidth + layout.preeditWidth, textColor);

			if (!layout.preedit.empty())
			{
				paint.setColor(skia_utils::ToSkColor(textColor));
				SkPaint underline = paint;
				underline.setStyle(SkPaint::kStroke_Style);
				underline.setStrokeWidth(std::max(metrics.UnderlineThickness(), 1.0f));
				const float y = layout.baselineY + metrics.UnderlineOffset();
				canvas->drawLine(
					layout.textX + layout.prefixWidth,
					y,
					layout.textX + layout.prefixWidth + layout.preeditWidth,
					y,
					underline);
			}
		}

		const bool showCaret =
			input->States()->IsFocused() &&
			m_caretVisible &&
			!input->HasSelection();
		if (showCaret)
		{
			const float caretX = layout.textX + layout.prefixWidth + layout.preeditWidth;
			paint.setColor(skia_utils::ToSkColor(textColor));
			SkPaint caretPaint = paint;
			caretPaint.setStyle(SkPaint::kStroke_Style);
			caretPaint.setStrokeWidth(1.0f);
			canvas->drawLine(
				caretX,
				layout.baselineY - layout.ascent,
				caretX,
				layout.baselineY + layout.descent,
				caretPaint);
		}

		canvas->restore();
		SyncImeCaretRect();
	}

	void InputTextVisual::SyncImeCaretRect()
	{
		auto* input = InputText();
		auto* view = View();
		if (!input || !view || !view->Window() || !input->States()->IsFocused())
			return;

		EnsureCaretVisible();

		InputTextLayout layout;
		if (!ResolveInputTextLayout(input, GetComputedStyle(), LocalContentRect(), m_scrollX, layout))
			return;

		const float caretX = layout.textX + layout.prefixWidth + layout.preeditWidth;
		const float top = layout.baselineY - layout.ascent;
		const float bottom = layout.baselineY + layout.descent;

		const PointF rootTopLeft = MapLocalToRoot(this, PointF(caretX, top));
		const PointF rootBottom = MapLocalToRoot(this, PointF(caretX, bottom));
		const RectF clientRect = view->Window()->ClientRect();
		view->Window()->SetImeCaretRect(RectF(
			rootTopLeft.x + clientRect.left,
			rootTopLeft.y + clientRect.top,
			rootTopLeft.x + clientRect.left + 1.0f,
			rootBottom.y + clientRect.top));
	}

	void InputTextVisual::SyncCaretBlink(bool focused)
	{
		if (focused)
		{
			ResetCaretBlink();
		}
		else
		{
			m_blinkTimer.Stop();
			m_caretVisible = false;
			m_dragging = false;
			Repaint();
		}
	}

	void InputTextVisual::ResetCaretBlink()
	{
		m_caretVisible = true;
		m_blinkTimer.Start(kBlinkIntervalMs);
		Repaint();
	}

	void InputTextVisual::OnBlinkTimeout()
	{
		auto* input = InputText();
		if (!input || !input->States()->IsFocused() || input->HasSelection())
		{
			m_caretVisible = false;
			Repaint();
			return;
		}

		m_caretVisible = !m_caretVisible;
		Repaint();
	}

	size_t InputTextVisual::CaretIndexAtLocalX(basic::single localX) const
	{
		auto* input = InputText();
		const ComputedStyle* style = GetComputedStyle();
		if (!input || !style)
			return 0;

		const RectF contentRect = LocalContentRect();
		const float x = localX - contentRect.left + m_scrollX;
		const std::u8string_view text = input->Text();
		FontMetrics metrics(style->visualProps.font);

		size_t best = 0;
		float bestDist = std::abs(x);
		for (size_t index = 0; index <= text.size(); )
		{
			const float width = metrics.HorizontalAdvance(text.substr(0, index));
			const float dist = std::abs(width - x);
			if (dist < bestDist)
			{
				bestDist = dist;
				best = index;
			}

			if (index >= text.size())
				break;
			index = utf8::NextIndex(text, index);
		}
		return best;
	}

	void InputTextVisual::HandleMousePress(MousePressEvent* e)
	{
		auto* input = InputText();
		if (!input || !e || e->button != MouseButton::Left || input->IsComposing())
			return;

		const size_t index = CaretIndexAtLocalX(e->pos.x);
		const bool extend = (e->modifier & Modifier::Shift) != Modifier::None;
		if (extend)
			input->SetSelection(input->AnchorIndex(), index);
		else
			input->SetSelection(index, index);

		m_dragging = true;
		ResetCaretBlink();
	}

	void InputTextVisual::HandleMouseMove(MouseMoveEvent* e)
	{
		auto* input = InputText();
		if (!input || !e || !m_dragging || input->IsComposing())
			return;

		input->SetSelection(input->AnchorIndex(), CaretIndexAtLocalX(e->pos.x));
		ResetCaretBlink();
	}

	void InputTextVisual::HandleMouseRelease(MouseReleaseEvent* e)
	{
		if (e && e->button == MouseButton::Left)
			m_dragging = false;
	}

	bool InputTextVisual::Filter(events::EventTarget* target, events::Event* e)
	{
		auto* input = InputText();
		if (target == input && e)
		{
			switch (e->type)
			{
			case Type::ElementTextChanged:
				Reflow();
				EnsureCaretVisible();
				if (input->States()->IsFocused())
					ResetCaretBlink();
				else
					Repaint();
				return false;
			case Type::ElementStatesChanged:
				Reflow();
				SyncCaretBlink(input->States()->IsFocused());
				return false;
			case Type::MousePress:
				HandleMousePress(static_cast<MousePressEvent*>(e));
				return false;
			case Type::MouseMove:
				HandleMouseMove(static_cast<MouseMoveEvent*>(e));
				return false;
			case Type::MouseRelease:
				HandleMouseRelease(static_cast<MouseReleaseEvent*>(e));
				return false;
			default:
				break;
			}
		}

		return Visual::Filter(target, e);
	}
}

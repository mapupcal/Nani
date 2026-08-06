#include "input_text_visual.h"
#include "view.h"
#include "../elements/input_text_element.h"
#include "../elements/element_states.h"
#include "../events/event.h"
#include "../styles.h"
#include "../window.h"
#include "../text/font_metrics.h"
#include "../text/text_alignment.h"
#include "../internal/computed_style.h"
#include "../internal/skia_defs.h"
#include "../internal/skia_utils.h"
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
		Color ResolveTextColor(const ComputedStyle* style)
		{
			if (!style)
				return Colors::Black;

			if (style->visualProps.color.a != 0)
				return style->visualProps.color;
			return Colors::Black;
		}

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
			float textTop = contentRect.top;
			switch (style->visualProps.textAlignment.VerticalAlign())
			{
			case TextAlignment::Vertical::Center:
				textTop = contentRect.top + (contentRect.Height() - lineHeight) * 0.5f;
				break;
			case TextAlignment::Vertical::Bottom:
				textTop = contentRect.bottom - lineHeight;
				break;
			case TextAlignment::Vertical::Top:
			default:
				textTop = contentRect.top;
				break;
			}

			out.textX = contentRect.left;
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

			float measuredWidth = contentWidth;
			if (widthMode == YGMeasureModeExactly)
				measuredWidth = width;
			else if (widthMode == YGMeasureModeAtMost)
				measuredWidth = std::min(contentWidth, width);

			float measuredHeight = contentHeight;
			if (heightMode == YGMeasureModeExactly)
				measuredHeight = height;
			else if (heightMode == YGMeasureModeAtMost)
				measuredHeight = std::min(contentHeight, height);

			return { measuredWidth, measuredHeight };
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
	}

	InputTextVisual::InputTextVisual(visuals::View* view, InputTextElement* element, Visual* parent)
		: Visual(view, element, parent)
	{
	}

	InputTextElement* InputTextVisual::InputText() const
	{
		return static_cast<InputTextElement*>(Element());
	}

	RectF InputTextVisual::LocalContentRect() const
	{
		// Paint is in local space with origin at the border-box top-left.
		// ContentRect() keeps Yoga parent offsets, so rebuild from a local layout rect.
		RectF content = LayoutRect();
		content.MoveTo(PointF(0.0f, 0.0f));
		YGNodeRef yogaNode = YogaNode();
		if (!yogaNode)
			return content;
		return content - (yoga_utils::GetNodeBorders(yogaNode) + yoga_utils::GetNodePaddings(yogaNode));
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
		InputTextLayout layout;
		if (!ResolveInputTextLayout(input, style, LocalContentRect(), layout))
			return;

		canvas->save();
		canvas->clipRect(SkRect::MakeLTRB(
			layout.contentRect.left,
			layout.contentRect.top,
			layout.contentRect.right,
			layout.contentRect.bottom));

		FontMetrics metrics(style->visualProps.font);

		SkPaint paint;
		paint.setAntiAlias(true);
		paint.setStyle(SkPaint::kFill_Style);
		paint.setColor(skia_utils::ToSkColor(ResolveTextColor(style)));

		auto drawUtf8 = [&](const std::u8string_view& chunk, float x)
		{
			metrics.DrawText(canvas, chunk, x, layout.baselineY, paint);
		};

		drawUtf8(layout.prefix, layout.textX);
		drawUtf8(layout.preedit, layout.textX + layout.prefixWidth);
		drawUtf8(layout.suffix, layout.textX + layout.prefixWidth + layout.preeditWidth);

		if (!layout.preedit.empty())
		{
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

		if (input->States()->IsFocused())
		{
			const float caretX = layout.textX + layout.prefixWidth + layout.preeditWidth;
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

		InputTextLayout layout;
		if (!ResolveInputTextLayout(input, GetComputedStyle(), LocalContentRect(), layout))
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

	bool InputTextVisual::Filter(events::EventTarget* target, events::Event* e)
	{
		if (target == InputText() &&
			(e->type == Type::ElementTextChanged || e->type == Type::ElementStatesChanged))
		{
			Reflow();
			Repaint();
			return false;
		}

		return Visual::Filter(target, e);
	}
}

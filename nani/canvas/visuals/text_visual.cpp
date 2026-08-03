#include "text_visual.h"
#include "../elements/text_element.h"
#include "../elements/element_states.h"
#include "../events/event.h"
#include "../styles.h"
#include "../text/font_metrics.h"
#include "../internal/computed_style.h"
#include "../internal/font_manager_p.h"
#include "../internal/skia_utils.h"
#include "../internal/yoga_utils.h"
#include <core/SkCanvas.h>
#include <core/SkPaint.h>
#include <effects/SkDashPathEffect.h>
#include <yoga/Yoga.h>
#include <algorithm>

using namespace nani::canvas::elements;
using namespace nani::canvas::events;
using namespace nani::canvas::basic;
using namespace nani::canvas::text;
using namespace nani::canvas::internal;

namespace nani::canvas::visuals
{
	namespace
	{
		elements::TextElement* AsTextElement(const Visual* visual)
		{
			return visual ? static_cast<elements::TextElement*>(visual->Element()) : nullptr;
		}

		Color ResolveTextColor(const ComputedStyle* style)
		{
			if (!style)
				return Colors::Black;

			const auto& visualProps = style->visualProps;
			if (visualProps.color.a != 0)
				return visualProps.color;

			if (visualProps.textDecoration.Color().a != 0)
				return visualProps.textDecoration.Color();

			return Colors::Black;
		}

		void DrawHorizontalLine(
			SkCanvas* canvas,
			const SkPaint& basePaint,
			float x1,
			float y,
			float x2,
			TextDecoration::DecorationStyle style)
		{
			SkPaint paint = basePaint;
			paint.setStyle(SkPaint::kStroke_Style);
			paint.setStrokeWidth(1.0f);

			if (style == TextDecoration::DecorationStyle::Double)
			{
				canvas->drawLine(x1, y - 1.5f, x2, y - 1.5f, paint);
				canvas->drawLine(x1, y + 1.5f, x2, y + 1.5f, paint);
				return;
			}

			if (style == TextDecoration::DecorationStyle::Dotted)
				paint.setPathEffect(SkDashPathEffect::Make({ 1.0f, 2.0f }, 0.0f));
			else if (style == TextDecoration::DecorationStyle::Dashed)
				paint.setPathEffect(SkDashPathEffect::Make({ 4.0f, 2.0f }, 0.0f));

			canvas->drawLine(x1, y, x2, y, paint);
		}

		void DrawTextDecorations(
			SkCanvas* canvas,
			const TextDecoration& decoration,
			float x,
			float baselineY,
			float textWidth,
			const FontMetrics& metrics)
		{
			const auto lines = decoration.Lines();
			if (lines == TextDecoration::DecorationLine::None || textWidth <= 0.0f)
				return;

			SkPaint paint;
			paint.setAntiAlias(true);
			paint.setColor(skia_utils::ToSkColor(decoration.Color()));

			const auto style = decoration.Style();
			const float x2 = x + textWidth;

			if ((lines & TextDecoration::DecorationLine::Overline) != TextDecoration::DecorationLine::None)
				DrawHorizontalLine(canvas, paint, x, baselineY - metrics.Ascent(), x2, style);

			if ((lines & TextDecoration::DecorationLine::Underline) != TextDecoration::DecorationLine::None)
				DrawHorizontalLine(canvas, paint, x, baselineY + metrics.Descent(), x2, style);

			if ((lines & TextDecoration::DecorationLine::LineThrough) != TextDecoration::DecorationLine::None)
				DrawHorizontalLine(canvas, paint, x, baselineY - metrics.Ascent() * 0.35f, x2, style);
		}

		YGSize MeasureTextVisual(
			YGNodeConstRef node,
			float width,
			YGMeasureMode widthMode,
			float height,
			YGMeasureMode heightMode)
		{
			auto* visual = static_cast<TextVisual*>(YGNodeGetContext(node));
			auto* textElement = AsTextElement(visual);
			if (!textElement)
				return { 0.0f, 0.0f };

			// Free Yoga callbacks cannot use Visual's protected helpers; resolve style
			// through the public Styles API instead.
			Styles* styles = textElement->GetStyles();
			if (!styles)
				return { 0.0f, 0.0f };

			std::shared_ptr<ComputedStyle> style = styles->Compute(
				textElement->StyleClass(),
				textElement->States()->GetStateProps());
			if (!style)
				return { 0.0f, 0.0f };

			FontMetrics metrics(style->visualProps.font);
			const float textWidth = metrics.HorizontalAdvance(textElement->Text());
			const float textHeight = metrics.LineHeight();

			float measuredWidth = textWidth;
			if (widthMode == YGMeasureModeExactly)
				measuredWidth = width;
			else if (widthMode == YGMeasureModeAtMost)
				measuredWidth = std::min(textWidth, width);

			float measuredHeight = textHeight;
			if (heightMode == YGMeasureModeExactly)
				measuredHeight = height;
			else if (heightMode == YGMeasureModeAtMost)
				measuredHeight = std::min(textHeight, height);

			return { measuredWidth, measuredHeight };
		}
	}

	TextVisual::TextVisual(visuals::View* view, elements::TextElement* element, Visual* parent)
		: Visual(view, element, parent)
	{
	}

	void TextVisual::BuildVisuals()
	{
		Visual::BuildVisuals();
		YGNodeRef yogaNode = YogaNode();
		if (!yogaNode)
			return;

		YGNodeSetContext(yogaNode, this);
		YGNodeSetMeasureFunc(yogaNode, MeasureTextVisual);
		YGNodeMarkDirty(yogaNode);
	}

	void TextVisual::PaintOverride(SkCanvas* canvas, const basic::RectF& dirtyRect)
	{
		Visual::PaintOverride(canvas, dirtyRect);

		const ComputedStyle* style = GetComputedStyle();
		auto* textElement = AsTextElement(this);
		if (!style || !textElement)
			return;

		const std::u8string_view text = textElement->Text();
		if (text.empty())
			return;

		const Font& font = style->visualProps.font;
		auto skFont = FontManagerPrivate::Instance()->CreateSkFont(font);
		if (!skFont)
			return;

		FontMetrics metrics(font);

		// Paint coordinates are local to the border box (parent already translated by LayoutRect).
		RectF contentRect = LayoutRect();
		contentRect.MoveTo(PointF(0.0f, 0.0f));
		YGNodeRef yogaNode = YogaNode();
		contentRect = contentRect - (yoga_utils::GetNodeBorders(yogaNode) + yoga_utils::GetNodePaddings(yogaNode));

		if (contentRect.Width() <= 0.0f || contentRect.Height() <= 0.0f)
			return;

		std::u8string displayText;
		if (textElement->ElideMode() == TextElideMode::None)
			displayText = std::u8string(text);
		else
			displayText = metrics.ElidedText(text, contentRect.Width(), textElement->ElideMode());

		if (displayText.empty())
			return;

		const float textWidth = metrics.HorizontalAdvance(displayText);
		const float textHeight = metrics.Ascent() + metrics.Descent();
		const TextAlignment& textAlignment = style->visualProps.textAlignment;

		float baselineX = contentRect.left;
		switch (textAlignment.HorizontalAlign())
		{
		case TextAlignment::Horizontal::Center:
			baselineX = contentRect.left + (contentRect.Width() - textWidth) * 0.5f;
			break;
		case TextAlignment::Horizontal::Right:
			baselineX = contentRect.right - textWidth;
			break;
		case TextAlignment::Horizontal::Left:
		default:
			baselineX = contentRect.left;
			break;
		}

		float baselineY = contentRect.top + metrics.Ascent();
		switch (textAlignment.VerticalAlign())
		{
		case TextAlignment::Vertical::Center:
			baselineY = contentRect.top + (contentRect.Height() - textHeight) * 0.5f + metrics.Ascent();
			break;
		case TextAlignment::Vertical::Bottom:
			baselineY = contentRect.bottom - metrics.Descent();
			break;
		case TextAlignment::Vertical::Top:
		default:
			baselineY = contentRect.top + metrics.Ascent();
			break;
		}

		SkPaint paint;
		paint.setAntiAlias(true);
		paint.setStyle(SkPaint::kFill_Style);
		paint.setColor(skia_utils::ToSkColor(ResolveTextColor(style)));

		SkFont drawFont = *skFont;
		drawFont.setEdging(SkFont::Edging::kAntiAlias);
		drawFont.setSubpixel(true);

		const char* utf8Data = reinterpret_cast<const char*>(displayText.data());
		canvas->drawSimpleText(
			utf8Data,
			displayText.size(),
			SkTextEncoding::kUTF8,
			baselineX,
			baselineY,
			drawFont,
			paint);

		DrawTextDecorations(
			canvas,
			style->visualProps.textDecoration,
			baselineX,
			baselineY,
			textWidth,
			metrics);
	}

	bool TextVisual::Filter(events::EventTarget* target, events::Event* e)
	{
		if (target == AsTextElement(this) && e->type == Type::ElementTextChanged)
		{
			Reflow();
			Repaint();
			return false;
		}

		return Visual::Filter(target, e);
	}
}

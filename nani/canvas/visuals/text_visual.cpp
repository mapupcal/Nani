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
#include <core/SkPathBuilder.h>
#include <effects/SkDashPathEffect.h>
#include <yoga/Yoga.h>
#include <algorithm>
#include <cmath>
#include <numbers>

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

		bool HasDecorationLine(DecorationLine lines, DecorationLine flag)
		{
			return (lines & flag) != DecorationLine::None;
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

		Color ResolveDecorationColor(const TextDecoration& decoration, const ComputedStyle* style)
		{
			if (decoration.Color().a != 0)
				return decoration.Color();
			return ResolveTextColor(style);
		}

		void DrawDecorationStroke(
			SkCanvas* canvas,
			const SkPaint& basePaint,
			float x1,
			float y,
			float x2,
			float thickness,
			DecorationStyle style)
		{
			if (x2 <= x1 || thickness <= 0.0f)
				return;

			SkPaint paint = basePaint;
			paint.setStyle(SkPaint::kStroke_Style);
			paint.setStrokeWidth(thickness);
			paint.setStrokeCap(SkPaint::kButt_Cap);

			if (style == DecorationStyle::Double)
			{
				const float gap = std::max(thickness * 1.5f, 1.5f);
				canvas->drawLine(x1, y - gap, x2, y - gap, paint);
				canvas->drawLine(x1, y + gap, x2, y + gap, paint);
				return;
			}

			if (style == DecorationStyle::Wavy)
			{
				const float amplitude = std::max(thickness * 1.25f, 1.5f);
				const float wavelength = std::max(thickness * 4.0f, 4.0f);
				SkPathBuilder builder;
				builder.moveTo(x1, y);
				for (float x = x1 + 1.0f; x < x2; x += 1.0f)
				{
					const float t = (x - x1) / wavelength * (2.0f * std::numbers::pi_v<float>);
					builder.lineTo(x, y + std::sin(t) * amplitude);
				}
				const float endT = (x2 - x1) / wavelength * (2.0f * std::numbers::pi_v<float>);
				builder.lineTo(x2, y + std::sin(endT) * amplitude);
				canvas->drawPath(builder.detach(), paint);
				return;
			}

			if (style == DecorationStyle::Dotted)
			{
				const float dot = std::max(thickness, 1.0f);
				paint.setPathEffect(SkDashPathEffect::Make({ dot, dot * 1.5f }, 0.0f));
			}
			else if (style == DecorationStyle::Dashed)
			{
				const float dash = std::max(thickness * 3.0f, 3.0f);
				paint.setPathEffect(SkDashPathEffect::Make({ dash, dash * 0.75f }, 0.0f));
			}

			canvas->drawLine(x1, y, x2, y, paint);
		}

		void DrawTextDecorations(
			SkCanvas* canvas,
			const TextDecoration& decoration,
			const ComputedStyle* style,
			float x,
			float baselineY,
			float textWidth,
			const FontMetrics& metrics)
		{
			const auto lines = decoration.Lines();
			if (lines == DecorationLine::None || textWidth <= 0.0f)
				return;

			SkPaint paint;
			paint.setAntiAlias(true);
			paint.setColor(skia_utils::ToSkColor(ResolveDecorationColor(decoration, style)));

			const auto decoStyle = decoration.Style();
			const float x2 = x + textWidth;
			const float underlineThickness = metrics.UnderlineThickness();
			const float strikeThickness = metrics.StrikeoutThickness();

			if (HasDecorationLine(lines, DecorationLine::Overline))
			{
				DrawDecorationStroke(
					canvas, paint, x,
					baselineY - metrics.Ascent() + underlineThickness * 0.5f,
					x2, underlineThickness, decoStyle);
			}

			if (HasDecorationLine(lines, DecorationLine::Underline))
			{
				DrawDecorationStroke(
					canvas, paint, x,
					baselineY + metrics.UnderlineOffset(),
					x2, underlineThickness, decoStyle);
			}

			if (HasDecorationLine(lines, DecorationLine::LineThrough))
			{
				DrawDecorationStroke(
					canvas, paint, x,
					baselineY + metrics.StrikeoutOffset(),
					x2, strikeThickness, decoStyle);
			}
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

		struct TextPaintLayout
		{
			std::u8string displayText;
			float baselineX = 0.0f;
			float baselineY = 0.0f;
			float textWidth = 0.0f;
			RectF textRect;
		};

		bool ResolveTextPaintLayout(
			TextElement* textElement,
			const ComputedStyle* style,
			YGNodeRef yogaNode,
			const RectF& layoutRect,
			TextPaintLayout& out)
		{
			if (!textElement || !style || !yogaNode)
				return false;

			const std::u8string_view text = textElement->Text();
			if (text.empty())
				return false;

			FontMetrics metrics(style->visualProps.font);

			RectF contentRect = layoutRect;
			contentRect.MoveTo(PointF(0.0f, 0.0f));
			contentRect = contentRect - (yoga_utils::GetNodeBorders(yogaNode) + yoga_utils::GetNodePaddings(yogaNode));
			if (contentRect.Width() <= 0.0f || contentRect.Height() <= 0.0f)
				return false;

			if (textElement->ElideMode() == TextElideMode::None)
				out.displayText = std::u8string(text);
			else
				out.displayText = metrics.ElidedText(text, contentRect.Width(), textElement->ElideMode());

			if (out.displayText.empty())
				return false;

			out.textWidth = metrics.HorizontalAdvance(out.displayText);
			const float textHeight = metrics.Ascent() + metrics.Descent();
			const TextAlignment& textAlignment = style->visualProps.textAlignment;

			switch (textAlignment.HorizontalAlign())
			{
			case TextAlignment::Horizontal::Center:
				out.baselineX = contentRect.left + (contentRect.Width() - out.textWidth) * 0.5f;
				break;
			case TextAlignment::Horizontal::Right:
				out.baselineX = contentRect.right - out.textWidth;
				break;
			case TextAlignment::Horizontal::Left:
			default:
				out.baselineX = contentRect.left;
				break;
			}

			switch (textAlignment.VerticalAlign())
			{
			case TextAlignment::Vertical::Center:
				out.baselineY = contentRect.top + (contentRect.Height() - textHeight) * 0.5f + metrics.Ascent();
				break;
			case TextAlignment::Vertical::Bottom:
				out.baselineY = contentRect.bottom - metrics.Descent();
				break;
			case TextAlignment::Vertical::Top:
			default:
				out.baselineY = contentRect.top + metrics.Ascent();
				break;
			}

			out.textRect = RectF(
				out.baselineX,
				out.baselineY - metrics.Ascent(),
				out.baselineX + out.textWidth,
				out.baselineY + metrics.Descent());
			return true;
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

	bool TextVisual::HitTestOverride(const basic::PointF& localPos)
	{
		TextPaintLayout layout;
		if (!ResolveTextPaintLayout(
			AsTextElement(this),
			GetComputedStyle(),
			YogaNode(),
			LayoutRect(),
			layout))
		{
			return false;
		}

		return layout.textRect.IsContains(localPos);
	}

	void TextVisual::PaintOverride(SkCanvas* canvas, const basic::RectF& dirtyRect)
	{
		Visual::PaintOverride(canvas, dirtyRect);

		const ComputedStyle* style = GetComputedStyle();
		auto* textElement = AsTextElement(this);

		TextPaintLayout layout;
		if (!ResolveTextPaintLayout(textElement, style, YogaNode(), LayoutRect(), layout))
			return;

		const Font& font = style->visualProps.font;
		auto skFont = FontManagerPrivate::Instance()->CreateSkFont(font);
		if (!skFont)
			return;

		SkPaint paint;
		paint.setAntiAlias(true);
		paint.setStyle(SkPaint::kFill_Style);
		paint.setColor(skia_utils::ToSkColor(ResolveTextColor(style)));

		SkFont drawFont = *skFont;
		drawFont.setEdging(SkFont::Edging::kAntiAlias);
		drawFont.setSubpixel(true);

		const char* utf8Data = reinterpret_cast<const char*>(layout.displayText.data());
		canvas->drawSimpleText(
			utf8Data,
			layout.displayText.size(),
			SkTextEncoding::kUTF8,
			layout.baselineX,
			layout.baselineY,
			drawFont,
			paint);

		FontMetrics metrics(font);
		DrawTextDecorations(
			canvas,
			style->visualProps.textDecoration,
			style,
			layout.baselineX,
			layout.baselineY,
			layout.textWidth,
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

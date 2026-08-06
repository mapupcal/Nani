#include "text_visual.h"
#include "../elements/text_element.h"
#include "../elements/element_states.h"
#include "../events/event.h"
#include "../styles.h"
#include "../text/font_metrics.h"
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
			const bool wrap = textElement->WrapMode() == TextWrapMode::Wrap;
			const float wrapWidth =
				(wrap && (widthMode == YGMeasureModeExactly || widthMode == YGMeasureModeAtMost))
					? width
					: 0.0f;
			const auto lines = metrics.LayoutLines(textElement->Text(), wrapWidth, wrap);

			float contentWidth = 0.0f;
			for (const auto& line : lines)
				contentWidth = std::max(contentWidth, metrics.HorizontalAdvance(line));

			const float lineHeight = metrics.LineHeight();
			const float contentHeight = lines.empty()
				? lineHeight
				: lineHeight * static_cast<float>(lines.size());

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

		struct TextLineLayout
		{
			std::u8string text;
			float baselineX = 0.0f;
			float baselineY = 0.0f;
			float width = 0.0f;
			RectF rect;
		};

		struct TextPaintLayout
		{
			std::vector<TextLineLayout> lines;
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

			const bool wrap = textElement->WrapMode() == TextWrapMode::Wrap;
			auto rawLines = metrics.LayoutLines(
				text,
				wrap ? contentRect.Width() : 0.0f,
				wrap);
			if (rawLines.empty())
				return false;

			// Soft-wrapped text keeps full lines; NoWrap still supports hard '\n'
			// and applies elide per visual line.
			out.lines.clear();
			out.lines.reserve(rawLines.size());
			for (auto& line : rawLines)
			{
				TextLineLayout lineLayout;
				if (!wrap && textElement->ElideMode() != TextElideMode::None)
					lineLayout.text = metrics.ElidedText(line, contentRect.Width(), textElement->ElideMode());
				else
					lineLayout.text = std::move(line);

				if (lineLayout.text.empty() && rawLines.size() == 1)
					return false;

				lineLayout.width = metrics.HorizontalAdvance(lineLayout.text);
				out.lines.push_back(std::move(lineLayout));
			}

			const float lineHeight = metrics.LineHeight();
			const float blockHeight = lineHeight * static_cast<float>(out.lines.size());
			const TextAlignment& textAlignment = style->visualProps.textAlignment;

			float blockTop = contentRect.top;
			switch (textAlignment.VerticalAlign())
			{
			case TextAlignment::Vertical::Center:
				blockTop = contentRect.top + (contentRect.Height() - blockHeight) * 0.5f;
				break;
			case TextAlignment::Vertical::Bottom:
				blockTop = contentRect.bottom - blockHeight;
				break;
			case TextAlignment::Vertical::Top:
			default:
				blockTop = contentRect.top;
				break;
			}

			bool hasBounds = false;
			for (size_t i = 0; i < out.lines.size(); ++i)
			{
				auto& lineLayout = out.lines[i];
				switch (textAlignment.HorizontalAlign())
				{
				case TextAlignment::Horizontal::Center:
					lineLayout.baselineX =
						contentRect.left + (contentRect.Width() - lineLayout.width) * 0.5f;
					break;
				case TextAlignment::Horizontal::Right:
					lineLayout.baselineX = contentRect.right - lineLayout.width;
					break;
				case TextAlignment::Horizontal::Left:
				default:
					lineLayout.baselineX = contentRect.left;
					break;
				}

				lineLayout.baselineY = blockTop + lineHeight * static_cast<float>(i) + metrics.Ascent();
				lineLayout.rect = RectF(
					lineLayout.baselineX,
					lineLayout.baselineY - metrics.Ascent(),
					lineLayout.baselineX + lineLayout.width,
					lineLayout.baselineY + metrics.Descent());

				if (!hasBounds)
				{
					out.textRect = lineLayout.rect;
					hasBounds = true;
				}
				else
				{
					out.textRect |= lineLayout.rect;
				}
			}

			return hasBounds;
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
		FontMetrics metrics(font);

		SkPaint paint;
		paint.setAntiAlias(true);
		paint.setStyle(SkPaint::kFill_Style);
		paint.setColor(skia_utils::ToSkColor(ResolveTextColor(style)));

		for (const auto& line : layout.lines)
		{
			if (line.text.empty())
				continue;

			metrics.DrawText(canvas, line.text, line.baselineX, line.baselineY, paint);

			DrawTextDecorations(
				canvas,
				style->visualProps.textDecoration,
				style,
				line.baselineX,
				line.baselineY,
				line.width,
				metrics);
		}
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

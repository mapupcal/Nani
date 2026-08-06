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

#include <memory>

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

		struct DisplayBoundary
		{
			size_t displayOffset = 0;
			size_t sourceOffset = 0;
		};

		struct DisplayModel
		{
			std::u8string display;
			std::vector<DisplayBoundary> boundaries; // sorted by displayOffset
			size_t caretDisplayOffset = 0;
			size_t preeditDisplayStart = 0;
			size_t preeditDisplayEnd = 0;
		};

		struct VisualLine
		{
			size_t displayStart = 0;
			size_t displayEnd = 0;
			float width = 0.0f;
			float top = 0.0f;
			float baselineY = 0.0f;
			float bottom = 0.0f;
		};

		struct DocumentLayout
		{
			RectF contentRect;
			DisplayModel display;
			std::vector<VisualLine> lines;
			float ascent = 0.0f;
			float descent = 0.0f;
			float lineHeight = 0.0f;
			float blockHeight = 0.0f;
			float maxLineWidth = 0.0f;
		};

		struct LayoutKey
		{
			RectF contentRect;
			Font font;
			std::u8string text;
			std::u8string preedit;
			std::u8string passwordEcho;
			size_t caret = 0;
			bool multiLine = false;
			bool passwordMode = false;
			bool passwordVisible = false;
			TextAlignment::Vertical verticalAlign = TextAlignment::Vertical::Center;

			friend bool operator==(const LayoutKey& lhs, const LayoutKey& rhs)
			{
				return lhs.contentRect == rhs.contentRect &&
					lhs.font == rhs.font &&
					lhs.text == rhs.text &&
					lhs.preedit == rhs.preedit &&
					lhs.passwordEcho == rhs.passwordEcho &&
					lhs.caret == rhs.caret &&
					lhs.multiLine == rhs.multiLine &&
					lhs.passwordMode == rhs.passwordMode &&
					lhs.passwordVisible == rhs.passwordVisible &&
					lhs.verticalAlign == rhs.verticalAlign;
			}
		};

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

		void AppendMaskedRange(
			DisplayModel& model,
			const std::u8string_view& text,
			size_t begin,
			size_t end,
			bool mask,
			const std::u8string_view& echo)
		{
			for (size_t index = begin; index < end; )
			{
				const size_t next = utf8::NextIndex(text, index);
				if (mask)
					model.display.append(echo);
				else
					model.display.append(text.substr(index, next - index));
				model.boundaries.push_back({ model.display.size(), next });
				index = next;
			}
		}

		DisplayModel BuildDisplayModel(const InputTextElement* input)
		{
			DisplayModel model;
			model.boundaries.push_back({ 0, 0 });
			if (!input)
				return model;

			const std::u8string_view text = input->Text();
			const size_t caret = input->CaretIndex();
			const bool mask =
				input->IsPasswordMode() && !input->IsPasswordVisible();
			const std::u8string_view echo = input->PasswordEcho();

			AppendMaskedRange(model, text, 0, caret, mask, echo);
			model.caretDisplayOffset = model.display.size();
			model.preeditDisplayStart = model.display.size();

			const std::u8string_view preedit = input->PreeditText();
			if (!preedit.empty())
			{
				model.display.append(preedit);
				model.boundaries.push_back({ model.display.size(), caret });
				model.caretDisplayOffset = model.display.size();
			}
			model.preeditDisplayEnd = model.display.size();

			AppendMaskedRange(model, text, caret, text.size(), mask, echo);
			return model;
		}

		size_t SourceToDisplayOffset(const DisplayModel& model, size_t sourceOffset)
		{
			size_t best = 0;
			for (const auto& boundary : model.boundaries)
			{
				if (boundary.sourceOffset <= sourceOffset)
					best = boundary.displayOffset;
				else
					break;
			}
			return best;
		}

		size_t DisplayToSourceOffset(const DisplayModel& model, size_t displayOffset)
		{
			size_t best = 0;
			for (const auto& boundary : model.boundaries)
			{
				if (boundary.displayOffset <= displayOffset)
					best = boundary.sourceOffset;
				else
					break;
			}
			return best;
		}

		bool ResolveDocumentLayout(
			InputTextElement* input,
			const ComputedStyle* style,
			const RectF& contentRect,
			DocumentLayout& out)
		{
			if (!input || !style)
				return false;
			if (contentRect.Width() <= 0.0f || contentRect.Height() <= 0.0f)
				return false;

			FontMetrics metrics(style->visualProps.font);
			out.contentRect = contentRect;
			out.display = BuildDisplayModel(input);
			out.ascent = metrics.Ascent();
			out.descent = metrics.Descent();
			out.lineHeight = metrics.LineHeight();

			const bool multiLine = input->IsMultiLine();
			const float wrapWidth = multiLine ? contentRect.Width() : 0.0f;
			auto rawLines = metrics.LayoutLines(out.display.display, wrapWidth, multiLine);
			if (rawLines.empty())
				rawLines.emplace_back();

			out.lines.clear();
			out.lines.reserve(rawLines.size());
			out.maxLineWidth = 0.0f;

			size_t pos = 0;
			const std::u8string_view display = out.display.display;
			for (const auto& line : rawLines)
			{
				if (pos < display.size() && display[pos] == u8'\n')
					++pos;

				if (pos + line.size() > display.size() ||
					display.substr(pos, line.size()) != line)
				{
					// Recover by searching forward for the line content.
					const size_t found = display.find(line, pos);
					if (found == std::u8string_view::npos)
						pos = display.size();
					else
						pos = found;
				}

				VisualLine visualLine;
				visualLine.displayStart = pos;
				visualLine.displayEnd = pos + line.size();
				visualLine.width = metrics.HorizontalAdvance(line);
				out.maxLineWidth = std::max(out.maxLineWidth, visualLine.width);
				out.lines.push_back(visualLine);
				pos = visualLine.displayEnd;
			}

			out.blockHeight = out.lineHeight * static_cast<float>(out.lines.size());
			const float blockTop = multiLine
				? contentRect.top
				: text_paint_utils::AlignedBlockTop(
					contentRect,
					out.blockHeight,
					style->visualProps.textAlignment.VerticalAlign());

			for (size_t i = 0; i < out.lines.size(); ++i)
			{
				auto& line = out.lines[i];
				line.top = blockTop + out.lineHeight * static_cast<float>(i);
				line.baselineY = line.top + out.ascent;
				line.bottom = line.top + out.lineHeight;
			}
			return true;
		}

		size_t LineIndexForDisplayOffset(const DocumentLayout& layout, size_t displayOffset)
		{
			if (layout.lines.empty())
				return 0;
			for (size_t i = 0; i < layout.lines.size(); ++i)
			{
				const auto& line = layout.lines[i];
				if (displayOffset <= line.displayEnd)
					return i;
			}
			return layout.lines.size() - 1;
		}

		size_t CaretIndexFromContentPos(
			const FontMetrics& metrics,
			const DocumentLayout& layout,
			float contentX,
			float contentY)
		{
			if (layout.lines.empty())
				return 0;

			size_t lineIndex = 0;
			float bestY = std::abs(contentY - (layout.lines[0].top + layout.lineHeight * 0.5f - layout.contentRect.top));
			for (size_t i = 0; i < layout.lines.size(); ++i)
			{
				const float mid =
					layout.lines[i].top + layout.lineHeight * 0.5f - layout.contentRect.top;
				const float dist = std::abs(contentY - mid);
				if (dist < bestY)
				{
					bestY = dist;
					lineIndex = i;
				}
			}

			const auto& line = layout.lines[lineIndex];
			size_t bestDisplay = line.displayStart;
			float bestDist = std::abs(contentX);
			for (size_t displayOffset = line.displayStart; displayOffset <= line.displayEnd; )
			{
				const float width = metrics.HorizontalAdvance(
					std::u8string_view(layout.display.display).substr(
						line.displayStart,
						displayOffset - line.displayStart));
				const float dist = std::abs(width - contentX);
				if (dist < bestDist)
				{
					bestDist = dist;
					bestDisplay = displayOffset;
				}
				if (displayOffset >= line.displayEnd)
					break;
				displayOffset = utf8::NextIndex(layout.display.display, displayOffset);
			}
			return DisplayToSourceOffset(layout.display, bestDisplay);
		}

		PointF MapLocalToRoot(Visual* visual, PointF local)
		{
			for (Visual* current = visual; current; )
			{
				Visual* parent = current->Parent();
				if (!parent)
				{
					// Root has no parent offset; still apply its local transform.
					local = current->Transform().ApplyTo(local);
					break;
				}
				// Match paint/hit-test: child origin is LayoutRect - parent scroll.
				local = current->MapToParentLocal(local);
				current = parent;
			}
			return local;
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
			const DisplayModel display = BuildDisplayModel(input);
			const bool multiLine = input->IsMultiLine();
			const float wrapWidth =
				(multiLine && (widthMode == YGMeasureModeExactly || widthMode == YGMeasureModeAtMost))
					? width
					: 0.0f;
			auto lines = metrics.LayoutLines(display.display, wrapWidth, multiLine);
			if (lines.empty())
				lines.emplace_back();

			float contentWidth = 0.0f;
			for (const auto& line : lines)
				contentWidth = std::max(contentWidth, metrics.HorizontalAdvance(line));

			const float contentHeight =
				metrics.LineHeight() * static_cast<float>(std::max<size_t>(lines.size(), 1));
			return {
				yoga_utils::ResolveMeasuredSize(contentWidth, widthMode, width),
				yoga_utils::ResolveMeasuredSize(contentHeight, heightMode, height)
			};
		}
	}

	struct InputTextVisual::LayoutCache
	{
		DocumentLayout layout;
		LayoutKey key;
		bool valid = false;
	};

	InputTextVisual::InputTextVisual(visuals::View* view, InputTextElement* element, Visual* parent)
		: Visual(view, element, parent)
		, m_layoutCache(std::make_unique<LayoutCache>())
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

	basic::single InputTextVisual::ScrollOffsetX() const
	{
		return m_scrollX;
	}

	basic::single InputTextVisual::ScrollOffsetY() const
	{
		return m_scrollY;
	}

	RectF InputTextVisual::LocalContentRect() const
	{
		return yoga_utils::LocalContentRect(LayoutRect(), YogaNode());
	}

	void InputTextVisual::InvalidateLayoutCache() const
	{
		if (m_layoutCache)
			m_layoutCache->valid = false;
	}

	bool InputTextVisual::EnsureDocumentLayout() const
	{
		auto* input = InputText();
		const ComputedStyle* style = GetComputedStyle();
		if (!input || !style || !m_layoutCache)
			return false;

		LayoutKey key{
			LocalContentRect(),
			style->visualProps.font,
			std::u8string(input->Text()),
			std::u8string(input->PreeditText()),
			std::u8string(input->PasswordEcho()),
			input->CaretIndex(),
			input->IsMultiLine(),
			input->IsPasswordMode(),
			input->IsPasswordVisible(),
			style->visualProps.textAlignment.VerticalAlign(),
		};

		auto& cache = *m_layoutCache;
		if (cache.valid && cache.key == key)
			return true;

		if (!ResolveDocumentLayout(input, style, key.contentRect, cache.layout))
		{
			cache.valid = false;
			return false;
		}

		cache.key = std::move(key);
		cache.valid = true;
		return true;
	}

	void InputTextVisual::EnsureCaretVisible()
	{
		if (!EnsureDocumentLayout())
			return;

		const auto& layout = m_layoutCache->layout;
		const ComputedStyle* style = GetComputedStyle();
		FontMetrics metrics(style->visualProps.font);
		const float contentWidth = layout.contentRect.Width();
		const float contentHeight = layout.contentRect.Height();
		const float maxScrollX = std::max(0.0f, layout.maxLineWidth - contentWidth);
		const float maxScrollY = std::max(0.0f, layout.blockHeight - contentHeight);

		const size_t caretDisplay = layout.display.caretDisplayOffset;
		const size_t lineIndex = LineIndexForDisplayOffset(layout, caretDisplay);
		const auto& line = layout.lines[lineIndex];
		const float caretX = metrics.HorizontalAdvance(
			std::u8string_view(layout.display.display).substr(
				line.displayStart,
				std::min(caretDisplay, line.displayEnd) - line.displayStart));
		const float caretTop = line.top - layout.contentRect.top;
		const float caretBottom = line.bottom - layout.contentRect.top;

		m_scrollX = std::clamp(m_scrollX, 0.0f, maxScrollX);
		m_scrollY = std::clamp(m_scrollY, 0.0f, maxScrollY);

		if (caretX - m_scrollX > contentWidth)
			m_scrollX = caretX - contentWidth;
		else if (caretX - m_scrollX < 0.0f)
			m_scrollX = caretX;

		if (caretBottom - m_scrollY > contentHeight)
			m_scrollY = caretBottom - contentHeight;
		else if (caretTop - m_scrollY < 0.0f)
			m_scrollY = caretTop;

		m_scrollX = std::clamp(m_scrollX, 0.0f, maxScrollX);
		m_scrollY = std::clamp(m_scrollY, 0.0f, maxScrollY);
	}

	void InputTextVisual::BuildVisuals()
	{
		InvalidateLayoutCache();
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
		if (!EnsureDocumentLayout())
			return;

		const auto& layout = m_layoutCache->layout;
		FontMetrics metrics(style->visualProps.font);
		const Color textColor = text_paint_utils::ResolveTextColor(style);
		const Color selectedTextColor = ResolveSelectionTextColor(style);
		const bool hasSelection = input->HasSelection() && input->PreeditText().empty();
		const size_t selStartDisplay = hasSelection
			? SourceToDisplayOffset(layout.display, input->SelectionStart())
			: 0;
		const size_t selEndDisplay = hasSelection
			? SourceToDisplayOffset(layout.display, input->SelectionEnd())
			: 0;

		canvas->save();
		canvas->clipRect(SkRect::MakeLTRB(
			layout.contentRect.left,
			layout.contentRect.top,
			layout.contentRect.right,
			layout.contentRect.bottom));

		SkPaint paint;
		paint.setAntiAlias(true);
		paint.setStyle(SkPaint::kFill_Style);

		const float originX = layout.contentRect.left - m_scrollX;
		const float originY = -m_scrollY;

		for (const auto& line : layout.lines)
		{
			const float baselineY = line.baselineY + originY;
			const float top = line.top + originY;
			const float bottom = line.bottom + originY;
			const std::u8string_view lineText = std::u8string_view(layout.display.display).substr(
				line.displayStart,
				line.displayEnd - line.displayStart);

			if (hasSelection)
			{
				const size_t segStart = std::max(line.displayStart, selStartDisplay);
				const size_t segEnd = std::min(line.displayEnd, selEndDisplay);
				if (segStart < segEnd)
				{
					const float x0 = originX + metrics.HorizontalAdvance(
						std::u8string_view(layout.display.display).substr(
							line.displayStart,
							segStart - line.displayStart));
					const float x1 = originX + metrics.HorizontalAdvance(
						std::u8string_view(layout.display.display).substr(
							line.displayStart,
							segEnd - line.displayStart));
					SkPaint selectionPaint;
					selectionPaint.setAntiAlias(true);
					selectionPaint.setStyle(SkPaint::kFill_Style);
					selectionPaint.setColor(skia_utils::ToSkColor(ResolveSelectionBackground(style)));
					canvas->drawRect(SkRect::MakeLTRB(x0, top, x1, bottom), selectionPaint);
				}
			}

			if (!hasSelection)
			{
				paint.setColor(skia_utils::ToSkColor(textColor));
				metrics.DrawText(canvas, lineText, originX, baselineY, paint);
			}
			else
			{
				size_t cursor = line.displayStart;
				auto drawSeg = [&](size_t end, const Color& color)
				{
					if (end <= cursor)
						return;
					const auto chunk = std::u8string_view(layout.display.display).substr(cursor, end - cursor);
					const float x = originX + metrics.HorizontalAdvance(
						std::u8string_view(layout.display.display).substr(
							line.displayStart,
							cursor - line.displayStart));
					paint.setColor(skia_utils::ToSkColor(color));
					metrics.DrawText(canvas, chunk, x, baselineY, paint);
					cursor = end;
				};

				drawSeg(std::min(line.displayEnd, selStartDisplay), textColor);
				drawSeg(std::min(line.displayEnd, selEndDisplay), selectedTextColor);
				drawSeg(line.displayEnd, textColor);
			}

			if (layout.display.preeditDisplayEnd > layout.display.preeditDisplayStart &&
				line.displayStart < layout.display.preeditDisplayEnd &&
				line.displayEnd > layout.display.preeditDisplayStart)
			{
				const size_t u0 = std::max(line.displayStart, layout.display.preeditDisplayStart);
				const size_t u1 = std::min(line.displayEnd, layout.display.preeditDisplayEnd);
				const float x0 = originX + metrics.HorizontalAdvance(
					std::u8string_view(layout.display.display).substr(
						line.displayStart,
						u0 - line.displayStart));
				const float x1 = originX + metrics.HorizontalAdvance(
					std::u8string_view(layout.display.display).substr(
						line.displayStart,
						u1 - line.displayStart));
				SkPaint underline = paint;
				underline.setColor(skia_utils::ToSkColor(textColor));
				underline.setStyle(SkPaint::kStroke_Style);
				underline.setStrokeWidth(std::max(metrics.UnderlineThickness(), 1.0f));
				canvas->drawLine(
					x0,
					baselineY + metrics.UnderlineOffset(),
					x1,
					baselineY + metrics.UnderlineOffset(),
					underline);
			}
		}

		const bool showCaret =
			input->States()->IsFocused() &&
			m_caretVisible &&
			!input->HasSelection();
		if (showCaret)
		{
			const size_t caretDisplay = layout.display.caretDisplayOffset;
			const size_t lineIndex = LineIndexForDisplayOffset(layout, caretDisplay);
			const auto& line = layout.lines[lineIndex];
			const float caretX = originX + metrics.HorizontalAdvance(
				std::u8string_view(layout.display.display).substr(
					line.displayStart,
					std::min(caretDisplay, line.displayEnd) - line.displayStart));
			const float top = line.top + originY;
			const float bottom = line.bottom + originY;
			paint.setColor(skia_utils::ToSkColor(textColor));
			SkPaint caretPaint = paint;
			caretPaint.setStyle(SkPaint::kStroke_Style);
			caretPaint.setStrokeWidth(1.0f);
			canvas->drawLine(caretX, top, caretX, bottom, caretPaint);
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
		if (!EnsureDocumentLayout())
			return;

		const auto& layout = m_layoutCache->layout;
		FontMetrics metrics(GetComputedStyle()->visualProps.font);
		// Prefer the full preedit span so Imm anchors to the composed text, not
		// only the trailing caret (which sits at preedit end).
		const bool hasPreedit =
			layout.display.preeditDisplayEnd > layout.display.preeditDisplayStart;
		const size_t anchorDisplay = hasPreedit
			? layout.display.preeditDisplayStart
			: layout.display.caretDisplayOffset;
		const size_t endDisplay = hasPreedit
			? layout.display.preeditDisplayEnd
			: layout.display.caretDisplayOffset;
		const size_t lineIndex = LineIndexForDisplayOffset(layout, anchorDisplay);
		const auto& line = layout.lines[lineIndex];
		const float originX = layout.contentRect.left - m_scrollX;
		const float originY = -m_scrollY;
		const float x0 = originX + metrics.HorizontalAdvance(
			std::u8string_view(layout.display.display).substr(
				line.displayStart,
				std::min(anchorDisplay, line.displayEnd) - line.displayStart));
		const float x1 = hasPreedit
			? originX + metrics.HorizontalAdvance(
				std::u8string_view(layout.display.display).substr(
					line.displayStart,
					std::min(endDisplay, line.displayEnd) - line.displayStart))
			: x0 + 1.0f;
		const float top = line.top + originY;
		const float bottom = line.bottom + originY;

		const PointF rootTopLeft = MapLocalToRoot(this, PointF(x0, top));
		const PointF rootBottomRight = MapLocalToRoot(this, PointF(x1, bottom));
		const RectF clientRect = view->Window()->ClientRect();
		ImeCaretRectEvent imeCaret(RectF(
			rootTopLeft.x + clientRect.left,
			rootTopLeft.y + clientRect.top,
			rootBottomRight.x + clientRect.left,
			rootBottomRight.y + clientRect.top));
		view->FireEvent(&imeCaret);
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

	size_t InputTextVisual::CaretIndexAtLocalPos(basic::single localX, basic::single localY) const
	{
		if (!EnsureDocumentLayout())
			return 0;

		const auto& layout = m_layoutCache->layout;
		FontMetrics metrics(GetComputedStyle()->visualProps.font);
		const float contentX = localX - layout.contentRect.left + m_scrollX;
		const float contentY = localY - layout.contentRect.top + m_scrollY;
		return CaretIndexFromContentPos(metrics, layout, contentX, contentY);
	}

	bool InputTextVisual::HandleMultiLineKey(KeyPressEvent* e)
	{
		auto* input = InputText();
		if (!input || !e || !input->IsMultiLine() || input->IsComposing())
			return false;

		const bool extend = (e->modifier & Modifier::Shift) != Modifier::None;
		const bool ctrl = (e->modifier & Modifier::Ctrl) != Modifier::None;

		if (!EnsureDocumentLayout())
			return false;

		const auto& layout = m_layoutCache->layout;
		FontMetrics metrics(GetComputedStyle()->visualProps.font);
		const size_t caretDisplay = SourceToDisplayOffset(layout.display, input->CaretIndex());
		const size_t lineIndex = LineIndexForDisplayOffset(layout, caretDisplay);
		const auto& line = layout.lines[lineIndex];

		auto moveToDisplay = [&](size_t displayOffset)
		{
			const size_t source = DisplayToSourceOffset(layout.display, displayOffset);
			if (extend)
				input->SetSelection(input->AnchorIndex(), source);
			else
				input->SetSelection(source, source);
		};

		switch (e->key)
		{
		case Key::Home:
			if (ctrl)
				return false;
			moveToDisplay(line.displayStart);
			return true;
		case Key::End:
			if (ctrl)
				return false;
			moveToDisplay(line.displayEnd);
			return true;
		case Key::Up:
		case Key::Down:
		{
			if (lineIndex == 0 && e->key == Key::Up)
			{
				moveToDisplay(layout.lines.front().displayStart);
				return true;
			}
			if (lineIndex + 1 >= layout.lines.size() && e->key == Key::Down)
			{
				moveToDisplay(layout.lines.back().displayEnd);
				return true;
			}

			const float caretX = metrics.HorizontalAdvance(
				std::u8string_view(layout.display.display).substr(
					line.displayStart,
					std::min(caretDisplay, line.displayEnd) - line.displayStart));
			const size_t targetLine = e->key == Key::Up ? lineIndex - 1 : lineIndex + 1;
			const auto& dest = layout.lines[targetLine];
			const float contentY =
				dest.top + layout.lineHeight * 0.5f - layout.contentRect.top;
			const size_t source = CaretIndexFromContentPos(metrics, layout, caretX, contentY);
			if (extend)
				input->SetSelection(input->AnchorIndex(), source);
			else
				input->SetSelection(source, source);
			return true;
		}
		default:
			return false;
		}
	}

	void InputTextVisual::HandleMousePress(MousePressEvent* e)
	{
		auto* input = InputText();
		if (!input || !e || e->button != MouseButton::Left || input->IsComposing())
			return;

		const size_t index = CaretIndexAtLocalPos(e->pos.x, e->pos.y);
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

		input->SetSelection(input->AnchorIndex(), CaretIndexAtLocalPos(e->pos.x, e->pos.y));
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
				InvalidateLayoutCache();
				Reflow();
				EnsureCaretVisible();
				if (input->States()->IsFocused())
					ResetCaretBlink();
				else
					Repaint();
				return false;
			case Type::ElementStatesChanged:
				InvalidateLayoutCache();
				SyncCaretBlink(input->States()->IsFocused());
				// Fall through so Visual::Update() rebuilds focused/hovered styles.
				break;
			case Type::KeyPress:
				if (HandleMultiLineKey(static_cast<KeyPressEvent*>(e)))
				{
					EnsureCaretVisible();
					ResetCaretBlink();
					return true;
				}
				return false;
			case Type::MousePress:
				HandleMousePress(static_cast<MousePressEvent*>(e));
				e->Accept();
				return false;
			case Type::MouseMove:
				if (m_dragging)
				{
					HandleMouseMove(static_cast<MouseMoveEvent*>(e));
					e->Accept();
				}
				return false;
			case Type::MouseRelease:
				if (m_dragging || input->States()->IsFocused())
				{
					HandleMouseRelease(static_cast<MouseReleaseEvent*>(e));
					e->Accept();
				}
				return false;
			default:
				break;
			}
		}

		return Visual::Filter(target, e);
	}
}

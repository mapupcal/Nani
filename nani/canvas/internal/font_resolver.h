#pragma once

#include "defs.h"
#include "skia_defs.h"

#include "../text/font.h"

#include <span>
#include <vector>

namespace nani::canvas::internal::font_resolver
{
	// Resolve glyph runs: Font.families -> platform fallbacks -> matchFamilyStyleCharacter.
	float Measure(
		const SkFont& baseFont,
		SkFontMgr* fontMgr,
		std::span<const std::u8string> preferredFamilies,
		std::span<const std::u8string> fallbackFamilies,
		const std::u8string_view& text);

	float Measure(
		const SkFont& baseFont,
		SkFontMgr* fontMgr,
		const text::Font& font,
		std::span<const std::u8string> fallbackFamilies,
		const std::u8string_view& text);

	void Draw(
		SkCanvas* canvas,
		const SkFont& baseFont,
		SkFontMgr* fontMgr,
		std::span<const std::u8string> preferredFamilies,
		std::span<const std::u8string> fallbackFamilies,
		const std::u8string_view& text,
		float x,
		float y,
		const SkPaint& paint);

	void Draw(
		SkCanvas* canvas,
		const SkFont& baseFont,
		SkFontMgr* fontMgr,
		const text::Font& font,
		std::span<const std::u8string> fallbackFamilies,
		const std::u8string_view& text,
		float x,
		float y,
		const SkPaint& paint);
}

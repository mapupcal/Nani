#include "font_resolver.h"
#include "../text/utf8.h"

namespace nani::canvas::internal::font_resolver
{
	namespace
	{
		size_t NextUnichar(const char* data, size_t size, size_t index, SkUnichar& out)
		{
			char32_t codepoint = 0;
			const size_t next = text::utf8::DecodeNext(data, size, index, codepoint);
			out = static_cast<SkUnichar>(codepoint);
			return next;
		}

		const char* FamilyCStr(const std::u8string& family, std::string& storage)
		{
			storage.assign(family.begin(), family.end());
			return storage.c_str();
		}

		sk_sp<SkTypeface> MatchFamily(
			SkFontMgr* fontMgr,
			const std::u8string& family,
			const SkFontStyle& style)
		{
			if (!fontMgr || family.empty())
				return nullptr;

			std::string storage;
			const char* name = FamilyCStr(family, storage);
			sk_sp<SkTypeface> face = fontMgr->matchFamilyStyle(name, style);
			if (!face)
				face = fontMgr->matchFamilyStyle(name, SkFontStyle::Normal());
			return face;
		}

		bool TypefaceHasGlyph(SkTypeface* face, SkUnichar uni)
		{
			return face && face->unicharToGlyph(uni) != 0;
		}

		void CollectTypefaces(
			SkFontMgr* fontMgr,
			std::span<const std::u8string> families,
			const SkFontStyle& style,
			std::vector<sk_sp<SkTypeface>>& out)
		{
			for (const auto& family : families)
			{
				if (sk_sp<SkTypeface> face = MatchFamily(fontMgr, family, style))
					out.push_back(std::move(face));
			}
		}

		sk_sp<SkTypeface> ResolveTypefaceForUnichar(
			SkFontMgr* fontMgr,
			SkTypeface* primary,
			const std::vector<sk_sp<SkTypeface>>& chain,
			const char* hintFamilyName,
			const SkFontStyle& style,
			SkUnichar uni)
		{
			if (TypefaceHasGlyph(primary, uni))
				return sk_ref_sp(primary);

			for (const auto& face : chain)
			{
				if (TypefaceHasGlyph(face.get(), uni))
					return face;
			}

			if (fontMgr)
			{
				if (sk_sp<SkTypeface> face =
					fontMgr->matchFamilyStyleCharacter(hintFamilyName, style, nullptr, 0, uni))
				{
					return face;
				}
				if (hintFamilyName)
				{
					if (sk_sp<SkTypeface> face =
						fontMgr->matchFamilyStyleCharacter(nullptr, style, nullptr, 0, uni))
					{
						return face;
					}
				}
			}

			return sk_ref_sp(primary);
		}

		template <typename RunFn>
		void ForEachRun(
			const SkFont& baseFont,
			SkFontMgr* fontMgr,
			std::span<const std::u8string> preferredFamilies,
			std::span<const std::u8string> fallbackFamilies,
			const std::u8string_view& text,
			RunFn&& runFn)
		{
			if (text.empty())
				return;

			const char* data = reinterpret_cast<const char*>(text.data());
			const size_t size = text.size();
			SkTypeface* primary = baseFont.getTypeface();
			const SkFontStyle style = primary ? primary->fontStyle() : SkFontStyle::Normal();

			std::vector<sk_sp<SkTypeface>> chain;
			chain.reserve(preferredFamilies.size() + fallbackFamilies.size());
			CollectTypefaces(fontMgr, preferredFamilies, style, chain);
			CollectTypefaces(fontMgr, fallbackFamilies, style, chain);

			std::string hintStorage;
			const char* hintFamilyName = nullptr;
			if (!preferredFamilies.empty())
				hintFamilyName = FamilyCStr(preferredFamilies.front(), hintStorage);

			size_t index = 0;
			SkUnichar firstUni = 0;
			size_t next = NextUnichar(data, size, index, firstUni);
			sk_sp<SkTypeface> runFace = ResolveTypefaceForUnichar(
				fontMgr,
				primary,
				chain,
				hintFamilyName,
				style,
				firstUni);
			size_t runStart = index;
			index = next;

			auto flush = [&](size_t runEnd)
			{
				if (runEnd <= runStart)
					return;
				SkFont runFont = baseFont;
				if (runFace)
					runFont.setTypeface(runFace);
				runFn(runFont, data + runStart, runEnd - runStart);
			};

			while (index < size)
			{
				SkUnichar uni = 0;
				next = NextUnichar(data, size, index, uni);
				sk_sp<SkTypeface> face = ResolveTypefaceForUnichar(
					fontMgr,
					primary,
					chain,
					hintFamilyName,
					style,
					uni);
				const bool sameFace =
					(runFace && face && runFace->uniqueID() == face->uniqueID()) ||
					(!runFace && !face);
				if (!sameFace)
				{
					flush(index);
					runFace = std::move(face);
					runStart = index;
				}
				index = next;
			}
			flush(size);
		}
	}

	float Measure(
		const SkFont& baseFont,
		SkFontMgr* fontMgr,
		std::span<const std::u8string> preferredFamilies,
		std::span<const std::u8string> fallbackFamilies,
		const std::u8string_view& text)
	{
		float width = 0.0f;
		ForEachRun(
			baseFont,
			fontMgr,
			preferredFamilies,
			fallbackFamilies,
			text,
			[&](const SkFont& font, const char* bytes, size_t byteCount)
			{
				width += font.measureText(bytes, byteCount, SkTextEncoding::kUTF8);
			});
		return width;
	}

	void Draw(
		SkCanvas* canvas,
		const SkFont& baseFont,
		SkFontMgr* fontMgr,
		std::span<const std::u8string> preferredFamilies,
		std::span<const std::u8string> fallbackFamilies,
		const std::u8string_view& text,
		float x,
		float y,
		const SkPaint& paint)
	{
		if (!canvas || text.empty())
			return;

		float cursorX = x;
		ForEachRun(
			baseFont,
			fontMgr,
			preferredFamilies,
			fallbackFamilies,
			text,
			[&](const SkFont& font, const char* bytes, size_t byteCount)
			{
				canvas->drawSimpleText(
					bytes,
					byteCount,
					SkTextEncoding::kUTF8,
					cursorX,
					y,
					font,
					paint);
				cursorX += font.measureText(bytes, byteCount, SkTextEncoding::kUTF8);
			});
	}
}

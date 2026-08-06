#include "font_resolver.h"
#include "skia_utils.h"
#include "../text/utf8.h"

#include <algorithm>
#include <unordered_map>

namespace nani::canvas::internal::font_resolver
{
	namespace
	{
		constexpr size_t kMaxUnicharCacheEntries = 20000;
		constexpr size_t kMaxChainCacheEntries = 256;

		size_t NextUnichar(const char* data, size_t size, size_t index, SkUnichar& out)
		{
			char32_t codepoint = 0;
			const size_t next = text::utf8::DecodeNext(data, size, index, codepoint);
			out = static_cast<SkUnichar>(codepoint);
			return next;
		}

		bool TypefaceHasGlyph(SkTypeface* face, SkUnichar uni)
		{
			return face && face->unicharToGlyph(uni) != 0;
		}

		size_t MixHash(size_t seed, size_t value)
		{
			return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
		}

		size_t HashFamilies(
			std::span<const std::u8string> preferredFamilies,
			std::span<const std::u8string> fallbackFamilies)
		{
			size_t hash = 0;
			for (const auto& family : preferredFamilies)
				hash = MixHash(hash, std::hash<std::u8string>{}(family));
			hash = MixHash(hash, 0x9e3779b9u);
			for (const auto& family : fallbackFamilies)
				hash = MixHash(hash, std::hash<std::u8string>{}(family));
			return hash;
		}

		uint32_t StyleBits(const SkFontStyle& style)
		{
			return (static_cast<uint32_t>(style.weight()) & 0xFFFFu)
				| ((static_cast<uint32_t>(style.width()) & 0xFFu) << 16)
				| ((static_cast<uint32_t>(style.slant()) & 0xFFu) << 24);
		}

		uint64_t UnicharCacheKey(
			uint32_t primaryId,
			uint32_t styleBits,
			size_t familiesHash,
			SkUnichar uni)
		{
			uint64_t key = primaryId;
			key = (key << 32) ^ (static_cast<uint64_t>(styleBits) << 16) ^ familiesHash;
			key ^= static_cast<uint64_t>(static_cast<uint32_t>(uni)) * 0x9e3779b97f4a7c15ULL;
			return key;
		}

		void CollectTypefaces(
			SkFontMgr* fontMgr,
			std::span<const std::u8string> families,
			const SkFontStyle& style,
			std::vector<sk_sp<SkTypeface>>& out)
		{
			for (const auto& family : families)
			{
				if (sk_sp<SkTypeface> face = skia_utils::MatchFamilyStyle(fontMgr, family, style))
					out.push_back(std::move(face));
			}
		}

		struct TypefaceChain
		{
			uint32_t styleBits = 0;
			size_t familiesHash = 0;
			std::vector<std::u8string> preferred;
			std::vector<std::u8string> fallback;
			std::vector<sk_sp<SkTypeface>> faces;
			std::string hintFamily;
		};

		struct ResolverCaches
		{
			std::unordered_map<size_t, TypefaceChain> chains;
			std::unordered_map<uint64_t, sk_sp<SkTypeface>> unicharFaces;

			void Clear()
			{
				chains.clear();
				unicharFaces.clear();
			}
		};

		ResolverCaches& Caches()
		{
			static ResolverCaches caches;
			return caches;
		}

		size_t ChainMapKey(uint32_t styleBits, size_t familiesHash)
		{
			return MixHash(static_cast<size_t>(styleBits), familiesHash);
		}

		const TypefaceChain& GetTypefaceChain(
			SkFontMgr* fontMgr,
			std::span<const std::u8string> preferredFamilies,
			std::span<const std::u8string> fallbackFamilies,
			const SkFontStyle& style)
		{
			const uint32_t styleBits = StyleBits(style);
			const size_t familiesHash = HashFamilies(preferredFamilies, fallbackFamilies);
			const size_t mapKey = ChainMapKey(styleBits, familiesHash);

			auto& caches = Caches();
			if (auto it = caches.chains.find(mapKey); it != caches.chains.end())
			{
				const TypefaceChain& cached = it->second;
				if (cached.styleBits == styleBits &&
					cached.familiesHash == familiesHash &&
					cached.preferred.size() == preferredFamilies.size() &&
					cached.fallback.size() == fallbackFamilies.size() &&
					std::equal(
						cached.preferred.begin(),
						cached.preferred.end(),
						preferredFamilies.begin()) &&
					std::equal(
						cached.fallback.begin(),
						cached.fallback.end(),
						fallbackFamilies.begin()))
				{
					return cached;
				}
			}

			TypefaceChain chain;
			chain.styleBits = styleBits;
			chain.familiesHash = familiesHash;
			chain.preferred.assign(preferredFamilies.begin(), preferredFamilies.end());
			chain.fallback.assign(fallbackFamilies.begin(), fallbackFamilies.end());
			chain.faces.reserve(preferredFamilies.size() + fallbackFamilies.size());
			CollectTypefaces(fontMgr, preferredFamilies, style, chain.faces);
			CollectTypefaces(fontMgr, fallbackFamilies, style, chain.faces);
			if (!preferredFamilies.empty())
			{
				const SkString hint = skia_utils::ToSkString(preferredFamilies.front());
				chain.hintFamily.assign(hint.c_str(), hint.size());
			}

			if (caches.chains.size() >= kMaxChainCacheEntries &&
				caches.chains.find(mapKey) == caches.chains.end())
			{
				caches.chains.clear();
				caches.unicharFaces.clear();
			}

			auto [it, inserted] = caches.chains.insert_or_assign(mapKey, std::move(chain));
			(void)inserted;
			return it->second;
		}

		sk_sp<SkTypeface> ResolveTypefaceForUnichar(
			SkFontMgr* fontMgr,
			SkTypeface* primary,
			const TypefaceChain& chain,
			const SkFontStyle& style,
			SkUnichar uni)
		{
			const uint32_t primaryId = primary ? primary->uniqueID() : 0;
			const uint64_t cacheKey = UnicharCacheKey(
				primaryId,
				chain.styleBits,
				chain.familiesHash,
				uni);

			auto& caches = Caches();
			if (auto it = caches.unicharFaces.find(cacheKey); it != caches.unicharFaces.end())
				return it->second;

			sk_sp<SkTypeface> resolved;
			if (TypefaceHasGlyph(primary, uni))
			{
				resolved = sk_ref_sp(primary);
			}
			else
			{
				for (const auto& face : chain.faces)
				{
					if (TypefaceHasGlyph(face.get(), uni))
					{
						resolved = face;
						break;
					}
				}
			}

			if (!resolved && fontMgr)
			{
				const char* hintFamilyName =
					chain.hintFamily.empty() ? nullptr : chain.hintFamily.c_str();
				resolved = fontMgr->matchFamilyStyleCharacter(
					hintFamilyName,
					style,
					nullptr,
					0,
					uni);
				if (!resolved && hintFamilyName)
				{
					resolved = fontMgr->matchFamilyStyleCharacter(
						nullptr,
						style,
						nullptr,
						0,
						uni);
				}
			}

			if (!resolved)
				resolved = sk_ref_sp(primary);

			if (caches.unicharFaces.size() >= kMaxUnicharCacheEntries)
				caches.unicharFaces.clear();
			caches.unicharFaces.emplace(cacheKey, resolved);
			return resolved;
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
			const TypefaceChain& chain = GetTypefaceChain(
				fontMgr,
				preferredFamilies,
				fallbackFamilies,
				style);

			size_t index = 0;
			SkUnichar firstUni = 0;
			size_t next = NextUnichar(data, size, index, firstUni);
			sk_sp<SkTypeface> runFace = ResolveTypefaceForUnichar(
				fontMgr,
				primary,
				chain,
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

		std::span<const std::u8string> AsSpan(const std::vector<std::u8string>& families)
		{
			return std::span<const std::u8string>(families.data(), families.size());
		}
	}

	void ClearCaches()
	{
		Caches().Clear();
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

	float Measure(
		const SkFont& baseFont,
		SkFontMgr* fontMgr,
		const text::Font& font,
		std::span<const std::u8string> fallbackFamilies,
		const std::u8string_view& text)
	{
		return Measure(
			baseFont,
			fontMgr,
			AsSpan(font.Families()),
			fallbackFamilies,
			text);
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

	void Draw(
		SkCanvas* canvas,
		const SkFont& baseFont,
		SkFontMgr* fontMgr,
		const text::Font& font,
		std::span<const std::u8string> fallbackFamilies,
		const std::u8string_view& text,
		float x,
		float y,
		const SkPaint& paint)
	{
		Draw(
			canvas,
			baseFont,
			fontMgr,
			AsSpan(font.Families()),
			fallbackFamilies,
			text,
			x,
			y,
			paint);
	}
}

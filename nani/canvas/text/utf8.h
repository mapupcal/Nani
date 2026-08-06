#pragma once

#include "../defs.h"

namespace nani::canvas::text::utf8
{
	inline bool IsContinuationByte(unsigned char byte)
	{
		return (byte & 0xC0) == 0x80;
	}

	inline size_t CodepointByteLength(unsigned char lead)
	{
		if ((lead & 0x80) == 0)
			return 1;
		if ((lead & 0xE0) == 0xC0)
			return 2;
		if ((lead & 0xF0) == 0xE0)
			return 3;
		if ((lead & 0xF8) == 0xF0)
			return 4;
		return 1;
	}

	// Floor to the UTF-8 codepoint start at or before index.
	inline size_t AlignBoundary(const char* data, size_t index, size_t length)
	{
		if (index >= length)
			return length;
		while (index > 0 && IsContinuationByte(static_cast<unsigned char>(data[index])))
			--index;
		return index;
	}

	inline size_t AlignBoundary(std::u8string_view text, size_t index)
	{
		return AlignBoundary(
			reinterpret_cast<const char*>(text.data()),
			index,
			text.size());
	}

	inline size_t NextIndex(const char* data, size_t index, size_t length)
	{
		if (index >= length)
			return length;

		const size_t len = CodepointByteLength(static_cast<unsigned char>(data[index]));
		return std::min(index + len, length);
	}

	inline size_t NextIndex(std::u8string_view text, size_t index)
	{
		return NextIndex(
			reinterpret_cast<const char*>(text.data()),
			index,
			text.size());
	}

	inline size_t PrevIndex(const char* data, size_t index)
	{
		if (index == 0)
			return 0;

		size_t i = index - 1;
		while (i > 0 && IsContinuationByte(static_cast<unsigned char>(data[i])))
			--i;
		return i;
	}

	inline size_t PrevIndex(std::u8string_view text, size_t index)
	{
		return PrevIndex(reinterpret_cast<const char*>(text.data()), index);
	}

	// Decode one codepoint at index. Returns the byte index after it.
	inline size_t DecodeNext(const char* data, size_t size, size_t index, char32_t& out)
	{
		if (index >= size)
		{
			out = 0;
			return size;
		}

		const unsigned char lead = static_cast<unsigned char>(data[index]);
		if ((lead & 0x80) == 0)
		{
			out = lead;
			return index + 1;
		}
		if ((lead & 0xE0) == 0xC0 && index + 1 < size)
		{
			out = (static_cast<char32_t>(lead & 0x1F) << 6)
				| (static_cast<unsigned char>(data[index + 1]) & 0x3F);
			return index + 2;
		}
		if ((lead & 0xF0) == 0xE0 && index + 2 < size)
		{
			out = (static_cast<char32_t>(lead & 0x0F) << 12)
				| (static_cast<char32_t>(static_cast<unsigned char>(data[index + 1]) & 0x3F) << 6)
				| (static_cast<unsigned char>(data[index + 2]) & 0x3F);
			return index + 3;
		}
		if ((lead & 0xF8) == 0xF0 && index + 3 < size)
		{
			out = (static_cast<char32_t>(lead & 0x07) << 18)
				| (static_cast<char32_t>(static_cast<unsigned char>(data[index + 1]) & 0x3F) << 12)
				| (static_cast<char32_t>(static_cast<unsigned char>(data[index + 2]) & 0x3F) << 6)
				| (static_cast<unsigned char>(data[index + 3]) & 0x3F);
			return index + 4;
		}

		out = lead;
		return index + 1;
	}

	inline size_t DecodeNext(std::u8string_view text, size_t index, char32_t& out)
	{
		return DecodeNext(
			reinterpret_cast<const char*>(text.data()),
			text.size(),
			index,
			out);
	}

	inline std::u8string Encode(char32_t codepoint)
	{
		char8_t bytes[4] = {};
		size_t count = 0;
		if (codepoint <= 0x7F)
		{
			bytes[0] = static_cast<char8_t>(codepoint);
			count = 1;
		}
		else if (codepoint <= 0x7FF)
		{
			bytes[0] = static_cast<char8_t>(0xC0 | ((codepoint >> 6) & 0x1F));
			bytes[1] = static_cast<char8_t>(0x80 | (codepoint & 0x3F));
			count = 2;
		}
		else if (codepoint <= 0xFFFF)
		{
			bytes[0] = static_cast<char8_t>(0xE0 | ((codepoint >> 12) & 0x0F));
			bytes[1] = static_cast<char8_t>(0x80 | ((codepoint >> 6) & 0x3F));
			bytes[2] = static_cast<char8_t>(0x80 | (codepoint & 0x3F));
			count = 3;
		}
		else
		{
			bytes[0] = static_cast<char8_t>(0xF0 | ((codepoint >> 18) & 0x07));
			bytes[1] = static_cast<char8_t>(0x80 | ((codepoint >> 12) & 0x3F));
			bytes[2] = static_cast<char8_t>(0x80 | ((codepoint >> 6) & 0x3F));
			bytes[3] = static_cast<char8_t>(0x80 | (codepoint & 0x3F));
			count = 4;
		}
		return std::u8string(bytes, count);
	}
}

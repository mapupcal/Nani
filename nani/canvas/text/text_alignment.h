#pragma once
#include "text_defs.h"

namespace nani::canvas::text
{
	class NANI_CANVAS_API TextAlignment
	{
	public:
		enum class Horizontal : basic::byte
		{
			Left,
			Center,
			Right
		};

		enum class Vertical : basic::byte
		{
			Top,
			Center,
			Bottom
		};

	public:
		TextAlignment() = default;
		TextAlignment(const TextAlignment& other) = default;
		~TextAlignment() = default;

	public:
		TextAlignment& operator=(const TextAlignment& other) = default;

	public:
		Horizontal HorizontalAlign() const;
		void SetHorizontal(Horizontal align);

		Vertical VerticalAlign() const;
		void SetVertical(Vertical align);

	private:
		Horizontal m_horizontal = Horizontal::Left;
		Vertical m_vertical = Vertical::Top;
	};

	inline bool operator==(const TextAlignment& lhs, const TextAlignment& rhs)
	{
		return (lhs.HorizontalAlign() == rhs.HorizontalAlign()) &&
			(lhs.VerticalAlign() == rhs.VerticalAlign());
	}

	inline bool operator!=(const TextAlignment& lhs, const TextAlignment& rhs)
	{
		return !(lhs == rhs);
	}
}

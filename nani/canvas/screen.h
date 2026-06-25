#pragma once
#include "defs.h"
#include "basic/rectf.h"

namespace nani::canvas
{
	class NANI_CANVAS_API Screen
	{
	public:
		Screen();
		~Screen();

	public:
		const basic::RectF Rect() const;
		const basic::RectF WorkAreaRect() const;
		const basic::RectF Geometry() const;
		const basic::RectF WorkAreaGeometry() const;

		const float DPI() const;
		const int Width() const;
		const int Height() const;

	public:
		static const Screen* Primary();
		static std::vector<const Screen*> Screens();

	private:
		internal::ScreenData* m_pData = nullptr;
	};
}
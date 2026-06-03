#include "screen.h"

using namespace nani::canvas::basic;

namespace nani::canvas
{
	const RectF Screen::Rect() const
	{
		return RectF();
	}

	const RectF Screen::WorkAreaRect() const
	{
		return RectF();
	}

	const RectF Screen::Geometry() const
	{
		return RectF();
	}

	const RectF Screen::WorkAreaGeometry() const
	{
		return RectF();
	}

	const float Screen::DPI() const
	{
		return 0.0f;
	}

	const int Screen::Width() const
	{
		return 0;
	}

	const int Screen::Height() const
	{
		return 0;
	}

	const Screen* Screen::Primary()
	{
		return nullptr;
	}

	std::vector<const Screen*> Screen::Screens()
	{
		return std::vector<const Screen*>();
	}

	Screen::Screen()
	{

	}

	Screen::~Screen()
	{

	}
}
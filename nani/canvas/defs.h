#pragma once

#ifdef NANI_CANVAS_IMPLEMENTATION
#define NANI_IMPLEMENTATION
#endif

#include "../global_defs.h"

namespace nani::canvas::internal
{
	class WindowPrivate;
	struct ScreenData;
}
namespace nani::canvas
{
	class Window;
}
namespace nani::canvas::elements
{
	class Element;
	class ElementsLayer;
	class ElementStates;
	class ElementVisibility;
	class ComputedStyle;
	class Styles;
}
namespace nani::canvas::visuals
{
	class Visual;
	class View;
}
namespace nani::canvas::events
{
	class Event;
	class EventFilter;
	class EventTarget;
}

class SkCanvas;

struct YGNode;
using YGNodeRef = YGNode*;

#define NANI_CANVAS_API NANI_API

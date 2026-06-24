#pragma once
#include "defs.h"
#include <yoga/Yoga.h>
#include <yoga/style/Style.h>

namespace nani::canvas::internal::yoga_utils
{
	void SetYogaNodeStyle(YGNodeRef node, const facebook::yoga::Style& style);
}

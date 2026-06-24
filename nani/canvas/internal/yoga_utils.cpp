#include "env_p.h"
#include "yoga_utils.h"
#include <yoga/node/Node.h>

namespace nani::canvas::internal::yoga_utils
{
	using namespace facebook::yoga;
	void SetYogaNodeStyle(YGNodeRef nodeRef, const Style& style)
	{
		Node* node = static_cast<Node*>(nodeRef);
		node->setStyle(style);
	}
}


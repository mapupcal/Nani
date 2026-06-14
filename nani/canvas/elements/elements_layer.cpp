#include "elements_layer.h"
#include "element.h"
using namespace nani::canvas::events;

namespace nani::canvas::elements
{
	ElementsLayer::ElementsLayer(Element* owner)
		: m_pOwner(owner)
	{

	}

	ElementsLayer::~ElementsLayer()
	{
		m_bDestroying = true;
		for (Element* element : m_elements)
			delete element;
	}

	void ElementsLayer::AddElement(Element* element)
	{
		if (!element)
			return;

		if (element->Parent() == m_pOwner)
			return;

		Element* oldParent = element->Parent();
		if (oldParent)
			oldParent->Layer()->RemoveElement(element);

		element->m_pParent = m_pOwner;
		m_elements.push_back(element);

		ElementModifyEvent event(Type::ElementAdd, element);
		m_pOwner->FireEvent(&event);
	}

	void ElementsLayer::RemoveElement(Element * element)
	{
		if (m_bDestroying)
			return;

		if (!element)
			return;
		
		if (element->Parent() != m_pOwner)
			return;

		auto iter = std::find(m_elements.begin(), m_elements.end(), element);
		if (iter != m_elements.end())
		{
			(*iter)->m_pParent = nullptr;
			m_elements.erase(iter);

			ElementModifyEvent event(Type::ElementRemove, element);
			m_pOwner->FireEvent(&event);
		}
	}

	std::vector<Element*> ElementsLayer::Elements()
	{
		return m_elements;
	}
}
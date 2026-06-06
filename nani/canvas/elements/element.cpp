#include "element.h"
#include "elements_layer.h"
namespace nani::canvas::elements
{
	Element::Element(Element* parent)
	{
		if (!parent)
			return;
		parent->Layer()->AddElement(this);
	}

	Element::~Element()
	{
		delete m_pLayer;
		if(Parent())
			Parent()->Layer()->RemoveElement(this);
	}

	Element* Element::Parent()
	{
		return m_pParent;
	}

	std::vector<Element*> Element::Children()
	{
		if(!m_pLayer)
			return std::vector<Element*>();

		return Layer()->Elements();
	}

	ElementsLayer* Element::Layer()
	{
		if (!m_pLayer)
			m_pLayer = new ElementsLayer(this);
		return m_pLayer;
	}
}
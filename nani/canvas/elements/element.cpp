#include "element.h"
#include "elements_layer.h"
#include "element_states.h"
#include "../visuals/visual.h"
using namespace nani::canvas::visuals;

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
		delete m_pStates;
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

	ElementStates* Element::States()
	{
		if (!m_pStates)
			m_pStates = new ElementStates(this);
		return m_pStates;
	}

	std::shared_ptr<Visual> Element::CreateVisual()
	{
		return std::move(std::make_shared<Visual>(this));
	}
}
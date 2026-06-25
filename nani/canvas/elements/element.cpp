#include "element.h"
#include "elements_layer.h"
#include "element_states.h"
#include "element_visibility.h"
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
		delete m_pVisibility;
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

	void Element::SetStyles(std::shared_ptr<Styles> styles)
	{
		m_spStyles = styles;
	}

	Styles* Element::GetStyles()
	{
		if (m_spStyles)
			return m_spStyles.get();

		auto parent = Parent();
		while (parent)
		{
			if (parent->m_spStyles)
				return parent->m_spStyles.get();
			parent = parent->Parent();
		}

		return nullptr;
	}

	void Element::SetStyleClass(const std::u8string_view& styleClass)
	{
		m_styleClass = styleClass;
	}

	const std::u8string_view Element::StyleClass() const
	{
		return m_styleClass;
	}

	ElementStates* Element::States()
	{
		if (!m_pStates)
			m_pStates = new ElementStates(this);
		return m_pStates;
	}

	ElementVisibility* Element::Visibility()
	{
		if (!m_pVisibility)
			m_pVisibility = new ElementVisibility(this);
		return m_pVisibility;
	}

	std::shared_ptr<Visual> Element::CreateVisual(View* view, Visual* visualParent)
	{
		return std::make_shared<Visual>(view, this, visualParent);
	}
}
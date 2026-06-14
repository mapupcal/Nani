#include "visual.h"
#include "../elements/element.h"

using namespace nani::canvas::elements;
using namespace nani::canvas::events;

namespace nani::canvas::visuals
{
	Visual::Visual(elements::Element* element)
		: m_pElement(element)
	{
		if (m_pElement)
			m_pElement->RegisterEventFilter(this);
	}

	Visual::~Visual()
	{
		if (m_pElement)
			m_pElement->UnRegisterEventFilter(this);
	}

	elements::Element* Visual::Element() const
	{
		return m_pElement;
	}

	std::vector<std::shared_ptr<Visual>> Visual::Visuals() const
	{
		return m_visuals;
	}

	void Visual::BuildVisuals()
	{
		if (!m_pElement)
			return;
		if (!m_visuals.empty())
			return;
		auto elements = m_pElement->Children();
		for (elements::Element* element : elements)
		{
			auto visual = element->CreateVisual();
			visual->BuildVisuals();
			m_visuals.push_back(visual);
		}
	}

	void Visual::Relayout()
	{
		//TODO:
	}

	bool Visual::Filter(events::EventTarget* target, events::Event* e)
	{
		if (target == m_pElement)
		{
			if (e->type == Type::ElementAdd)
			{
				ElementModifyEvent* event = static_cast<ElementModifyEvent*>(e);
				auto visual = event->element->CreateVisual();
				visual->BuildVisuals();
				m_visuals.push_back(visual);
				Relayout();
			}
			else if (e->type == Type::ElementRemove)
			{
				ElementModifyEvent* event = static_cast<ElementModifyEvent*>(e);
				auto iter = std::find_if(m_visuals.cbegin(), m_visuals.cend(),
					[=](const std::shared_ptr<Visual>& visual) {
						return visual->Element() == event->element;
					});

				m_visuals.erase(iter);
				Relayout();
			}
			else if (e->type == Type::ElementStateChanged)
			{
				ElementStatesChangedEvent* event = static_cast<ElementStatesChangedEvent*>(e);
				Relayout();
			}
		}
		return false;
	}
}
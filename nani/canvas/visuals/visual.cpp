#include "visual.h"
#include "../elements/element.h"
#include "../elements/styles.h"
#include <yoga/Yoga.h>

using namespace nani::canvas::elements;
using namespace nani::canvas::events;

namespace nani::canvas::visuals
{
	Visual::Visual(elements::Element* element, Visual* parent)
		: m_pElement(element)
		, m_pParent(parent)
	{
		if (m_pElement)
			m_pElement->RegisterEventFilter(this);
	}

	Visual::~Visual()
	{
		if (m_pElement)
			m_pElement->UnRegisterEventFilter(this);

		if (m_yogaNode)
		{
			if (Parent() && Parent()->m_yogaNode)
			{
				auto parentYogaNode = Parent()->m_yogaNode;
				YGNodeRemoveChild(parentYogaNode, m_yogaNode);
			}
			YGNodeFree(m_yogaNode);
		}
	}

	Visual* Visual::Parent()
	{
		return m_pParent;
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
		if (!m_yogaNode)
			return;
		m_yogaNode = YGNodeNew();
		if (Parent() && Parent()->m_yogaNode)
		{
			auto parentYogaNode = Parent()->m_yogaNode;
			YGNodeInsertChild(parentYogaNode, m_yogaNode, YGNodeGetChildCount(parentYogaNode));
		}
		auto elements = m_pElement->Children();
		for (elements::Element* element : elements)
		{
			auto visual = element->CreateVisual(this);
			visual->BuildVisuals();
			m_visuals.push_back(visual);
		}
	}

	void Visual::Update()
	{
		ComputedStyle* newComputedStyle = Element()->GetStyles()->Compute(Element());
		auto diff = newComputedStyle->Diff(m_pComputedStyle);
		m_pComputedStyle = newComputedStyle;

		if (diff.layoutChanged)
			Reflow();
		if (diff.visualChanged)
			Repaint();
	}

	void Visual::Reflow()
	{
		
	}

	void Visual::Repaint()
	{
		
	}

	bool Visual::Filter(events::EventTarget* target, events::Event* e)
	{
		if (target == m_pElement)
		{
			if (e->type == Type::ElementAdd)
			{
				ElementModifyEvent* event = static_cast<ElementModifyEvent*>(e);
				auto visual = event->element->CreateVisual(this);
				visual->BuildVisuals();
				m_visuals.push_back(visual);
				Reflow();
			}
			else if (e->type == Type::ElementRemove)
			{
				ElementModifyEvent* event = static_cast<ElementModifyEvent*>(e);
				auto iter = std::find_if(m_visuals.cbegin(), m_visuals.cend(),
					[=](const std::shared_ptr<Visual>& visual) {
						return visual->Element() == event->element;
					});

				m_visuals.erase(iter);
				Reflow();
			}
			else if (e->type == Type::ElementStatesChanged)
			{
				ElementStatesChangedEvent* event = static_cast<ElementStatesChangedEvent*>(e);
				Update();
			}
			else if (e->type == Type::ElementVisibilityChanged)
			{
				ElementVisibilityChangedEvent* event = static_cast<ElementVisibilityChangedEvent*>(e);
				Update();
			}
		}
		return false;
	}
}
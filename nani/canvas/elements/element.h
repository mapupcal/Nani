#pragma once
#include "elements_defs.h"
#include "properties.h"
namespace nani::canvas::visuals
{
	class Visual;
}
namespace nani::canvas::elements
{
	class ElementStates;
	class ElementVisibility;

	class NANI_CANVAS_API Element : public events::EventTarget
	{
	public:
		Element(Element* parent);
		Element(const Element&) = delete;
		virtual ~Element();

	public:
		Element* Parent();
		std::vector<Element*> Children();
		ElementsLayer* Layer();

	public:
		ElementStates* States();
		ElementVisibility* Visibility();
		std::shared_ptr<visuals::Visual> CreateVisual();

	private:
		friend class ElementsLayer;
		Element* m_pParent = nullptr;
		ElementsLayer* m_pLayer = nullptr;
		ElementStates* m_pStates = nullptr;
		ElementVisibility* m_pVisibility = nullptr;
	};
}
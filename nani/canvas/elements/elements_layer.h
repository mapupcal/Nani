#pragma once
#include "elements_defs.h"
namespace nani::canvas::elements
{
	class NANI_API ElementsLayer
	{
	public:
		ElementsLayer(Element* owner);
		ElementsLayer(const ElementsLayer&) = delete;
		~ElementsLayer();

	public:
		void AddElement(Element* element);
		void RemoveElement(Element* element);
		std::vector<Element*> Elements();

	private:
		bool m_bDestroying = false;
		Element* m_pOwner = nullptr;
		std::vector<Element*> m_elements;
	};
}
#pragma once
#include "visuals_defs.h"
namespace nani::canvas::visuals
{
	class NANI_CANVAS_API Visual : public events::EventFilter
	{
	public:
		Visual(elements::Element* element, Visual* parent);
		virtual ~Visual();

	public:
		Visual* Parent();
		elements::Element* Element() const;
		std::vector<std::shared_ptr<Visual>> Visuals() const;

		virtual void BuildVisuals();
		virtual void Relayout();

	private:
		virtual bool Filter(events::EventTarget* target, events::Event* e) override;

	private:
		elements::Element* m_pElement = nullptr;
		Visual* m_pParent = nullptr;
		std::vector<std::shared_ptr<Visual>> m_visuals;
	};
}
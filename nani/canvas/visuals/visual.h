#pragma once
#include "visuals_defs.h"
namespace nani::canvas::visuals
{
	class NANI_CANVAS_API Visual : public events::EventFilter
	{
	public:
		Visual(elements::Element* element);
		virtual ~Visual();

	public:
		elements::Element* Element() const;
		virtual void BuildVisuals();
		virtual void Relayout();
	private:
		virtual bool Filter(events::EventTarget* target, events::Event* e) override;

	private:
		elements::Element* m_pElement = nullptr;
		std::vector<std::shared_ptr<Visual>> m_visuals;
	};
}
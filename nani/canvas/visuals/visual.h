#pragma once
#include "visuals_defs.h"
struct YGNode;
namespace nani::canvas::elements
{
	struct ComputedStyle;
}
namespace nani::canvas::visuals
{
	using YGNodeRef = YGNode*;
	class NANI_CANVAS_API Visual : public events::EventFilter
	{
	public:
		Visual(elements::Element* element, Visual* parent);
		virtual ~Visual();

	public:
		Visual* Parent();
		elements::Element* Element() const;
		std::vector<std::shared_ptr<Visual>> Visuals() const;

		void BuildVisuals();
		void Update();
		void Reflow();
		void Repaint();

	private:
		virtual bool Filter(events::EventTarget* target, events::Event* e) override;

	private:
		elements::Element* m_pElement = nullptr;
		Visual* m_pParent = nullptr;
		std::vector<std::shared_ptr<Visual>> m_visuals;
		YGNodeRef m_yogaNode = nullptr;
		elements::ComputedStyle* m_pComputedStyle = nullptr;
		basic::RectF m_frame;
	};
}
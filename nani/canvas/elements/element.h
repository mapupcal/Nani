#pragma once
#include "elements_defs.h"
namespace nani::canvas::visuals
{
	class Visual;
}
namespace nani::canvas::elements
{
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

		void SetStyles(std::shared_ptr<Styles> styles);
		Styles* GetStyles();
		void SetStyleClass(std::u8string_view& styleClass);
		const std::u8string_view SylteClass() const;

		ElementStates* States();
		ElementVisibility* Visibility();
		virtual std::shared_ptr<visuals::Visual> CreateVisual(visuals::Visual* visualParent);

	private:
		friend class ElementsLayer;
		Element* m_pParent = nullptr;
		ElementsLayer* m_pLayer = nullptr;
		ElementStates* m_pStates = nullptr;
		ElementVisibility* m_pVisibility = nullptr;
		std::shared_ptr<Styles> m_spStyles;
		std::u8string m_styleClass;
	};
}
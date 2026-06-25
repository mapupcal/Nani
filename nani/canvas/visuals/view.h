#pragma once
#include "visuals_defs.h"
namespace nani::canvas::visuals
{
	class NANI_CANVAS_API View : public events::EventTarget
	{
	public:
		View(Window* window);
		virtual ~View();

	public:
		canvas::Window* Window() const;
		visuals::Visual* Visual();

		void BuildVisuals();
		void MarkDirty();
		void Resize(const basic::SizeF& size);
		void Flush();

	protected:
		void OnEvent(events::Event* e) override;

	private:
		// from visuals.
		void OnLayoutRequest(events::LayoutRequestEvent* e);
		void OnPaintRequest(events::PaintRequestEvent* e);

		// from window.
		void OnMouseMove(events::MouseMoveEvent* e);
		void OnMousePress(events::MousePressEvent* e);
		void OnMouseRelease(events::MouseButtonEvent* e);
		void OnKeyPress(events::KeyPressEvent* e);
		void OnKeyRelease(events::KeyReleaseEvent* e);

	private:
		friend class canvas::Window;
		canvas::Window* m_pWindow = nullptr;
		std::shared_ptr<visuals::Visual> m_spVisual;

		bool m_bLayoutDirty = true;
		bool m_bPaintDirty = true;
		basic::RectF m_dirtyRect;

		//TODO: use target_ptr to avoid dangling pointer.
		//TODO: implement target_ptr to support weak reference.
		//events::target_ptr<elements::Element> m_pHoverElement;
		//events::target_ptr<elements::Element> m_pFocusElement;
	};
}
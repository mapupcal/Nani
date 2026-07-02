#pragma once
#include "defs.h"
#include "events/event_target.h"
#include "basic/pointf.h"
#include "basic/sizef.h"
#include "basic/rectf.h"
#include "basic/color.h"

namespace nani::canvas
{
	class NANI_CANVAS_API Window : public events::EventTarget
	{
	public:
		enum Hint
		{
			None					= 0,
			Resizable				= 1,
			Tool					= 1 << 1,
			Top						= 1 << 2,
			TruncatedPassThrough	= 1 << 3,	// work with SetTruncatedColor, default is rgb(0, 0, 0)
		};
	public:
		explicit Window(const basic::PointF& pos, const basic::SizeF& size);
		~Window();

	public:
		const basic::PointF Position() const;
		const basic::SizeF Size() const;
		const basic::RectF Rect() const;
		const basic::RectF ClientRect() const;
		const basic::RectF Geometry() const;
		bool IsVisible() const;

		void Show();
		void Hide();
		void Close();
		void Move(const basic::PointF& pos);
		void Resize(const basic::SizeF& size);

		void SetRadius(basic::single fRadius);
		basic::single Radius() const;
		void SetBorderWidth(basic::single fWidth);
		basic::single BorderWidth() const;
		void SetBorderColor(const basic::Color& color);
		const basic::Color BorderColor() const;
		void SetBackgroundColor(const basic::Color& color);
		const basic::Color BackgroundColor() const;
		void SetTitle(const std::string_view& title);
		const std::string_view Title() const;

		void SetTruncatedColor(const basic::Color& color);
		void SetHints(Hint hints);
		Hint Hints() const;

		elements::Element* RootElement();
		visuals::View* GetView() const;
		SkCanvas* GetCanvas();

	private:
		void OnEvent(events::Event* e);

	private:
		internal::WindowPrivate* m_pImpl = nullptr;

		std::shared_ptr<Styles> m_spStyles = nullptr;
		elements::Element* m_pRootElement = nullptr;
		visuals::View* m_pView = nullptr;
	};

	inline Window::Hint operator|(Window::Hint lhs, Window::Hint rhs)
	{
		return static_cast<Window::Hint>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
	}

	inline Window::Hint operator&(Window::Hint lhs, Window::Hint rhs)
	{
		return static_cast<Window::Hint>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
	}
}

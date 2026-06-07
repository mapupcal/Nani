#pragma once
#include "defs.h"
#include "events/event_target.h"
#include "basic/pointf.h"
#include "basic/sizef.h"
#include "basic/rectf.h"
#include "basic/color.h"

namespace nani::canvas::internal
{
	class WindowPrivate;
}

namespace nani::canvas
{
	class NANI_API Window : public events::EventTarget
	{
	public:
		explicit Window(const basic::PointF& pos, const basic::SizeF& size);
		~Window();

	public:
		const basic::PointF Position() const;
		const basic::SizeF Size() const;
		const basic::RectF Rect() const;
		const basic::RectF Geometry() const;
		bool IsVisible() const;

		void Show();
		void Hide();
		void Close();
		void Move(const basic::PointF& pos);
		void Resize(const basic::SizeF& size);
		void Update();

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

	private:
		void OnEvent(events::Event* e);

	private:
		internal::WindowPrivate* m_pImpl = nullptr;
	};
}

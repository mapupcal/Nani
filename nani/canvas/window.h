#pragma once
#include "defs.h"
#include "event.h"
#include "basic/pointf.h"
#include "basic/sizef.h"
#include "basic/rectf.h"

namespace nani::canvas::internal
{
	class WindowPrivate;
}

namespace nani::canvas
{
	class NANI_API Window
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

		void RegisterEventFilter(Event::IFilter* filter);
		void UnRegisterEventFilter(Event::IFilter* filter);
	private:
		void RaiseEvent(Event* e);

	private:
		friend class internal::WindowPrivate;
		internal::WindowPrivate* m_pImpl = nullptr;
	};
}

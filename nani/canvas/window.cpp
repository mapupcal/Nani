#include "window.h"
#include "internal/window_p.h"
#include "elements/element.h"
#include "elements/styles.h"
#include "visuals/visual.h"
#include "visuals/view.h"

using namespace nani::canvas::basic;
using namespace nani::canvas::events;
using namespace nani::canvas::elements;
using namespace nani::canvas::visuals;

namespace nani::canvas
{
	Window::Window(const PointF& pos, const SizeF& size)
		: m_pImpl(new internal::WindowPrivate(this))
		, m_pView(new View(this))
	{
		m_pImpl->pos = pos;
		m_pImpl->size = size;
	}

	Window::~Window()
	{
		delete m_pView;
		delete m_pImpl;
		delete m_pRootElement;
	}

	const PointF Window::Position() const
	{
		return m_pImpl->pos;
	}

	const SizeF Window::Size() const
	{
		return m_pImpl->size;
	}

	const RectF Window::Rect() const
	{
		return RectF(0, 0, Size());
	}

	const basic::RectF Window::ClientRect() const
	{
		return Rect() - basic::MarginsF(BorderWidth(), BorderWidth(), BorderWidth(), BorderWidth());
	}

	const RectF Window::Geometry() const
	{
		return RectF(Position(), Size());
	}

	bool Window::IsVisible() const
	{
		return m_pImpl->IsVisible();
	}

	void Window::Show()
	{
		m_pImpl->Show();
	}

	void Window::Hide()
	{
		m_pImpl->Hide();
	}

	void Window::Close()
	{
		m_pImpl->Close();
	}

	void Window::Move(const basic::PointF& pos)
	{
		m_pImpl->Move(pos);
	}

	void Window::Resize(const basic::SizeF& size)
	{
		m_pImpl->Resize(size);
	}

	void Window::Update()
	{
		m_pView->Flush();
	}

	void Window::SetRadius(basic::single fRadius)
	{
		m_pImpl->SetRadius(fRadius);
	}

	basic::single Window::Radius() const
	{
		return m_pImpl->radius;
	}

	void Window::SetBorderWidth(basic::single fWidth)
	{
		m_pImpl->SetBorderWidth(fWidth);
	}

	basic::single Window::BorderWidth() const
	{
		return m_pImpl->borderWidth;
	}

	void Window::SetBorderColor(const basic::Color& color)
	{
		m_pImpl->SetBorderColor(color);
	}

	const basic::Color Window::BorderColor() const
	{
		return m_pImpl->borderColor;
	}

	void Window::SetBackgroundColor(const basic::Color& color)
	{
		m_pImpl->SetBackgroundColor(color);
	}

	const basic::Color Window::BackgroundColor() const
	{
		return m_pImpl->backgroundColor;
	}

	void Window::SetTitle(const std::string_view& title)
	{
		m_pImpl->SetTitle(title);
	}

	const std::string_view Window::Title() const
	{
		return m_pImpl->title;
	}

	void Window::SetTruncatedColor(const basic::Color& color)
	{
		m_pImpl->SetTruncatedColor(color);
	}

	void Window::SetHints(Hint hints)
	{
		m_pImpl->SetHints(hints);
	}

	Window::Hint Window::Hints() const
	{
		return (Window::Hint)m_pImpl->hints;
	}

	Element* Window::RootElement()
	{
		if (!m_pRootElement)
		{
			m_pRootElement = new Element(nullptr);
			m_spStyles = std::make_shared<Styles>();
			//TODO: default add Window Styles.
			m_pRootElement->SetStyles(m_spStyles);
			m_pRootElement->SetStyleClass(u8"Window");
		}
		return m_pRootElement;
	}

	visuals::View* Window::GetView() const
	{
		return m_pView;
	}

	SkCanvas* Window::GetCanvas()
	{
		return m_pImpl->GetCanvas();
	}

	void Window::OnEvent(Event* e)
	{
		switch (e->type)
		{
		case events::Type::Resize:
		{
			m_pView->MarkDirty();
			m_pView->Flush();
			break;
		}
		case events::Type::Show:
		{
			m_pView->BuildVisuals();
			break;
		}
		case events::Type::Close:
		{
			m_pView->MarkDirty();
			break;
		}
		case events::Type::MouseMove:
		{
			m_pView->OnMouseMove(static_cast<MouseMoveEvent*>(e));
			break;
		}
		case events::Type::MousePress:
		{
			m_pView->OnMousePress(static_cast<MousePressEvent*>(e));
			break;
		}
		case events::Type::MouseRelease:
		{
			m_pView->OnMouseRelease(static_cast<MouseReleaseEvent*>(e));
			break;
		}
		case events::Type::KeyPress:
		{
			m_pView->OnKeyPress(static_cast<KeyPressEvent*>(e));
			break;
		}
		case events::Type::KeyRelease:
		{
			m_pView->OnKeyRelease(static_cast<KeyReleaseEvent*>(e));
			break;
		}
		}
	}
}

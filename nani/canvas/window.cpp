#include "window.h"
#include "internal/window_p.h"
using namespace nani::canvas::basic;

namespace nani::canvas
{
	Window::Window(const PointF& pos, const SizeF& size)
		: m_pImpl(new internal::WindowPrivate(this))
	{
		m_pImpl->pos = pos;
		m_pImpl->size = size;
	}

	Window::~Window()
	{
		delete m_pImpl;
	}

	const PointF Window::Position() const
	{
		return m_pImpl->pos;
	}

	const SizeF Window::Size() const
	{
		return m_pImpl->size;
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

	}

	void Window::RaiseEvent(Event* e)
	{
		//TODO:
	}
}

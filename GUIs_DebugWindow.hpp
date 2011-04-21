#ifndef	DEBUG_WINDOW_HPP
#define	DEBUG_WINDOW_HPP

#include "D3D9.hpp"
#ifdef	OBGE_DEVLING

class DebugWindow
{
public:
	DebugWindow();
	~DebugWindow();

	static DebugWindow *Create();
	static DebugWindow *Expunge();

	void	PumpEvents(void);
	HWND ControlActiveWindow(HWND org);

private:
	LRESULT					WindowProc(UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK	_WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

	void	OnButtonHit(void);

	HWND			m_window;
	HWND			m_parent;
	HWND			m_master;
	volatile bool	m_done;

	HWND	m_button;
	HWND	m_editText, m_editText2;
};

#endif
#endif

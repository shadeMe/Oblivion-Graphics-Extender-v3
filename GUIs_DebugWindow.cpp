#include "GUIs_DebugWindow.hpp"
#include "obse/Utilities.h"
#include "GlobalSettings.h"
#include <assert.h>

static global<bool> DWEnabled(false, NULL, "General", "bEnabledDW");
static global<bool> FullScreen(0, "Oblivion.ini", "Display", "bFull Screen");

DebugWindow::DebugWindow()
:m_window(NULL), m_parent(NULL), m_master(NULL), m_button(NULL), m_editText(NULL), m_editText2(NULL)
{
	m_master = GetActiveWindow();

	// register our window class
	WNDCLASS	windowClass =
	{
		0,						// style
		_WindowProc,					// window proc
		0, 0,						// no extra memory required
		GetModuleHandle(NULL),				// instance
		NULL, NULL,					// icon, cursor
		(HBRUSH)(COLOR_BACKGROUND + 1),			// background brush
		NULL,						// menu name
		"OBGEWindow"					// class name
	};

	ATOM	classAtom = RegisterClass(&windowClass);
	ASSERT(classAtom);

	CreateWindow(
		(LPCTSTR)classAtom,				// class
		"OBGE Frame-Walker",				// name
		WS_OVERLAPPEDWINDOW | WS_SIZEBOX | WS_VISIBLE,	// style
		-100, 0,					// x y
		300, 300,					// width height
		NULL/*GetActiveWindow()*/,			// parent
		NULL,						// menu
		GetModuleHandle(NULL),				// instance
		(LPVOID)this);
}

DebugWindow::~DebugWindow()
{
	if(m_window)
		DestroyWindow(m_window);
}

void DebugWindow::PumpEvents(void)
{
	MSG	msg;

	m_done = false;

	while(!m_done)
	{
		BOOL	result = GetMessage(&msg, m_window, 0, 0);
		if(result == -1)
		{
			_MESSAGE("message pump error");
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if(result == 0)
			break;
	}
}

LRESULT DebugWindow::WindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
//	_MESSAGE("windowproc: %08X %08X %08X", msg, wParam, lParam);

	switch(msg)
	{
		case WM_CREATE:
			m_button = CreateWindow(
				"BUTTON",
				"Push Button",	// receive bacon
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				0, 0,
				100, 30,
				m_window,
				NULL,
				GetModuleHandle(NULL),
				NULL);
			m_editText = CreateWindow(
				"EDIT",
				NULL,
				WS_CHILD | WS_VISIBLE | ES_LEFT,
				0, 30,
				100, 30,
				m_window,
				NULL,
				GetModuleHandle(NULL),
				NULL);
			m_editText2 = CreateWindow(
				"EDIT",
				NULL,
				WS_CHILD | WS_VISIBLE | ES_LEFT,
				110, 30,
				100, 30,
				m_window,
				NULL,
				GetModuleHandle(NULL),
				NULL);

			ASSERT(m_button && m_editText);
			break;

		case WM_COMMAND:
		{
			HWND	source = (HWND)lParam;
			if(source == m_button)
			{
				OnButtonHit();
			}
		}
		break;

		case WM_ACTIVATE:
		//	if (m_parent)
		//		SendMessage(m_parent, msg, wParam, lParam);
		//	if (m_master)
		//		SendMessage(m_master, msg, wParam, lParam);
			if (wParam == WA_ACTIVE) {
				SetCursor(LoadCursor(NULL, IDC_ARROW));
				ShowCursor(TRUE);
			}
			break;
		case WM_SETFOCUS:
		//	if (m_parent)
		//		SetFocus(m_parent);
		//	if (m_master)
		//		SetFocus(m_master);
			ShowCursor(TRUE);
		//	SetFocus(m_window);
			break;
		case WM_DESTROY:
			m_done = true;
			break;
	};

	return DefWindowProc(m_window, msg, wParam, lParam);
}

LRESULT DebugWindow::_WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DebugWindow	* _this = NULL;

	if(msg == WM_CREATE)
	{
		CREATESTRUCT	* info = (CREATESTRUCT *)lParam;
		_this = (DebugWindow *)info->lpCreateParams;

		SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)_this);

		_this->m_window = window;
		_this->m_parent = GetParent(window);
	}
	else
	{
		_this = (DebugWindow *)GetWindowLongPtr(window, GWLP_USERDATA);
	}

	LRESULT	result;

	if(_this)
		result = _this->WindowProc(msg, wParam, lParam);
	else
		result = DefWindowProc(window, msg, wParam, lParam);

	return result;
}

void DebugWindow::OnButtonHit(void)
{
}

HWND DebugWindow::ControlActiveWindow(HWND org) {
	if ((org == m_master) && (m_window != NULL))
		return m_window;

	return org;
}

DebugWindow *dw = NULL;
DebugWindow *DebugWindow::Create() {
	if (!FullScreen.data && !dw)
		dw = new DebugWindow();

	return dw;
}

DebugWindow *DebugWindow::Expunge() {
	if (DWEnabled.data) {
		return DebugWindow::Create();
	}

	return NULL;
}

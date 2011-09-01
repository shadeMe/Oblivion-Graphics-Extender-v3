/* Version: MPL 1.1/LGPL 3.0
 *
 * "The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the Oblivion Graphics Extender, short OBGE.
 *
 * The Initial Developer of the Original Code is
 * Ethatron <niels@paradice-insight.us>. Portions created by The Initial
 * Developer are Copyright (C) 2011 The Initial Developer.
 * All Rights Reserved.
 *
 * Contributor(s):
 *  Timeslip (Version 1)
 *  scanti (Version 2)
 *  IlmrynAkios (Version 3)
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Library General Public License Version 3 license (the
 * "LGPL License"), in which case the provisions of LGPL License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the LGPL License and not
 * to allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL License.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the MPL or the LGPL License."
 */

#ifndef	OBGE_NOSHADER
#ifndef	DEBUG_WINDOW_HPP
#define	DEBUG_WINDOW_HPP

#include "D3D9.hpp"
#ifdef	OBGE_DEVLING
#define	OBGE_wx

class DebugWindow
{
public:
	DebugWindow();
	~DebugWindow();

	static DebugWindow *Create();
	static DebugWindow *Expunge();
	static DebugWindow *Get();
	static void Destroy();
	static void Exit();

#ifndef	OBGE_wx
	void	PumpEvents(void);
	HWND ControlActiveWindow(HWND org);

private:
	LRESULT				      WindowProc(UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK	_WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

	void	OnButtonHit(void);

	HWND			m_window;
	HWND			m_parent;
	HWND			m_master;
	volatile bool	m_done;

	HWND	m_button;
	HWND	m_editText, m_editText2;
#else
	void SetProgress(int a, int amax, int b, int bmax);
	void *sdev;
#endif
};

#endif
#endif
#endif

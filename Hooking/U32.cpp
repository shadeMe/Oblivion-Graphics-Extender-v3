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

#ifndef	HOOKING_U32_CPP
#define	HOOKING_U32_CPP

#include "U32.hpp"

// Hook structure.
enum
{
	U32FN_GetActiveWindow = 0
};

SDLLHook U32Hook =
{
	"user32.dll", NULL,
	false, NULL,		// Default hook disabled, NULL function pointer.
	{
		{ "GetActiveWindow", OBGEGetActiveWindow},
		{ NULL, NULL }
	}
};

extern class DebugWindow *dw;

// Hook function.
HWND WINAPI OBGEGetActiveWindow(void)
{
	GetActiveWindow_t old_func = (GetActiveWindow_t) U32Hook.Functions[U32FN_GetActiveWindow].OrigFn;
	HWND hWnd = old_func();

#if 0
	// hook active window
	if (!dw)
		DebugWindow::Expunge();
	if (dw)
		hWnd = dw->ControlActiveWindow(hWnd);
#endif

	return hWnd;
}

#endif

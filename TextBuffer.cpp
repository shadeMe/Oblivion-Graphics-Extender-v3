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

#include "TextBuffer.h"

TextBuffer::TextBuffer(int size)
{
	Buffer = new char[size];
	BufferMaxSize=size;
	BufferUsed=0;
	Overrun=false;
}

TextBuffer::~TextBuffer()
{
	delete [] Buffer;
}

void TextBuffer::ResetBuffer()
{
	Buffer[0]=0;
	BufferUsed=0;
	Overrun=false;
	return;
}

void TextBuffer::ResizeBuffer(int size)
{
	delete [] Buffer;
	Buffer = new char [size];
	BufferMaxSize=size;
	BufferUsed=0;
	return;
}

bool TextBuffer::AddText(char *text)
{

	if(!Overrun)
	{
		int len = strlen(text);
		if ((BufferUsed+len)<BufferMaxSize)
		{
			strcpy_s(&Buffer[BufferUsed],BufferMaxSize-BufferUsed,text);
			BufferUsed=BufferUsed+len;
			return false;
		}
		else
			Overrun=true;
	}
	return true;
}

bool TextBuffer::AddFormatedText(char *Format, ...)
{
	va_list		ArgList;

	if (!Overrun)
	{
		va_start(ArgList,Format);
		int len=_vscprintf(Format,ArgList);
		if((BufferUsed+len)<BufferMaxSize)
		{
			vsprintf_s(&Buffer[BufferUsed],BufferMaxSize-BufferUsed,Format,ArgList);
			BufferUsed=BufferUsed+len;
			va_end(ArgList);
			return false;
		}
		va_end(ArgList);
		Overrun=true;
	}
	return true;
}

bool TextBuffer::AddChar(char text)
{
	if(!Overrun)
	{
		Buffer[BufferUsed]=text;
		BufferUsed++;
		if (BufferUsed>=BufferMaxSize)
		{
			Buffer[BufferMaxSize-1]=0;
			Overrun=true;
			return true;
		}
		return false;
	}
	return(true);
}

bool TextBuffer::NewLine()
{
	return AddText("\n");
}



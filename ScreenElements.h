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

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <vector>

class ScreenElement
{
public:
	bool		enabled;
	D3DXVECTOR3	pos;
	D3DXVECTOR2	scale;
	float		rot;
	D3DXMATRIX	transform;
	DWORD		color;

	void			SetPosition(float x, float y, float z);
	void			SetScale(float x, float y);
	void			SetRotation(float rotation);
	void			UpdateTransform(void);
	void			SetColor(float red, float green, float blue);
	void			SetAlpha(float alpha);
	virtual void		Render(ID3DXSprite *sprite)=0;
};

class Sprite : public ScreenElement
{
public:
	int tex;

	bool			SetTexture(int texture);
	int			GetTexture(void);
	virtual void		Render(ID3DXSprite *sprite);
};

/*
class AnimatedSprite : public Sprite
{
public:

	float	fps;
	float	time;
	int		CurrentFrame;
	int		Width;
	int		Height;

	void	SetFramesPerSecond(float speed);
	void	SetCurrentFrame(int index);
	void	SetFilmStripDimensions(int width,int height);
	void	Render(ID3DXSprite *sprite);
};
*/

class HUDManager
{
public:
	static	HUDManager*			Singleton;
	std::vector<Sprite*>			ScreenElementList;
	int					NextElementIndex;
	LPD3DXSPRITE				sprite;

private:
	HUDManager();									// Declare as private. Use GetSingleton to initialise HUDManager.
													// This is so we can't init multiple HUD managers.
public:
	static HUDManager	*GetSingleton(void);
	int				AddScreenElement(Sprite *data);
	Sprite				*index(int ind);
	void				PurgeTexture(IDirect3DBaseTexture9 *texture, int TexNum = -1);
	void				Render(void);
	void				DeviceLost(void);
	void				DeviceReset(void);
	void				NewGame(void);
	void				LoadGame(void);
	void				SaveGame(void);
};
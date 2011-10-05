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

#include <assert.h>

#include "obse/Utilities.h"
#include "GlobalSettings.h"

#include "D3D9.hpp"
#include "D3D9Device.hpp"

#include "GUIs_DebugWindow.hpp"

#ifdef	OBGE_DEVLING

#include "EffectManager.h"
#include "ShaderManager.h"
#include "TextureConversions.h"
#include "TextureManager.h"
#include "TextureIOHook.hpp"
#include "Half.hpp"

static global<bool> DWEnabled(false, NULL, "General", "bEnabledDW");
static global<bool> FullScreen(0, "Oblivion.ini", "Display", "bFull Screen");

#define	OBGE_wx
#ifndef	OBGE_wx
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

#else
/* ---------------------------------------- */

#include <msvc/wx/setup.h>
#include <wx/init.h>
#include <wx/app.h>
#include <wx/dialog.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/string.h>
#include <wx/gauge.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#pragma comment(lib,"Comctl32")
#pragma comment(lib,"Rpcrt4")

#if defined(OGBE_PROFILE) || 1
#include <wx/xy/xyplot.h>
#include <wx/xy/xysimpledataset.h>
#include <wx/xy/xydynamicdataset.h>
#include <wx/xy/xylinerenderer.h>
#include <wx/xy/xyarearenderer.h>
#include <wx/chart.h>
#include <wx/chartpanel.h>

#ifndef	NDEBUG
#pragma comment(lib,"wxcode_msw28d_freechart")
#else
#pragma comment(lib,"wxcode_msw28_freechart")
#endif
#endif

#include "EffectManager.h"

#define RETURN_RGB(r,g,b) (((int)(r * 0xFF) & 0xFF) | ((int)(g * 0xFFFF) & 0xFF00) | ((int)(b * 0xFFFFFF) & 0xFF0000))
static int POStoRGB(int pos, int max) {
  // RGB are each returned on [0, 1].
  float h = (6.0 * pos) / max, s = 1.0, v = 1.0, m, n, f;
  int i;

  i = floor(h);
  f = h - i;
  if ( !(i&1) ) f = 1 - f; // if i is even
  m = v * (1 - s);
  n = v * (1 - s * f);
  switch (i) {
		  case 6:
		  case 0: return RETURN_RGB(v, n, m);
		  case 1: return RETURN_RGB(n, v, m);
		  case 2: return RETURN_RGB(m, v, n);
		  case 3: return RETURN_RGB(m, n, v);
		  case 4: return RETURN_RGB(n, m, v);
		  case 5: return RETURN_RGB(v, m, n);
  }
  return 0;
}

#include "GUIs_ShaderDeveloper.h"
#include "GUIs_ShaderDeveloper.cpp"

#define SDVIEW_SHADER	0
#define SDVIEW_EFFECT	1
#define SDVIEW_SCENES	2
#define SDVIEW_STATS	3

#define SDSURF_PRIMARY	0
#define SDSURF_GRABBED	1
#define SDSURF_DEPTH	2

#define wxBName		wxString((*BShader)->Name)
#define wxMName		wxString((*MEffect)->Name)
#define wxBMError	wxBitmap(wxT("#107"), wxBITMAP_TYPE_RESOURCE)
#define wxBMWarning	wxBitmap(wxT("#108"), wxBITMAP_TYPE_RESOURCE)
#define wxBMMissing	wxBitmap(wxT("#110"), wxBITMAP_TYPE_RESOURCE)
#define wxBMUnhooked	wxBitmap(wxT("#109"), wxBITMAP_TYPE_RESOURCE)
#define wxBMApplied	wxBitmap(wxT("#106"), wxBITMAP_TYPE_RESOURCE)

///////////////////////////////////////////////////////////////////////////////
/// Class wxShaderDeveloper
///////////////////////////////////////////////////////////////////////////////
class GUIs_ShaderDeveloper : public wxShaderDeveloper
{
public:
  GUIs_ShaderDeveloper(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 630,704 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL) :
      wxShaderDeveloper(parent, id, title, pos, size, style) {
    DefaultStyle.SetTextColour(wxColour("BLACK"));
    CommentStyle.SetTextColour(wxColour("SEA GREEN"));
    KeywordStyle.SetTextColour(wxColour("BLUE"));
    FunctionStyle.SetTextColour(wxColour("RED"));
    DeclaresStyle.SetTextColour(wxColour("ORANGE"));

    /* start enabling these functions */
    SDButtonEffectNew->Enable();

    SDToolsSettings->FindChildItem(wxID_MIPGAMMA, NULL)->Check(true);
    SDToolsSettings->FindChildItem(wxID_AMPLIFY, NULL)->Check(true);

    SDTweaks->FindChildItem(wxID_LINEAR, NULL)->Enable(true);
    SDTweaks->FindChildItem(wxID_LINEAR, NULL)->Check(DeGamma && ReGamma);

    if (1 > lastOBGEDirect3D9CAPS.MaxAnisotropy)
      SDTweaksAF->FindChildItem(wxID_AF1, NULL)->Enable(false);
    else if (1 == Anisotropy)
      SDTweaksAF->FindChildItem(wxID_AF1, NULL)->Check(true);
    if (2 > lastOBGEDirect3D9CAPS.MaxAnisotropy)
      SDTweaksAF->FindChildItem(wxID_AF2, NULL)->Enable(false);
    else if (2 == Anisotropy)
      SDTweaksAF->FindChildItem(wxID_AF2, NULL)->Check(true);
    if (4 > lastOBGEDirect3D9CAPS.MaxAnisotropy)
      SDTweaksAF->FindChildItem(wxID_AF4, NULL)->Enable(false);
    else if (4 == Anisotropy)
      SDTweaksAF->FindChildItem(wxID_AF4, NULL)->Check(true);
    if (8 > lastOBGEDirect3D9CAPS.MaxAnisotropy)
      SDTweaksAF->FindChildItem(wxID_AF8, NULL)->Enable(false);
    else if (8 == Anisotropy)
      SDTweaksAF->FindChildItem(wxID_AF8, NULL)->Check(true);
    if (16 > lastOBGEDirect3D9CAPS.MaxAnisotropy)
      SDTweaksAF->FindChildItem(wxID_AF16, NULL)->Enable(false);
    else if (16 == Anisotropy)
      SDTweaksAF->FindChildItem(wxID_AF16, NULL)->Check(true);
    if (32 > lastOBGEDirect3D9CAPS.MaxAnisotropy)
      SDTweaksAF->FindChildItem(wxID_AF32, NULL)->Enable(false);
    else if (32 == Anisotropy)
      SDTweaksAF->FindChildItem(wxID_AF32, NULL)->Check(true);

#if 0
    SDStatusGauge = new wxGauge(SDStatusBar, wxID_ANY, 100);
    SDStatusGauge->SetValue(50);
    SDStatusGauge->SetSize(400, 8);

    SDStatusText = new wxStaticText(SDStatusBar, wxID_ANY, wxT("..."), wxDefaultPosition, wxDefaultSize, 0);

    SDStatusSizerV = new wxBoxSizer( wxVERTICAL );
    SDStatusSizerH = new wxBoxSizer( wxHORIZONTAL );

    SDStatusSizerH->Add(SDStatusText, 1, wxALL | wxALIGN_CENTER_VERTICAL, 0);
    SDStatusSizerH->Add(SDStatusGauge, 1, wxALL | wxALIGN_CENTER_VERTICAL, 0);
    SDStatusSizerV->Add(SDStatusSizerH, 0, wxEXPAND | wxLEFT | wxRIGHT, 5);

    SDStatusBar->SetSizer(SDStatusSizerV);
    SDStatusBar->Layout();
    SDStatusSizerV->Fit(SDStatusBar);
#endif

    SDStatusBar->SetStatusText(wxT("Ready"), 0);

#if defined(OGBE_PROFILE) || 1
    memset(hist, 0, sizeof(hist));
//  assert(NULL);

    // first step: create plot
    plot = new XYPlot();

    // set line renderer to dataset & prevent deletion
//  rndr = new XYLineStepRenderer();
//  rndr = new XYLineRenderer();
    rndrP = new XYAreaRenderer(); rndrP->AddRef();
    rndrS = new XYAreaRenderer(); rndrS->AddRef();

    // create dataset
    vset = new XYDynamicDataset();
    vset->AddRef();

    // set line renderer to dataset
    vset->SetRenderer(rndrP);

    // add our dataset to plot
    plot->AddDataset(vset);

    // create dataset
    hist[OBGEPASS_ANY].dset = new XYDynamicDataset();
    hist[OBGEPASS_ANY].dset->AddRef();

    // set line renderer to dataset
    hist[OBGEPASS_ANY].dset->SetRenderer(rndrP);

    for (int p = OBGEPASS_MAX; p >= OBGEPASS_MIN; p--) {
      // create dataset
      hist[p].dset = new XYDynamicDataset();
      hist[p].dset->AddRef();

      // set line renderer to dataset
      hist[p].dset->SetRenderer(rndrS);
    }

    aset = NULL;

    // create left and bottom number axes
    leftAxis = new NumberAxis(AXIS_LEFT);
    bottomAxis = new NumberAxis(AXIS_BOTTOM);

    // optional: set axis titles
    leftAxis->SetTitle(wxT("ms"));
    bottomAxis->SetTitle(wxT("Frame"));

    // add axes to plot
    plot->AddAxis(leftAxis);
    plot->AddAxis(bottomAxis);

    // link axes and dataset
    plot->LinkDataVerticalAxis(0, 0);
    plot->LinkDataHorizontalAxis(0, 0);

    // set legend to plot
    plot->SetLegend(new Legend(wxTOP, wxLEFT));

    // and finally create chart
    chrt = new Chart(plot, wxString("Performance Graph"));

    // And finally create chart panel to display it:
    st = new wxChartPanel(SDStatsView, wxID_ANY, chrt, wxPoint(0,0), wxSize(256, 256));
    st->SetAntialias(true);

    ChartSizer = new wxBoxSizer( wxVERTICAL );

    ChartSizer->Add(st, 1, wxEXPAND | wxALL, 0);
    SDStatsView->SetSizer(ChartSizer);
//    SDStatsView->Layout();
    ChartSizer->Fit(SDStatsView);
//    ChartSizer->Layout();
    SDPanelStatsTop->Layout();

    normed = false;
#endif

    ProgressSubject = "...";
  }

//wxGauge *SDStatusGauge;
//wxStaticText *SDStatusText;
//wxBoxSizer *SDStatusSizerV;
//wxBoxSizer *SDStatusSizerH;

  ShaderManager *sm;
  EffectManager *em;
  wxImage rt, gt, ds;
  wxMemoryDC crt, grt, cds, sts;
  wxString ProgressSubject;
#if defined(OGBE_PROFILE) || 1
  NumberAxis *leftAxis, *bottomAxis;
  XYAreaRenderer *rndrP;
  XYAreaRenderer *rndrS;
  XYPlot *plot;
  Chart *chrt;
  wxChartPanel *st;
  wxBoxSizer *ChartSizer;
  XYDynamicDataset *vset, *aset;

  bool normed;
  struct history {
    int hconsm, maxcol;
    XYDynamicDataset *dset;
    XYDynamicSerie *series[OBGESCENE_NUM + 1];
  } hist[OBGEPASS_NUM];
#endif
  ShaderRecord *currs;
  EffectRecord *currx;
  int currv, currh;
  int fs[4];
  int pass;
  int scene;

  /* --------------------------------------------------------------
  */
  std::vector<wxString> keywordsHLSL;
  std::vector<wxString> keywordsAsm;
  std::vector<char> symbols;

  wxTextAttr DefaultStyle;
  wxTextAttr CommentStyle;
  wxTextAttr KeywordStyle;
  wxTextAttr FunctionStyle;
  wxTextAttr DeclaresStyle;
  wxTextAttr PreprocessorStyle;
  wxTextAttr SymbolStyle;

  /* --------------------------------------------------------------
   */
  IDirect3DSurface9 *FindGrabbedRT(int offs = 0) {
    if (pass == OBGEPASS_EFFECTS) {
      if (em && em->CurrDS.Srf[0])
	return em->CurrDS.Srf[0];
    }

    /* search shader render-targets */
    RuntimeShaderList::iterator RS = sm->RuntimeShaders.begin();
    while (RS != sm->RuntimeShaders.end()) {
      if (pass == OBGEPASS_ANY) {
	for (int p = OBGEPASS_MIN; p < OBGEPASS_NUM; p++)
	  if ((*RS)->rsb[p].pTextRT && (*RS)->rsb[p].pGrabRT)
	    if (--offs < 0)
	      return (*RS)->rsb[p].pGrabRT;
      }
      else {
	if ((*RS)->rsb[pass].pTextRT && (*RS)->rsb[pass].pGrabRT)
	  return (*RS)->rsb[pass].pGrabRT;
      }

      RS++;
    }

    return NULL;
  }

  IDirect3DSurface9 *FindGrabbedDS(int offs = 0) {
    if (pass == OBGEPASS_EFFECTS) {
      if (em && em->CurrDS.Srf[0])
	return em->CurrDS.Srf[0];
    }

    /* search shader render-targets */
    RuntimeShaderList::iterator RS = sm->RuntimeShaders.begin();
    while (RS != sm->RuntimeShaders.end()) {
      if (pass == OBGEPASS_ANY) {
	for (int p = OBGEPASS_MIN; p < OBGEPASS_NUM; p++)
	  if ((*RS)->rsb[p].pTextDS && (*RS)->rsb[p].pGrabDS)
	    if (--offs < 0)
	      return (*RS)->rsb[p].pGrabDS;
      }
      else {
	if ((*RS)->rsb[pass].pTextDS && (*RS)->rsb[pass].pGrabDS)
	  return (*RS)->rsb[pass].pGrabDS;
      }

      RS++;
    }

    return NULL;
  }

  /* --------------------------------------------------------------
   */
  const char *DescribeTexture(IDirect3DBaseTexture9 *tex) {
    static char buf[256];
    buf[0] = '\0';

    if (tex) {
      /* search alternate render targets */
      if (em) {
	if (em->OrigRT.IsTexture(tex))
	  return "Incoming effects-rendertarget";
	if (em->TrgtRT.IsTexture(tex))
	  return "Outgoing effects-rendertarget";
	if (em->CopyRT.IsTexture(tex))
	  return "Alternating[0] effects-rendertarget";
	if (em->LastRT.IsTexture(tex))
	  return "Alternating[1] effects-rendertarget";
	if (em->PrevRT.IsTexture(tex))
	  return "Last pass effects-rendertarget copy";
	if (em->PastRT.IsTexture(tex))
	  return "Last frame effects-rendertarget copy";
	if (em->OrigDS.IsTexture(tex))
	  return "Incoming effects-zbuffer";
	if (em->CurrDS.IsTexture(tex))
	  return "Converted effects-zbuffer";
      }

      /* search render targets */
      if (sm) {
	for (int p = OBGEPASS_MIN; p < OBGEPASS_NUM; p++)
	  for (int s = 0; s < sm->trackd[p].frame_cntr; s++) {
	    IDirect3DSurface9 *pRenderTarget;
	    textureSurface *pTextureSurface;

	    if ((pRenderTarget = sm->trackd[p].rt[s]) && ((int)pRenderTarget != -1)) {
	      if ((pTextureSurface = surfaceTexture[pRenderTarget])) {
		if (pTextureSurface->tex == tex) {
		  sprintf(buf, "RT of pass %d, scene %d", p, s);
		  return buf;
		}
	      }
	    }
	  }

	/* search shader render-targets */
	RuntimeShaderList::iterator RS = sm->RuntimeShaders.begin();
	while (RS != sm->RuntimeShaders.end()) {
	  for (int p = OBGEPASS_MIN; p < OBGEPASS_NUM; p++) {
	    if ((*RS)->rsb[p].pTextRT == tex) {
	      if ((*RS)->pAssociate)
		sprintf(buf, "%s pass %d rendertarget copy", (*RS)->pAssociate->Name, p);
	      else
		sprintf(buf, "Unknown shader pass %d rendertarget copy", p);
	      return buf;
	    }

	    if ((*RS)->rsb[p].pTextDS == tex) {
	      if ((*RS)->pAssociate)
		sprintf(buf, "%s pass %d depth-stencil copy", (*RS)->pAssociate->Name, p);
	      else
		sprintf(buf, "Unknown shader pass %d depth-stencil copy", p);
	      return buf;
	    }

	    if ((*RS)->rsb[p].pTextDZ == tex) {
	      if ((*RS)->pAssociate)
		sprintf(buf, "%s pass %d depth-stencil", (*RS)->pAssociate->Name, p);
	      else
		sprintf(buf, "Unknown shader pass %d depth-stencil", p);
	      return buf;
	    }
	  }

	  RS++;
	}
      }

      /* search other render-targets */
      std::map <void *, struct textureMap *>::iterator TRT = textureMaps.begin();
      while (TRT != textureMaps.end()) {
	if (TRT->second && (TRT->second->Usage & D3DUSAGE_RENDERTARGET))
	  if (TRT->first == tex) {
	    sprintf(buf, "Unidentified rendertarget");
	    return buf;
	  }

	TRT++;
      }

      /* search loaded texture files */
      std::map<std::string, IDirect3DBaseTexture9 *>::iterator TFile = textureFiles.begin();
      while (TFile != textureFiles.end()) {
      	if ((*TFile).second == tex) {
	  std::string str = (*TFile).first;
	  const char *ptr = str.data(), *ofs;

	  if ((ofs = strstr(ptr, "Textures\\")) ||
	      (ofs = strstr(ptr, "textures\\")) ||
	      (ofs = strstr(ptr, "textures/"))) {
	    sprintf(buf, "%s", ofs + 9);
	  }
	  else
	    sprintf(buf, "%s", ptr);

	  return buf;
      	}

      	TFile++;
      }

      /* search managed texture files */
      TextureManager *em = TextureManager::GetSingleton();
      int TexNum = em->FindTexture(tex); ManagedTextureRecord *Tex;
      if ((TexNum != -1) && (Tex = em->GetTexture(TexNum))) {
	const char *ptr = Tex->GetPath(), *ofs;

	if ((ofs = strstr(ptr, "Textures\\")) ||
	    (ofs = strstr(ptr, "textures\\")) ||
	    (ofs = strstr(ptr, "textures/"))) {
	  sprintf(buf, "%s", ofs + 9);
	}
	else
	  sprintf(buf, "%s", ptr);

	return buf;
      }

      sprintf(buf, "0x%p", tex);
    }
    else
      strcpy(buf, "cleared");

    return buf;
  }

  /* --------------------------------------------------------------
   */

  void SetShaderConstantTable(wxGrid *gt, bool prefix, LPD3DXCONSTANTTABLE c, int len, int col, int offs) {
    D3DXCONSTANTTABLE_DESC desc;
    D3DXCONSTANT_DESC cnst;
    UINT count = 1;
    char buf[256];

    if (gt && c) {
      if (c->GetDesc(&desc) == D3D_OK) {
	if (prefix) {
	  gt->SetCellValue(offs + 0, col, wxString(desc.Creator));
	  gt->SetCellTextColour(offs + 0, col, wxColour("BLACK"));

	  sprintf(buf, "%d.%d", (desc.Version >> 8) & 0xFF, desc.Version & 0xFF);
	  gt->SetCellValue(offs + 1, col, wxString(buf));
	  gt->SetCellTextColour(offs + 1, col, wxColour("BLACK"));

	  sprintf(buf, "%d", len);
	  gt->SetCellValue(offs + 2, col, wxString(buf));
	  gt->SetCellTextColour(offs + 2, col, wxColour("BLACK"));

	  sprintf(buf, "%d", desc.Constants);
	  gt->SetCellValue(offs + 3, col, wxString(buf));
	  gt->SetCellTextColour(offs + 3, col, wxColour("BLACK"));
	}

	for (int f = D3DXRS_BOOL; f <= D3DXRS_SAMPLER; f++) {
	  int pos = offs + (prefix ? 4 : 0);
	  for (int s = D3DXRS_BOOL; s < f; s++)
	    pos += fs[s];

	  for (int r = 0; r < desc.Constants; r++) {
	    D3DXHANDLE handle = c->GetConstant(NULL, r);
	    c->GetConstantDesc(handle, &cnst, &count);
	    D3DXREGISTER_SET rs = cnst.RegisterSet;

//	    switch (cnst.Type) {
//	      case D3DXPT_BOOL: rs = D3DXRS_BOOL; break;
//	      case D3DXPT_INT: rs = D3DXRS_INT4; break;
//	      case D3DXPT_FLOAT: rs = D3DXRS_FLOAT4; break;
//	      case D3DXPT_SAMPLER:
//	      case D3DXPT_SAMPLER1D:
//	      case D3DXPT_SAMPLER2D:
//	      case D3DXPT_SAMPLER3D:
//	      case D3DXPT_SAMPLERCUBE: rs = D3DXRS_SAMPLER; break;
//	    }

	    if (rs != f)
	      continue;

	    char c = ' ';
	    wxColour clr;
	    switch (f) {
	      case D3DXRS_BOOL: c = 'b'; clr = wxColour("GREY"); break;
	      case D3DXRS_INT4: c = 'i'; clr = wxColour("SIENNA"); break;
	      case D3DXRS_FLOAT4: c = 'c'; clr = wxColour("BLUE"); break;
	      case D3DXRS_SAMPLER: c = 's'; clr = wxColour("DARK GREEN"); break;
	    }

	    /* TODO: fix up vertex-shader sampler positions */
	    int range = cnst.RegisterIndex + cnst.RegisterCount;
	    int delta = fs[f];
	    if (range > fs[f]) {
	      gt->InsertRows(pos + fs[f], range - fs[f]);
	      fs[f] = range;
	    }

	    for (int d = delta; d < fs[f]; d++) {
	      sprintf(buf, "%c%d", c, d);
	      gt->SetRowLabelValue(pos + d, wxString(buf));
	    }

	    for (int x = cnst.RegisterIndex, i = 0; x < range; x++, i++) {
	      if (cnst.Name[0]) {
		if (cnst.RegisterCount > 1)
		  sprintf(buf, "%s[%d]", cnst.Name, i);
		else
		  sprintf(buf, "%s", cnst.Name);
	      }
	      else {
		if (cnst.RegisterCount > 1)
		  sprintf(buf, "c%d[%d]", x, i);
		else
		  sprintf(buf, "c%d", x);
	      }

	      gt->SetCellTextColour(pos + x, col, clr);
	      gt->SetCellValue(pos + x, col, wxString(buf));
	    }
	  }
	}
      }
    }
  }

  void SetShaderConstSetTable(wxGrid *gt, bool prefix, struct RuntimeShaderRecord::trace *t, int len, int col, int offs) {
    char buf[256];

    if (gt && t) {
      {
	int f = D3DXRS_BOOL; {
	  int pos = offs + (prefix ? 4 : 0);
	  for (int s = D3DXRS_BOOL; s < f; s++)
	    pos += fs[s];

	  wxColour clr = wxColour("GREY");
	  for (int x = 0; x < fs[f]; x++) {
	    assert(x < 256);
	    if (*((char *)&t->values_b[x]) != -1) {
	      sprintf(buf, "%d", t->values_b[x]);

	      gt->SetCellTextColour(pos + x, col, clr);
	      gt->SetCellValue(pos + x, col, wxString(buf));

	      /* non-editable cell */
	      wxString Description = gt->GetCellValue(pos + x, col - 1);
	      if ((Description.GetData() == strstr(Description.GetData(), "glob_"))) {
		gt->SetReadOnly(pos + x, col, false);
		gt->SetCellBackgroundColour(pos + x, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ));
	      }
	      else if ((Description.GetData() == strstr(Description.GetData(), "cust_"))) {
		gt->SetReadOnly(pos + x, col, false);
		gt->SetCellBackgroundColour(pos + x, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ));
	      }
	      else {
		gt->SetReadOnly(pos + x, col, true);
		gt->SetCellBackgroundColour(pos + x, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ));
	      }
	    }
	  }
	}

	f = D3DXRS_INT4; {
	  int pos = offs + (prefix ? 4 : 0);
	  for (int s = D3DXRS_BOOL; s < f; s++)
	    pos += fs[s];

	  wxColour clr = wxColour("SIENNA");
	  for (int x = 0; x < fs[f]; x++) {
	    assert(x < 256);
	    if (*((int *)&t->values_i[x][0]) != -1) {
	      sprintf(buf, "%d, %d, %d, %d", t->values_i[x][0], t->values_i[x][1], t->values_i[x][2], t->values_i[x][3]);

	      gt->SetCellTextColour(pos + x, col, clr);
	      gt->SetCellValue(pos + x, col, wxString(buf));

	      /* non-editable cell */
	      wxString Description = gt->GetCellValue(pos + x, col - 1);
	      if ((Description.GetData() == strstr(Description.GetData(), "glob_"))) {
		gt->SetReadOnly(pos + x, col, false);
		gt->SetCellBackgroundColour(pos + x, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ));
	      }
	      else if ((Description.GetData() == strstr(Description.GetData(), "cust_"))) {
		gt->SetReadOnly(pos + x, col, false);
		gt->SetCellBackgroundColour(pos + x, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ));
	      }
	      else {
		gt->SetReadOnly(pos + x, col, true);
		gt->SetCellBackgroundColour(pos + x, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ));
	      }
	    }
	  }
	}

	f = D3DXRS_FLOAT4; {
	  int pos = offs + (prefix ? 4 : 0);
	  for (int s = D3DXRS_BOOL; s < f; s++)
	    pos += fs[s];

	  wxColour clr = wxColour("BLUE");
	  for (int x = 0; x < fs[f]; x++) {
	    assert(x < 256);
	    if (*((int *)&t->values_c[x][0]) != -1) {
	      sprintf(buf, "%f, %f, %f, %f", t->values_c[x][0], t->values_c[x][1], t->values_c[x][2], t->values_c[x][3]);

	      gt->SetCellValue(pos + x, col, wxString(buf));

	      /* non-editable cell */
	      wxString Description = gt->GetCellValue(pos + x, col - 1);
	      if ((Description.GetData() == strstr(Description.GetData(), "glob_"))) {
		gt->SetReadOnly(pos + x, col, false);
		gt->SetCellBackgroundColour(pos + x, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ));
	      }
	      else if ((Description.GetData() == strstr(Description.GetData(), "cust_"))) {
		gt->SetReadOnly(pos + x, col, false);
		gt->SetCellBackgroundColour(pos + x, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ));
	      }
	      else {
		gt->SetReadOnly(pos + x, col, true);
		gt->SetCellBackgroundColour(pos + x, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ));
	      }

	      wxString meaning0 = gt->GetCellValue(pos + x, col - 2);
	      wxString meaning1 = gt->GetCellValue(pos + x, col - 1);
	      if (strstr(meaning0.GetData(), "color") ||
		  strstr(meaning0.GetData(), "Color") ||
		  strstr(meaning1.GetData(), "color") ||
		  strstr(meaning1.GetData(), "Color")) {
		  unsigned long b = (unsigned long)(t->values_c[x][0] * 255);
		  unsigned long g = (unsigned long)(t->values_c[x][1] * 255);
		  unsigned long r = (unsigned long)(t->values_c[x][2] * 255);

		  gt->SetCellBackgroundColour(pos + x, col,
		    wxColour((r << 16) | (g << 8) | (b << 0)));
		  gt->SetCellTextColour(pos + x, col,
		    wxColour(((255 - r) << 16) | ((255 - g) << 8) | ((255 - b) << 0)));
	      }
	      else
		gt->SetCellTextColour(pos + x, col, clr);
	    }
	  }
	}

	f = D3DXRS_SAMPLER; {
	  int pos = offs + (prefix ? 4 : 0);
	  for (int s = D3DXRS_BOOL; s < f; s++)
	    pos += fs[s];

	  wxColour clr = wxColour("DARK GREEN");
	  for (int x = 0; x < fs[f]; x++) {
	    assert(x < OBGESAMPLER_NUM);
	    if (*((int *)&t->values_s[x]) != -1) {
	      const char *dt = DescribeTexture(t->values_s[x]);

	      gt->SetCellTextColour(pos + x, col, clr);
	      gt->SetCellValue(pos + x, col, wxString(dt));

	      /* non-editable cell */
	      wxString Description = gt->GetCellValue(pos + x, col - 1);
	      if ((Description.GetData() == strstr(Description.GetData(), "glob_"))) {
		gt->SetReadOnly(pos + x, col, false);
		gt->SetCellBackgroundColour(pos + x, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ));
	      }
	      else if ((Description.GetData() == strstr(Description.GetData(), "cust_"))) {
		gt->SetReadOnly(pos + x, col, false);
		gt->SetCellBackgroundColour(pos + x, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ));
	      }
	      else {
		gt->SetReadOnly(pos + x, col, true);
		gt->SetCellBackgroundColour(pos + x, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ));
	      }
	    }
	  }
	}
      }
    }
  }

  void SetShaderSamplerTable(wxGrid *gt, bool prefix, struct RuntimeShaderRecord::trace *t, int len, int col, int offs) {
    char buf[256];

    if (gt && t) {
      int pos = offs + 0, d = 0;
      for (int x = 0; x < OBGESAMPLER_NUM; x++) {
	/* no -1 (never set), and no 0 (cleared) */
	if (*((int *)&t->values_s[x]) != -1) {
	  {
	    gt->InsertRows(pos + d, 1);

	    sprintf(buf, "s%d", x);
	    gt->SetRowLabelValue(pos + d, wxString(buf));

	    wxColour clr = wxColour("DARK GREEN");
	    gt->SetCellTextColour(pos + d, col + 0, clr);
	    gt->SetCellValue(pos + d, col + 0, wxString("Texture"));
	    gt->SetReadOnly(pos + d, col + 0, true);

	    const char *dt = DescribeTexture(t->values_s[x]);
	    gt->SetCellValue(pos + d, col + 1, wxString(dt));
	    gt->SetReadOnly(pos + d, col + 1, true);

	    d++;
	  }

	  for (int y = 0; y < 14; y++) {
	    if (*((int *)&t->states_s[x][y]) != -1) {
	      gt->InsertRows(pos + d, 1);
	      sprintf(buf, "s%d", x);
	      gt->SetRowLabelValue(pos + d, wxString(buf));

	      wxColour clr = wxColour("SIENNA");
	      gt->SetCellTextColour(pos + d, col + 0, clr);

	      sprintf(buf, "%s", findSamplerState((D3DSAMPLERSTATETYPE)y));
	      gt->SetCellValue(pos + d, col + 0, wxString(buf));
	      gt->SetReadOnly(pos + d, col + 0, true);

	      sprintf(buf, "%s", findSamplerStateValue((D3DSAMPLERSTATETYPE)y, t->states_s[x][y]));
	      gt->SetCellValue(pos + d, col + 1, wxString(buf));
	      gt->SetReadOnly(pos + d, col + 1, true);

	      d++;
	    }
	  }
	}
      }
    }
  }

  void SetEffectConstSetTable(wxGrid *gt, bool prefix, ID3DXEffect *x, int len, int col, int offs) {
    char buf[1024];

    if (gt && x) {
      int pos = offs + 0, d = 0;
      D3DXEFFECT_DESC Description;
      x->GetDesc(&Description);

      for (int par = 0; par < Description.Parameters; par++) {
	D3DXHANDLE handle;

	if ((handle = x->GetParameter(NULL, par))) {
	  D3DXPARAMETER_DESC Description;
	  x->GetParameterDesc(handle, &Description);

	  switch (Description.Type) {
	    case D3DXPT_INT: {
	      IntType IntData;

	      IntData.size = Description.Elements;
	      if (IntData.size == 0)
		IntData.size = 1;
	      IntData.size *= Description.Columns;

	      x->GetIntArray(handle, (int *)&IntData.data, IntData.size * Description.Rows);

	      for (int rw = 0; rw < Description.Rows; rw++) {
		gt->InsertRows(pos + d, 1);

		buf[0] = buf[1023] = '\0';
		if (Description.Rows > 1)
		  _snprintf(buf, 1023, "%s[%d]", Description.Name, rw);
		else
		  _snprintf(buf, 1023, "%s", Description.Name);

		gt->SetRowLabelValue(pos + d, wxString(buf));

		buf[0] = buf[1023] = '\0';
		for (int i = rw * IntData.size; i < (rw + 1) * IntData.size; i++)
		  _snprintf(buf, 1023, "%s%d%s", buf, IntData.data[i], i != ((rw + 1) * IntData.size - 1) ? ", " : "");

		wxColour clr = wxColour("BLUE");
		gt->SetCellTextColour(pos + d, col + 0, clr);
		gt->SetCellValue(pos + d, col + 0, wxString(buf));

		/* non-editable cell */
		if ((Description.Name != strstr(Description.Name, "obge_")) &&
		    (Description.Name != strstr(Description.Name, "oblv_"))) {
		  gt->SetReadOnly(pos + d, col, false);
		  gt->SetCellBackgroundColour(pos + d, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ));
		}
		else {
		  gt->SetReadOnly(pos + d, col, true);
		  gt->SetCellBackgroundColour(pos + d, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ));
		}

		d++;
	      }
	    } break;
	    case D3DXPT_FLOAT: {
	      FloatType FloatData;

	      FloatData.size = Description.Elements;
	      if (FloatData.size == 0)
		FloatData.size = 1;
	      FloatData.size *= Description.Columns;

	      x->GetFloatArray(handle, (float *)&FloatData.data, FloatData.size * Description.Rows);

	      for (int rw = 0; rw < Description.Rows; rw++) {
		gt->InsertRows(pos + d, 1);

		buf[0] = buf[1023] = '\0';
		if (Description.Rows > 1)
		  _snprintf(buf, 1023, "%s[%d]", Description.Name, rw);
		else
		  _snprintf(buf, 1023, "%s", Description.Name);

		gt->SetRowLabelValue(pos + d, wxString(buf));

		buf[0] = buf[1023] = '\0';
		for (int i = rw * FloatData.size; i < (rw + 1) * FloatData.size; i++)
		  _snprintf(buf, 1023, "%s%f%s", buf, FloatData.data[i], i != ((rw + 1) * FloatData.size - 1) ? ", " : "");

		wxColour clr = wxColour("BLUE");
		gt->SetCellTextColour(pos + d, col + 0, clr);
		gt->SetCellValue(pos + d, col + 0, wxString(buf));

		/* non-editable cell */
		if ((Description.Name != strstr(Description.Name, "obge_")) &&
		    (Description.Name != strstr(Description.Name, "oblv_"))) {
		  gt->SetReadOnly(pos + d, col, false);
		  gt->SetCellBackgroundColour(pos + d, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ));
		}
		else {
		  gt->SetReadOnly(pos + d, col, true);
		  gt->SetCellBackgroundColour(pos + d, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ));
		}

		if (strstr(Description.Name, "color") ||
		    strstr(Description.Name, "Color")) {
		    unsigned long b = (unsigned long)(FloatData.data[0] * 255);
		    unsigned long g = (unsigned long)(FloatData.data[1] * 255);
		    unsigned long r = (unsigned long)(FloatData.data[2] * 255);

		    gt->SetCellBackgroundColour(pos + d, col,
		      wxColour((r << 16) | (g << 8) | (b << 0)));
		    gt->SetCellTextColour(pos + d, col,
		      wxColour(((255 - r) << 16) | ((255 - g) << 8) | ((255 - b) << 0)));
		}
		else
		  gt->SetCellTextColour(pos + d, col, clr);

		d++;
	      }
	    } break;
	  }
	}
      }
    }
  }

  void SetEffectTextureTable(wxGrid *gt, bool prefix, ID3DXEffect *x, int len, int col, int offs) {
//  char buf[256];

    if (gt && x) {
      int pos = offs + 0, d = 0;
      TextureManager *TexMan = TextureManager::GetSingleton();
      D3DXEFFECT_DESC Description;
      x->GetDesc(&Description);

      for (int par = 0; par < Description.Parameters; par++) {
	D3DXHANDLE handle;

	if ((handle = x->GetParameter(NULL, par))) {
	  D3DXPARAMETER_DESC Description;
	  x->GetParameterDesc(handle, &Description);

	  switch (Description.Type) {
	    case D3DXPT_TEXTURE:
	    case D3DXPT_TEXTURE1D:
	    case D3DXPT_TEXTURE2D:
	    case D3DXPT_TEXTURE3D:
	    case D3DXPT_TEXTURECUBE: {
	      IDirect3DBaseTexture9 *Texture = NULL;
	      x->GetTexture(handle, &Texture);

	      {
		gt->InsertRows(pos + d, 1);

		gt->SetRowLabelValue(pos + d, wxString(Description.Name));

		wxColour clr = wxColour("DARK GREEN");
		gt->SetCellTextColour(pos + d, col + 0, clr);
		gt->SetCellValue(pos + d, col + 0, wxString(DescribeTexture(Texture)));

		/* non-editable cell */
		if ((Description.Name != strstr(Description.Name, "obge_")) &&
		    (Description.Name != strstr(Description.Name, "oblv_"))) {
		  gt->SetReadOnly(pos + d, col, false);
		  gt->SetCellBackgroundColour(pos + d, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ));
		}
		else {
		  gt->SetReadOnly(pos + d, col, true);
		  gt->SetCellBackgroundColour(pos + d, col + 0, wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ));
		}

		d++;
	      }
	    } break;
	  }
	}
      }
    }
  }

  void SetRenderTable(wxGrid *gt, bool prefix, int o, struct ShaderManager::track *t, int len, int col, int offs) {
    char buf[256];

    if (gt && t) {
      int pos = offs + 0, d = 0;

      for (int v = 0; v < 3; v++) {
        if (*((int *)&t->transf[o][v].m[0][0]) != -1) {
          for (int x = 0; x < 4; x++) {
            gt->InsertRows(pos + d, 1);

            sprintf(buf, "%d", d);
            gt->SetRowLabelValue(pos + d, wxString(buf));

            wxColour clr = wxColour("BLUE");

	    sprintf(buf, "%s[%d]", v == 0 ? "View" : (v == 1 ? "Projection" : "World"), x);
            gt->SetCellTextColour(pos + d, col + 0, clr);
            gt->SetCellValue(pos + d, col + 0, wxString(buf));

	    sprintf(buf, "%f, %f, %f, %f", t->transf[o][v].m[x][0], t->transf[o][v].m[x][1], t->transf[o][v].m[x][2], t->transf[o][v].m[x][3]);
            gt->SetCellTextColour(pos + d, col + 1, clr);
            gt->SetCellValue(pos + d, col + 1, wxString(buf));

            d++;
          }
        }
      }

      for (int y = 0; y < 210; y++) {
        if (*((int *)&t->states[o][y]) != -1) {
          gt->InsertRows(pos + d, 1);

          sprintf(buf, "%d", d);
          gt->SetRowLabelValue(pos + d, wxString(buf));

          wxColour clr = wxColour("SIENNA");
          gt->SetCellTextColour(pos + d, col + 0, clr);

          sprintf(buf, "%s", findRenderState((D3DRENDERSTATETYPE)y));
          gt->SetCellValue(pos + d, col + 0, wxString(buf));

	  sprintf(buf, "%s", findRenderStateValue((D3DRENDERSTATETYPE)y, t->states[o][y]));
          gt->SetCellValue(pos + d, col + 1, wxString(buf));

          d++;
        }
      }
    }
  }

  /* --------------------------------------------------------------
   */

  void SetShaderConstantTable(ShaderRecord *o) {
    wxGrid *gt = SDShaderConstantGrid;

    memset(fs, 0, sizeof(fs));
    gt->ClearGrid();

    if (3 > gt->GetNumberCols())
      gt->AppendCols(3 - gt->GetNumberCols());
    if (gt->GetNumberCols() > 3)
      gt->DeleteCols(0, gt->GetNumberCols() - 3);

    gt->SetColLabelValue(0, wxT("Original"));
    gt->SetColLabelValue(1, wxT("Replaced"));
    gt->SetColLabelValue(2, wxT("Runtime"));

    if (o) {
      if (4 > gt->GetNumberRows())
	gt->AppendRows(4 - gt->GetNumberRows());
      if (gt->GetNumberRows() > 4)
	gt->DeleteRows(0, gt->GetNumberRows() - 4);

      gt->SetRowLabelValue(0, wxT("Creator"));
      gt->SetRowLabelValue(1, wxT("Version"));
      gt->SetRowLabelValue(2, wxT("Binary Size"));
      gt->SetRowLabelValue(3, wxT("Constants"));

      SetShaderConstantTable(gt, true, o->pConstsOriginal, o->pShaderOriginal ? o->pShaderOriginal->GetBufferSize() : -1, 0, 0);
      SetShaderConstantTable(gt, true, o->pConstsReplaced, o->pShaderReplaced ? o->pShaderReplaced->GetBufferSize() : -1, 1, 0);
      SetShaderConstantTable(gt, true, o->pConstsRuntime , o->pShaderRuntime  ? o->pShaderRuntime->GetBufferSize()  : -1, 2, 0);
    }

    gt->SetRowLabelSize(wxGRID_AUTOSIZE);
    wxSize sz = gt->GetClientSize();
    int dv = sz.GetWidth() - gt->GetRowLabelSize();
    gt->SetColSize(0, dv / 3);
    gt->SetColSize(1, dv / 3);
    gt->SetColSize(2, dv / 3);
  }

  void SetShaderConstSetTable(ShaderRecord *o) {
    wxGrid *gt = SDShaderConstSetGrid;

    memset(fs, 0, sizeof(fs));
    gt->ClearGrid();

    if (3 > gt->GetNumberCols())
      gt->AppendCols(3 - gt->GetNumberCols());
    if (gt->GetNumberCols() > 3)
      gt->DeleteCols(0, gt->GetNumberCols() - 3);

    gt->SetColLabelValue(0, wxT("Original"));
    gt->SetColLabelValue(1, wxT("Replaced"));
    gt->SetColLabelValue(2, wxT("Runtime"));

    if (o && o->pAssociate) {
      if (0 > gt->GetNumberRows())
	gt->AppendRows(0 - gt->GetNumberRows());
      if (gt->GetNumberRows() > 0)
	gt->DeleteRows(0, gt->GetNumberRows() - 0);

      SetShaderConstantTable(gt, false, o->pConstsOriginal, o->pShaderOriginal ? o->pShaderOriginal->GetBufferSize() : -1, 0, 0);

      switch (o->pDX9ShaderType) {
	default:
	case SHADER_REPLACED: SetShaderConstantTable(gt, false, o->pConstsReplaced, o->pShaderReplaced ? o->pShaderReplaced->GetBufferSize() : -1, 1, 0); break;
	case SHADER_ORIGINAL: SetShaderConstantTable(gt, false, o->pConstsOriginal, o->pShaderOriginal ? o->pShaderOriginal->GetBufferSize() : -1, 1, 0); break;
	case SHADER_RUNTIME : SetShaderConstantTable(gt, false, o->pConstsRuntime , o->pShaderRuntime  ? o->pShaderRuntime->GetBufferSize()  : -1, 1, 0); break;
      }

      RuntimeShaderRecord *r = o->pAssociate;
      if (r->frame_used[pass] >= 0)
	SetShaderConstSetTable(gt, false, &r->traced[pass], -1, 2, 0);
    }

    gt->SetRowLabelSize(wxGRID_AUTOSIZE);
    wxSize sz = gt->GetClientSize();
    int dv = sz.GetWidth() - gt->GetRowLabelSize();
    gt->SetColSize(0, dv / 4);
    gt->SetColSize(1, dv / 4);
    gt->SetColSize(2, dv / 2);
  }

  void SetShaderSamplerTable(ShaderRecord *o) {
    wxGrid *gt = SDShaderSamplerGrid;

    memset(fs, 0, sizeof(fs));
    gt->ClearGrid();

    if (2 > gt->GetNumberCols())
      gt->AppendCols(2 - gt->GetNumberCols());
    if (gt->GetNumberCols() > 2)
      gt->DeleteCols(0, gt->GetNumberCols() - 2);

    gt->SetColLabelValue(0, wxT("State"));
    gt->SetColLabelValue(1, wxT("Value"));

    if (o && o->pAssociate) {
      if (0 > gt->GetNumberRows())
	gt->AppendRows(0 - gt->GetNumberRows());
      if (gt->GetNumberRows() > 0)
	gt->DeleteRows(0, gt->GetNumberRows() - 0);

      RuntimeShaderRecord *r = o->pAssociate;
      if (r->frame_used[pass] >= 0)
	SetShaderSamplerTable(gt, false, &r->traced[pass], -1, 0, 0);
    }

    gt->SetRowLabelSize(wxGRID_AUTOSIZE);
    wxSize sz = gt->GetClientSize();
    int dv = sz.GetWidth() - gt->GetRowLabelSize();
    gt->SetColSize(0, dv / 2);
    gt->SetColSize(1, dv / 2);
  }

  /* --------------------------------------------------------------
   */

  void SetEffectConstantTable(EffectRecord *o) {
  }

  void SetEffectConstSetTable(EffectRecord *o) {
    wxGrid *gt = SDEffectConstSetGrid;

    memset(fs, 0, sizeof(fs));
    gt->ClearGrid();

    if (1 > gt->GetNumberCols())
      gt->AppendCols(1 - gt->GetNumberCols());
    if (gt->GetNumberCols() > 1)
      gt->DeleteCols(0, gt->GetNumberCols() - 1);

    gt->SetColLabelValue(0, wxT("Value"));

    if (o && o->pEffect) {
      if (0 > gt->GetNumberRows())
	gt->AppendRows(0 - gt->GetNumberRows());
      if (gt->GetNumberRows() > 0)
	gt->DeleteRows(0, gt->GetNumberRows() - 0);

      if (o->IsEnabled())
	SetEffectConstSetTable(gt, false, o->GetEffect(), -1, 0, 0);
    }

    gt->SetRowLabelSize(wxGRID_AUTOSIZE);
    wxSize sz = gt->GetClientSize();
    int dv = sz.GetWidth() - gt->GetRowLabelSize();
    gt->SetColSize(0, dv);
  }

  void SetEffectTextureTable(EffectRecord *o) {
    wxGrid *gt = SDEffectTexturesGrid;

    memset(fs, 0, sizeof(fs));
    gt->ClearGrid();

    if (1 > gt->GetNumberCols())
      gt->AppendCols(1 - gt->GetNumberCols());
    if (gt->GetNumberCols() > 1)
      gt->DeleteCols(0, gt->GetNumberCols() - 1);

    gt->SetColLabelValue(0, wxT("Value"));

    if (o && o->pEffect) {
      if (0 > gt->GetNumberRows())
	gt->AppendRows(0 - gt->GetNumberRows());
      if (gt->GetNumberRows() > 0)
	gt->DeleteRows(0, gt->GetNumberRows() - 0);

      if (o->IsEnabled())
	SetEffectTextureTable(gt, false, o->GetEffect(), -1, 0, 0);
    }

    gt->SetRowLabelSize(wxGRID_AUTOSIZE);
    wxSize sz = gt->GetClientSize();
    int dv = sz.GetWidth() - gt->GetRowLabelSize();
    gt->SetColSize(0, dv);
  }

  /* --------------------------------------------------------------
   */

  void SetStatesTable(int o) {
    int p = pass; FindScene(p, o);
    wxGrid *gt = SDSceneStateGrid;

    gt->ClearGrid();

    if (2 > gt->GetNumberCols())
      gt->AppendCols(2 - gt->GetNumberCols());
    if (gt->GetNumberCols() > 2)
      gt->DeleteCols(0, gt->GetNumberCols() - 2);

    gt->SetColLabelValue(0, wxT("State"));
    gt->SetColLabelValue(1, wxT("Value"));

    if (sm) {
      if (0 > gt->GetNumberRows())
	gt->AppendRows(0 - gt->GetNumberRows());
      if (gt->GetNumberRows() > 0)
	gt->DeleteRows(0, gt->GetNumberRows() - 0);

      if (sm->trackd[p].frame_cntr > 0)
        SetRenderTable(gt, false, o, &sm->trackd[p], -1, 0, 0);
    }

    gt->SetRowLabelSize(wxGRID_AUTOSIZE);
    wxSize sz = gt->GetClientSize();
    int dv = sz.GetWidth() - gt->GetRowLabelSize();
    gt->SetColSize(0, dv / 2);
    gt->SetColSize(1, dv / 2);
  }

  /* --------------------------------------------------------------
   */

  void SetStatsTable(int o) {
  }

  /* --------------------------------------------------------------
   */

  void SetImage(int p, wxImage *im, IDirect3DSurface9 *pSurface) {
    bool valid = false;

    if (im && pSurface) {
      IDirect3DSurface9 *pBuf = NULL, *rBuf = NULL;
      D3DSURFACE_DESC VDesc;
      D3DLOCKED_RECT surf;

      // GetRenderTarget(0, &pRenderTarget);
      if (pSurface->GetDesc(&VDesc) == D3D_OK) {
	/* locking INTZ may succeed but it's only garbage in it. */
	if (0 && (pSurface->LockRect(&surf, NULL, D3DLOCK_NOSYSLOCK) == D3D_OK)) {
	  pSurface->UnlockRect();
	  pBuf = pSurface;

	  valid = true;
	}

	else if (VDesc.Usage & D3DUSAGE_DEPTHSTENCIL) {
#if 0
	  if (lastOBGEDirect3DDevice9->CreateOffscreenPlainSurface(VDesc.Width, VDesc.Height, VDesc.Format, D3DPOOL_SYSTEMMEM, &pBuf, NULL) == D3D_OK) {
	    if (D3DXLoadSurfaceFromSurface(pBuf, NULL, NULL, pSurface, NULL, NULL, D3DX_FILTER_NONE, 0) == D3D_OK) {
	      valid = true;
	    }
	  }

	  if (!valid && surfaceTexture[pSurface]) {
	    if (pBuf)
	      pBuf->Release();
	    pBuf = NULL;

	    {
	      EffectRecord D3D_Effect;

	      D3DXMACRO def[2] = { {"", "1"}, {NULL, NULL} };
	      switch (VDesc.Format) {
		    case D3DFMT_D16_LOCKABLE:  def[0].Name = "D16_LOCKABLE";  VDesc.Format = D3DFMT_L16; break;
		    case D3DFMT_D32:	       def[0].Name = "D32";           VDesc.Format = D3DFMT_R32F; /* loss */ break;
		    case D3DFMT_D15S1:	       def[0].Name = "D15S1";         VDesc.Format = D3DFMT_L16; break;
		    case D3DFMT_D24S8:	       def[0].Name = "D24S8";         VDesc.Format = D3DFMT_R32F; break;
		    case D3DFMT_D24X8:	       def[0].Name = "D24X8";         VDesc.Format = D3DFMT_R32F; break;
		    case D3DFMT_D24X4S4:       def[0].Name = "D24X4S4";       VDesc.Format = D3DFMT_R32F; break;
		    case D3DFMT_D16:	       def[0].Name = "D16";           VDesc.Format = D3DFMT_L16; break;

		    case D3DFMT_D32F_LOCKABLE: def[0].Name = "D32F_LOCKABLE"; VDesc.Format = D3DFMT_R32F; break;
		    case D3DFMT_D24FS8:	       def[0].Name = "D24FS8";        VDesc.Format = D3DFMT_R32F; break;
		    case D3DFMT_D32_LOCKABLE:  def[0].Name = "D32_LOCKABLE";  VDesc.Format = D3DFMT_R32F; /* loss */ break;

		    case (D3DFORMAT)MAKEFOURCC('I','N','T','Z'): def[0].Name = "INTZ"; VDesc.Format = D3DFMT_R32F; /* loss */ break;
		    case (D3DFORMAT)MAKEFOURCC('D','F','2','4'): def[0].Name = "DF24"; VDesc.Format = D3DFMT_R32F; break;
		    case (D3DFORMAT)MAKEFOURCC('D','F','1','6'): def[0].Name = "DF16"; VDesc.Format = D3DFMT_R16F; break;
		    case (D3DFORMAT)MAKEFOURCC('R','A','W','Z'): def[0].Name = "RAWZ"; break;
	      }

	      if (D3D_Effect.LoadEffect("TransferZ.fx", 0, true, (D3DXMACRO *)&def)) {
		ID3DXEffect *Effect = D3D_Effect.GetEffect();
		frame_trk = false;

		if (lastOBGEDirect3DDevice9->CreateRenderTarget(VDesc.Width, VDesc.Height, VDesc.Format, D3DMULTISAMPLE_NONE, 0, FALSE, &rBuf, NULL) == D3D_OK) {
		  lastOBGEDirect3DDevice9->SetRenderTarget(0, rBuf);
		  lastOBGEDirect3DDevice9->SetDepthStencilSurface(NULL);
		//lastOBGEDirect3DDevice9->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0);
		  lastOBGEDirect3DDevice9->BeginScene();

		  IDirect3DVertexBuffer9 *D3D_EffectBuffer;
		  float minx, minu, uadj, vadj;
		  float rcpres[2];

		  rcpres[0] = 1.0f / (float)VDesc.Width;
		  rcpres[1] = 1.0f / (float)VDesc.Height;

		  uadj = rcpres[0] * 0.5;
		  vadj = rcpres[1] * 0.5;

		  minx = -1;
		  minu =  0;

		  D3D_sShaderVertex ShaderVertices[] =
		  {
		    {minx, +1, 1, minu + uadj, 0 + vadj},
		    {minx, -1, 1, minu + uadj, 1 + vadj},
		    {1   , +1, 1, 1    + uadj, 0 + vadj},
		    {1   , -1, 1, 1    + uadj, 1 + vadj}
		  };

		  lastOBGEDirect3DDevice9->CreateVertexBuffer(4 * sizeof(D3D_sShaderVertex), D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_TEX1, D3DPOOL_DEFAULT, &D3D_EffectBuffer,0);
		  void *VertexPointer;

		  D3D_EffectBuffer->Lock(0, 0, &VertexPointer, 0);
		  CopyMemory(VertexPointer, ShaderVertices, sizeof(ShaderVertices));
		  D3D_EffectBuffer->Unlock();

		  lastOBGEDirect3DDevice9->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);

		  lastOBGEDirect3DDevice9->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);
		  lastOBGEDirect3DDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		  lastOBGEDirect3DDevice9->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		  lastOBGEDirect3DDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		  // Set up world/view/proj matrices to identity in case there's no vertex effect.
		  D3DXMATRIX mIdent;
		  D3DXMatrixIdentity(&mIdent);
		  // Sets up the viewport.
		  D3DVIEWPORT9 vp = { 0, 0, VDesc.Width, VDesc.Height, 0.0, 1.0 };

		  lastOBGEDirect3DDevice9->SetViewport(&vp);
		  lastOBGEDirect3DDevice9->SetTransform(D3DTS_PROJECTION, &mIdent);
		  lastOBGEDirect3DDevice9->SetTransform(D3DTS_VIEW, &mIdent);
		  lastOBGEDirect3DDevice9->SetTransform(D3DTS_WORLD, &mIdent);

		  Effect->SetTexture("zbufferTexture", surfaceTexture[pSurface]->tex);
	//	  Effect->SetTexture("zbufferTexture", surfaceTexture[sm->trackd[6].rt[0]]->tex);

		  UINT passes;
		  Effect->Begin(&passes, NULL);

		  for (UINT pass = 0; pass < passes; pass++) {
		    Effect->BeginPass(pass);
		    lastOBGEDirect3DDevice9->SetStreamSource(0, D3D_EffectBuffer, 0, sizeof(D3D_sShaderVertex));
		    lastOBGEDirect3DDevice9->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		    Effect->EndPass();
		  }

		  Effect->End();
		  lastOBGEDirect3DDevice9->EndScene();

		  if (lastOBGEDirect3DDevice9->CreateOffscreenPlainSurface(VDesc.Width, VDesc.Height, VDesc.Format, D3DPOOL_SYSTEMMEM, &pBuf, NULL) == D3D_OK) {
		    if (lastOBGEDirect3DDevice9->GetRenderTargetData(rBuf, pBuf) == D3D_OK) {
		      valid = true;
		    }
		  }

		  rBuf->Release();
		}

		frame_trk = true;
	      }
	    }
	  }
#endif
	}

	else if (VDesc.Usage & D3DUSAGE_RENDERTARGET) {
	  if (lastOBGEDirect3DDevice9->CreateOffscreenPlainSurface(VDesc.Width, VDesc.Height, VDesc.Format, D3DPOOL_SYSTEMMEM, &pBuf, NULL) == D3D_OK) {
	    if (lastOBGEDirect3DDevice9->GetRenderTargetData(pSurface, pBuf) == D3D_OK) {
	      valid = true;
	    }
	  }
	}

	if (valid) {
	  valid = false;

	  {
	    if (pBuf->LockRect(&surf, NULL, D3DLOCK_NOSYSLOCK) == D3D_OK) {
              unsigned char *rgb;
              unsigned char *a;

              if (!im->IsOk() || (im->GetWidth() != VDesc.Width) || (im->GetHeight() != VDesc.Height)) {
                if (im->IsOk())
                  im->Destroy();

                if (!im->Create(VDesc.Width, VDesc.Height, false))
                  assert(NULL);
              }

              int w = im->GetWidth();
              int h = im->GetHeight();
              rgb = im->GetData();
	      if (im->HasAlpha())
		a = im->GetAlpha();

	      switch (VDesc.Format) {
		case D3DFMT_R8G8B8: {
		  unsigned char *_rgb = (unsigned char *)surf.pBits;
		  memcpy(rgb, _rgb, w * h * 3);
		  valid = true;
		} break;
		case D3DFMT_A8R8G8B8: {
		  unsigned char *_argb = (unsigned char *)surf.pBits;
		  if (p == OBGEPASS_REFLECTION)
		    for (int hh = 0; hh < h; hh++) {
		      for (int ww = 0; ww < w; ww++) {
			int b = _argb[((hh * w) + ww) * 4 + 0];
			int g = _argb[((hh * w) + ww) * 4 + 1];
			int r = _argb[((hh * w) + ww) * 4 + 2];
			int a = _argb[((hh * w) + ww) * 4 + 3];
			rgb[((hh * w) + ww) * 3 + 0] = r;
			rgb[((hh * w) + ww) * 3 + 1] = g;
			rgb[((hh * w) + ww) * 3 + 2] = b;
		      }
		    }
		  else
		    for (int hh = 0; hh < h; hh++) {
		      for (int ww = 0; ww < w; ww++) {
			int b = _argb[((hh * w) + ww) * 4 + 0];
			int g = _argb[((hh * w) + ww) * 4 + 1];
			int r = _argb[((hh * w) + ww) * 4 + 2];
			int a = _argb[((hh * w) + ww) * 4 + 3];
			rgb[((hh * w) + ww) * 3 + 0] = r;
			rgb[((hh * w) + ww) * 3 + 1] = g;
			rgb[((hh * w) + ww) * 3 + 2] = b;
		      }
		    }
		  valid = true;
		} break;
		case D3DFMT_A16B16G16R16: {
		  unsigned short *_abgr = (unsigned short *)surf.pBits;
		  int R = 0, RR = 0xFFFF;
		  int G = 0, GG = 0xFFFF;
		  int B = 0, BB = 0xFFFF;
		  int A = 0, AA = 0xFFFF;
		  for (int hh = 0; hh < h; hh++) {
		    for (int ww = 0; ww < w; ww++) {
		      int r = _abgr[((hh * w) + ww) * 4 + 0];
		      int g = _abgr[((hh * w) + ww) * 4 + 1];
		      int b = _abgr[((hh * w) + ww) * 4 + 2];
		      int a = _abgr[((hh * w) + ww) * 4 + 3];
		      if (r > R) R = r; if (r < RR) RR = r;
		      if (g > G) G = g; if (g < GG) GG = g;
		      if (b > B) B = b; if (b < BB) BB = b;
		      if (a > A) A = a; if (a < AA) AA = a;
		    }
		  }
		  R -= RR; if (0 >= R) R = 1;
		  G -= GG; if (0 >= G) G = 1;
		  B -= BB; if (0 >= B) B = 1;
		  A -= AA; if (0 >= A) A = 1;
		  for (int hh = 0; hh < h; hh++) {
		    for (int ww = 0; ww < w; ww++) {
		      int r = _abgr[((hh * w) + ww) * 4 + 0] - RR;
		      int g = _abgr[((hh * w) + ww) * 4 + 1] - GG;
		      int b = _abgr[((hh * w) + ww) * 4 + 2] - BB;
		      int a = _abgr[((hh * w) + ww) * 4 + 3] - AA;
		      rgb[((hh * w) + ww) * 3 + 0] = (r * 255) / R;
		      rgb[((hh * w) + ww) * 3 + 1] = (g * 255) / G;
		      rgb[((hh * w) + ww) * 3 + 2] = (b * 255) / B;
		    }
		  }
		  valid = true;
		} break;
		case D3DFMT_A16B16G16R16F: {
		  half *_abgr = (half *)surf.pBits;
		  for (int hh = 0; hh < h; hh++) {
		    for (int ww = 0; ww < w; ww++) {
		      int r = (int)((float)_abgr[((hh * w) + ww) * 4 + 0] * 255);
		      int g = (int)((float)_abgr[((hh * w) + ww) * 4 + 1] * 255);
		      int b = (int)((float)_abgr[((hh * w) + ww) * 4 + 2] * 255);
		      int a = (int)((float)_abgr[((hh * w) + ww) * 4 + 3] * 255);
		      rgb[((hh * w) + ww) * 3 + 0] = (r >= 255 ? 255 : (r >= 0 ? r : 0));
		      rgb[((hh * w) + ww) * 3 + 1] = (g >= 255 ? 255 : (g >= 0 ? g : 0));
		      rgb[((hh * w) + ww) * 3 + 2] = (b >= 255 ? 255 : (b >= 0 ? b : 0));
		    }
		  }
		  valid = true;
		} break;
		case D3DFMT_A32B32G32R32F: {
		  float *_abgr = (float *)surf.pBits;
		  for (int hh = 0; hh < h; hh++) {
		    for (int ww = 0; ww < w; ww++) {
		      int r = (int)(((float)_abgr[((hh * w) + ww) * 4 + 0] + 0.5) * 255);
		      int g = (int)(((float)_abgr[((hh * w) + ww) * 4 + 1] + 0.5) * 255);
		      int b = (int)(((float)_abgr[((hh * w) + ww) * 4 + 2] + 0.5) * 255);
		      int a = (int)(((float)_abgr[((hh * w) + ww) * 4 + 3] + 0.5) * 255);
		      rgb[((hh * w) + ww) * 3 + 0] = (r >= 255 ? 255 : (r >= 0 ? r : 0));
		      rgb[((hh * w) + ww) * 3 + 1] = (g >= 255 ? 255 : (g >= 0 ? g : 0));
		      rgb[((hh * w) + ww) * 3 + 2] = (b >= 255 ? 255 : (b >= 0 ? b : 0));
		    }
		  }
		  valid = true;
		} break;
		case D3DFMT_R32F: {
		  float *_r = (float *)surf.pBits;
		  if (p == OBGEPASS_WATERHEIGHTMAP)
		    for (int hh = 0; hh < h; hh++) {
		      for (int ww = 0; ww < w; ww++) {
			int r = (int)(((float)_r[((hh * w) + ww) * 1 + 0] + 0.5) * 255);
			rgb[((hh * w) + ww) * 3 + 0] = (r >= 255 ? 255 : (r >= 0 ? r : 0));
			rgb[((hh * w) + ww) * 3 + 1] = (r >= 255 ? 255 : (r >= 0 ? r : 0));
			rgb[((hh * w) + ww) * 3 + 2] = (r >= 255 ? 255 : (r >= 0 ? r : 0));
		      }
		    }
		  else
		    for (int hh = 0; hh < h; hh++) {
		      for (int ww = 0; ww < w; ww++) {
			int r = (int)(((float)_r[((hh * w) + ww) * 1 + 0] + 0.0) * 255);
			rgb[((hh * w) + ww) * 3 + 0] = (r >= 255 ? 255 : (r >= 0 ? r : 0));
			rgb[((hh * w) + ww) * 3 + 1] = (r >= 255 ? 255 : (r >= 0 ? r : 0));
			rgb[((hh * w) + ww) * 3 + 2] = (r >= 255 ? 255 : (r >= 0 ? r : 0));
		      }
		    }
		  im->ConvertToGreyscale();
		  valid = true;
		} break;
#if 0
		    D3DFMT_D16_LOCKABLE         = 70,
		    D3DFMT_D32                  = 71,
		    D3DFMT_D15S1                = 73,
		    D3DFMT_D24S8                = 75,
		    D3DFMT_D24X8                = 77,
		    D3DFMT_D24X4S4              = 79,
		    D3DFMT_D16                  = 80,

		    D3DFMT_D32F_LOCKABLE        = 82,
		    D3DFMT_D24FS8               = 83,
		    D3DFMT_D32_LOCKABLE         = 84,
		case (D3DFORMAT)MAKEFOURCC('D','F','2','4'): def[0].Name = "DF24"; VDesc.Format = D3DFMT_R32F; break;
		case (D3DFORMAT)MAKEFOURCC('D','F','1','6'): def[0].Name = "DF16"; VDesc.Format = D3DFMT_R16F; break;
		case (D3DFORMAT)MAKEFOURCC('R','A','W','Z'): def[0].Name = "RAWZ"; break;

#endif
		case (D3DFORMAT)MAKEFOURCC('I','N','T','Z'): {
		  unsigned long *_z = (unsigned long *)surf.pBits;
		  unsigned long Z = 0, ZZ = 0xFFFFFFFF;
		  for (int hh = 0; hh < h; hh++) {
		    for (int ww = 0; ww < w; ww++) {
		      unsigned long z = _z[((hh * w) + ww) * 1 + 0];
		      if (z > Z) Z = z; if (z < ZZ) ZZ = z;
		    }
		  }
		  Z -= ZZ; if (0 >= Z) Z = 1;
		  for (int hh = 0; hh < h; hh++) {
		    for (int ww = 0; ww < w; ww++) {
		      unsigned long z = _z[((hh * w) + ww) * 1 + 0] - ZZ;
		      rgb[((hh * w) + ww) * 3 + 0] = (z * 255) / Z;
		      rgb[((hh * w) + ww) * 3 + 1] = (z * 255) / Z;
		      rgb[((hh * w) + ww) * 3 + 2] = (z * 255) / Z;
		    }
		  }
		  valid = true;
		} break;
                default:
		  assert(NULL);
                  break;
              }
            }

            pBuf->UnlockRect();
          }
        }
      }

      if (pBuf && (pBuf != pSurface))
        pBuf->Release();
    }

    if (!valid) {
      if (!im->IsOk())
        if (!im->Create(1, 1, false))
          assert(NULL);

      if (im->IsOk())
	memset(im->GetData(), 0, im->GetWidth() * im->GetHeight() * 3);
    }
  }

  /* --------------------------------------------------------------
   */

  void UpdateShaderOptions(ShaderRecord *o) {
    SDShaderVersion->Clear();

    if (o) {
      wxMenuItem *mi; mi = SDShaderOptions->FindChildItem(wxID_SUPGRADE, NULL);
      int idx = 0, sel = wxNOT_FOUND;

      if (o->iType == SHADER_PIXEL) {
	if (!mi || !mi->IsChecked()) {
	  SDShaderVersion->Append(wxString("ps_1_1")); if (o->pProfile && !strcmp(o->pProfile, "ps_1_1")) sel = idx; idx++;
	  SDShaderVersion->Append(wxString("ps_1_2")); if (o->pProfile && !strcmp(o->pProfile, "ps_1_2")) sel = idx; idx++;
	  SDShaderVersion->Append(wxString("ps_1_3")); if (o->pProfile && !strcmp(o->pProfile, "ps_1_3")) sel = idx; idx++;
	  SDShaderVersion->Append(wxString("ps_1_4")); if (o->pProfile && !strcmp(o->pProfile, "ps_1_4")) sel = idx; idx++;
	}

	SDShaderVersion->Append(wxString("ps_2_0"));  if (o->pProfile && !strcmp(o->pProfile, "ps_2_0" )) sel = idx; idx++;
	SDShaderVersion->Append(wxString("ps_2_a"));  if (o->pProfile && !strcmp(o->pProfile, "ps_2_a" )) sel = idx; idx++;
	SDShaderVersion->Append(wxString("ps_2_b"));  if (o->pProfile && !strcmp(o->pProfile, "ps_2_b" )) sel = idx; idx++;
	SDShaderVersion->Append(wxString("ps_2_sw")); if (o->pProfile && !strcmp(o->pProfile, "ps_2_sw")) sel = idx; idx++;
	SDShaderVersion->Append(wxString("ps_3_0"));  if (o->pProfile && !strcmp(o->pProfile, "ps_3_0" )) sel = idx; idx++;
	SDShaderVersion->Append(wxString("ps_3_sw")); if (o->pProfile && !strcmp(o->pProfile, "ps_3_sw")) sel = idx; idx++;
      }

      else if (o->iType == SHADER_VERTEX) {
	if (!mi || !mi->IsChecked()) {
	  SDShaderVersion->Append(wxString("vs_1_1")); if (o->pProfile && !strcmp(o->pProfile, "vs_1_1")) sel = idx; idx++;
	}

	SDShaderVersion->Append(wxString("vs_2_0"));  if (o->pProfile && !strcmp(o->pProfile, "vs_2_0" )) sel = idx; idx++;
	SDShaderVersion->Append(wxString("vs_2_a"));  if (o->pProfile && !strcmp(o->pProfile, "vs_2_a" )) sel = idx; idx++;
	SDShaderVersion->Append(wxString("vs_2_sw")); if (o->pProfile && !strcmp(o->pProfile, "vs_2_sw")) sel = idx; idx++;
	SDShaderVersion->Append(wxString("vs_3_0"));  if (o->pProfile && !strcmp(o->pProfile, "vs_3_0" )) sel = idx; idx++;
	SDShaderVersion->Append(wxString("vs_3_sw")); if (o->pProfile && !strcmp(o->pProfile, "vs_3_sw")) sel = idx; idx++;
      }

      if (sel != wxNOT_FOUND)
	SDShaderVersion->SetSelection(sel);
      else if (idx > 0)
	SDShaderVersion->SetSelection(0);
    }
  }

  void UpdateEffectOptions(EffectRecord *o) {
  }

  /* --------------------------------------------------------------
   */

  void GetShaderRecordStatus(ShaderRecord *o) {
    char *status = "";
    char buf[256];

    int x = SDComboShader->GetSelection();
    if (x == wxNOT_FOUND)
      if (!SDComboShader->IsEmpty())
	SDComboShader->SetSelection(x = 0);

    if (sm && sm->RuntimeSources()) {
      SDShaderEnable->Enable();

      if (o && (o->pDX9ShaderType == SHADER_RUNTIME))
        SDShaderEnable->Set3StateValue(wxCHK_CHECKED);
      else if (o && (o->pDX9ShaderType == SHADER_REPLACED))
        SDShaderEnable->Set3StateValue(wxCHK_UNDETERMINED);
      else if (o && (o->pDX9ShaderType == SHADER_ORIGINAL))
	SDShaderEnable->Set3StateValue(wxCHK_UNCHECKED);

      else if (o && o->pShaderRuntime)
	SDShaderEnable->Set3StateValue(wxCHK_CHECKED);
      else if (o && o->pShaderReplaced)
	SDShaderEnable->Set3StateValue(wxCHK_UNDETERMINED);
      else
	SDShaderEnable->Set3StateValue(wxCHK_UNCHECKED);
    }
    else {
      SDShaderEnable->Disable();

      if (o && o->pShaderRuntime)
        SDShaderEnable->Set3StateValue(wxCHK_CHECKED);
      else if (o && o->pShaderReplaced)
        SDShaderEnable->Set3StateValue(wxCHK_UNDETERMINED);
      else
        SDShaderEnable->Set3StateValue(wxCHK_UNCHECKED);
    }

    if (o && o->pErrorMsgs) {
      char *buf = (char *)o->pErrorMsgs->GetBufferPointer();

      if (strstr(buf, "error")) {
      	/* two errors possible:
      	 * - runtime failed, replacement good
      	 * - both failed
      	 */
      	if (o->pSourceRuntime && o->pShaderReplaced)
	  status = "Status: compiled as %s, with errors, disabled, override enabled";
	else
	  status = "Status: compiled as %s, with errors, disabled";

	SDShaderCompile->Enable();
	if (x != wxNOT_FOUND)
	  SDComboShader->SetItemBitmap(x, wxBMError);
      }
      else if (strstr(buf, "warning")) {
      	/* two warnings possible:
      	 * - runtime warning
      	 * - replacement warning
      	 */
      	if (o->pShaderRuntime)
	  status = "Status: compiled as %s, with warnings, runtime enabled";
	else
	  status = "Status: compiled as %s, with warnings, override enabled";

	SDShaderCompile->Enable();
	if (x != wxNOT_FOUND)
	  SDComboShader->SetItemBitmap(x, wxBMWarning);
      }
      else {
	status = "Status: compiled as %s, enabled";
	SDShaderCompile->Disable();
	if (x != wxNOT_FOUND)
	  SDComboShader->SetItemBitmap(x, wxBMApplied);
      }
    }
    else {
      if (o && (o->pShaderRuntime || o->pShaderReplaced)) {
      	/* two activations possible:
      	 * - runtime
      	 * - replacement
      	 */
      	if (o->pShaderRuntime)
	  status = "Status: compiled as %s, runtime enabled";
	else
	  status = "Status: compiled as %s, override enabled";

	SDShaderCompile->Disable();
	if (x != wxNOT_FOUND)
	  SDComboShader->SetItemBitmap(x, wxBMApplied);
      }
      else if (o && !o->pShaderOriginal) {
	status = "Status: missing";
	SDShaderCompile->Disable();
	if (x != wxNOT_FOUND)
	  SDComboShader->SetItemBitmap(x, wxBMMissing);
      }
      else {
	status = "Status: not compiled";
	SDShaderCompile->Enable();
	if (x != wxNOT_FOUND)
	  SDComboShader->SetItemBitmap(x, wxBMUnhooked);
      }
    }

    sprintf(buf, status, o ? o->pProfile : "-");
    SDStatusShader->SetLabel(buf);

    if (o && o->pAssociate) {
      if (!o->pAssociate->Paired.empty()) {
	std::set<RuntimeShaderRecord *>::iterator Pair = o->pAssociate->Paired.begin();

	buf[0] = '\0';
	char pair[256]; pair[0] = '\0';
	while (Pair != o->pAssociate->Paired.end()) {
	  if (pair[0] != '\0')
	    strcat(pair, ", ");
	  strcat(pair, (*Pair)->pAssociate->Name);
	  Pair++;
	}

	sprintf(buf, "Pair: %s", pair);
      }
      else
	sprintf(buf, "Pair: -");

      SDShaderPairing->SetLabel(buf);
    }

    if (o && o->pSourceRuntime  && !o->pShaderRuntime )
      SDShaderCompile->Enable();
    if (o && o->pSourceReplaced && !o->pShaderReplaced)
      SDShaderCompile->Enable();
    if (o && o->pAsmblyReplaced && !o->pShaderReplaced)
      SDShaderCompile->Enable();
  }

  void UpdateShaderRecord(ShaderRecord *o) {
    GetShaderRecordStatus(o);

    /* source has changed */
    wxString oo = SDShaderSourceEditor->GetValue();
    const wxChar *ooo = oo.GetData();
    int size = strlen(ooo);

    /* not a runtime-shader (like IDENTIFY, COPYQUAD etc. */
    if (!o->pAssociate) {
      SDShaderEnable->Disable();

      SDShaderMark->Disable();
      SDShaderMark->SetValue(FALSE);
    }
    else {
      SDShaderEnable->Enable();

      SDShaderMark->Enable();
      SDShaderMark->SetValue(o->pAssociate->bMark);
    }

    /* prevent flickering */
    if (o) {
      if (o->pSourceRuntime) {
	if (strcmp(oo.GetData(), o->pSourceRuntime))
	  SDButtonShaderSave->Enable();
	else
	  SDButtonShaderSave->Disable();
      }
      else if (o->pSourceReplaced) {
	if (strcmp(oo.GetData(), o->pSourceReplaced))
	  SDButtonShaderSave->Enable();
	else
	  SDButtonShaderSave->Disable();
      }
      else
	SDButtonShaderSave->Enable();
    }
    else
      SDButtonShaderSave->Disable();
  }

  void UpdateShaderText(wxTextCtrl *txt, const char *val) {
    /* source has changed */
    wxString oo = txt->GetValue();
    const wxChar *ooo = oo.GetData();
    int size = strlen(ooo);

    if (strcmp(oo.GetData(), val)) {
      txt->SetValue(wxString(val));
    }
  }

  void SetShaderRecord(ShaderRecord *o) {
    if (o && !o->pDisasmbly)
      o->DisassembleShader();

    GetShaderRecordStatus(o);
    UpdateShaderOptions(o);

    SDButtonShaderSave->Disable();

#if 0
    int epos = SDShaderSourceEditor->GetLastPosition();
    int apos = SDShaderAssemblyEditor->GetLastPosition();
    int dpos = SDShaderDisassemblyView->GetLastPosition();
#endif

    /* HLSL goes always (load/save/saveas) */
    if (o && o->pSourceRuntime)
      UpdateShaderText(SDShaderSourceEditor, o->pSourceRuntime);
    else if (o && o->pSourceReplaced)
      UpdateShaderText(SDShaderSourceEditor, o->pSourceReplaced);
    else
      UpdateShaderText(SDShaderSourceEditor, "");

    /* Assembler goes always (load/save/saveas) */
    if (o && o->pAsmblyReplaced)
      UpdateShaderText(SDShaderAssemblyEditor, o->pAsmblyReplaced);
    else
      UpdateShaderText(SDShaderAssemblyEditor, "");

    /* Errors are only deactivated */
    if (o && o->pErrorMsgs) {
      SDShaderErrorView->Enable();
      UpdateShaderText(SDShaderErrorView, (char *)o->pErrorMsgs->GetBufferPointer());
    }
    else {
      SDShaderErrorView->Disable();
      UpdateShaderText(SDShaderErrorView, "");
    }

    /* Disassembly is only deactivated */
    if (o && o->pDisasmbly) {
      SDShaderDisassemblyView->Enable();
      UpdateShaderText(SDShaderDisassemblyView, (char *)o->pDisasmbly->GetBufferPointer());
    }
    else {
      SDShaderDisassemblyView->Disable();
      UpdateShaderText(SDShaderDisassemblyView, "");
    }

#if 0
    SDShaderSourceEditor->ShowPosition(epos);
    SDShaderAssemblyEditor->ShowPosition(apos);
    SDShaderDisassemblyView->ShowPosition(dpos);

    if (SDShaderSourceEditor->ScrollLines(1))
      SDShaderSourceEditor->ScrollLines(-1);
    if (SDShaderAssemblyEditor->ScrollLines(1))
      SDShaderAssemblyEditor->ScrollLines(-1);
    if (SDShaderDisassemblyView->ScrollLines(1))
      SDShaderDisassemblyView->ScrollLines(-1);
#endif

    SetShaderConstantTable(o);
    SetShaderSamplerTable(o);
  }

  /* --------------------------------------------------------------
   */

  void GetEffectRecordStatus(EffectRecord *o) {
    char *status = "";
    char buf[256];

    int x = SDComboEffect->GetSelection();
    if (x == wxNOT_FOUND)
      if (!SDComboEffect->IsEmpty())
	SDComboEffect->SetSelection(x = 0);

    if (1) {
      SDEffectEnable->Enable();

      if (o && o->IsEnabled())
	SDEffectEnable->SetValue(wxCHK_CHECKED);
      else
	SDEffectEnable->SetValue(wxCHK_UNCHECKED);
    }
    else {
      SDEffectEnable->Disable();

      if (o && o->IsEnabled())
        SDEffectEnable->SetValue(wxCHK_CHECKED);
      else
        SDEffectEnable->SetValue(wxCHK_UNCHECKED);
    }

    if (o && o->pErrorMsgs) {
      char *buf = (char *)o->pErrorMsgs->GetBufferPointer();

      if (strstr(buf, "error")) {
      	/* two errors possible:
      	 * - runtime failed, replacement good
      	 * - both failed
      	 */
      	status = "Status: compiled, with errors, unavailable";
	SDEffectCompile->Enable();
	if (x != wxNOT_FOUND)
	  SDComboEffect->SetItemBitmap(x, wxBMError);
      }
      else if (strstr(buf, "warning")) {
      	/* two warnings possible:
      	 * - runtime warning
      	 * - replacement warning
      	 */
      	status = "Status: compiled, with warnings, available";
	SDEffectCompile->Enable();
	if (x != wxNOT_FOUND)
	  SDComboEffect->SetItemBitmap(x, wxBMWarning);
      }
      else {
	status = "Status: compiled, available";
	SDEffectCompile->Disable();
	if (x != wxNOT_FOUND)
	  SDComboEffect->SetItemBitmap(x, wxBMApplied);
      }
    }
    else {
      if (o && o->pEffect) {
      	/* two activations possible:
      	 * - runtime
      	 * - replacement
      	 */
      	status = "Status: compiled, available";
	SDEffectCompile->Disable();
	if (x != wxNOT_FOUND)
	  SDComboEffect->SetItemBitmap(x, wxBMApplied);
      }
      else if (o && !o->pSource) {
	status = "Status: missing";
	SDEffectCompile->Disable();
	if (x != wxNOT_FOUND)
	  SDComboEffect->SetItemBitmap(x, wxBMMissing);
      }
      else {
	status = "Status: not compiled";
	SDEffectCompile->Enable();
	if (x != wxNOT_FOUND)
	  SDComboEffect->SetItemBitmap(x, wxBMUnhooked);
      }
    }

    sprintf(buf, status, "");
    SDStatusEffect->SetLabel(buf);

    if (o && o->pSource && !o->pEffect)
      SDEffectCompile->Enable();
  }

  void UpdateEffectRecord(EffectRecord *o) {
    GetEffectRecordStatus(o);

    /* source has changed */
    wxString oo = SDEffectSourceEditor->GetValue();
    const wxChar *ooo = oo.GetData();
    int size = strlen(ooo);

    /* prevent flickering */
    if (o) {
      if (o->pEffect) {
	if (strcmp(oo.GetData(), o->pSource))
	  SDButtonEffectSave->Enable();
	else
	  SDButtonEffectSave->Disable();
      }
      else
	SDButtonEffectSave->Enable();
    }
    else
      SDButtonEffectSave->Disable();
  }

  void UpdateEffectText(wxTextCtrl *txt, const char *val) {
    /* source has changed */
    wxString oo = txt->GetValue();
    const wxChar *ooo = oo.GetData();
    int size = strlen(ooo);

    if (strcmp(oo.GetData(), val)) {
      txt->SetValue(wxString(val));
    }
  }

  void SetEffectRecord(EffectRecord *o) {
    if (o && !o->pEffect)
      o->CompileEffect(em, true);

    GetEffectRecordStatus(o);
    UpdateEffectOptions(o);

    SDButtonEffectSave->Disable();

#if 0
    int epos = SDEffectSourceEditor->GetLastPosition();
#endif

    /* HLSL goes always (load/save/saveas) */
    if (o && o->pSource)
      UpdateEffectText(SDEffectSourceEditor, o->pSource);
    else
      UpdateEffectText(SDEffectSourceEditor, "");

    /* Errors are only deactivated */
    if (o && o->pErrorMsgs) {
      SDEffectErrorView->Enable();
      UpdateEffectText(SDEffectErrorView, (char *)o->pErrorMsgs->GetBufferPointer());
    }
    else {
      SDEffectErrorView->Disable();
      UpdateEffectText(SDEffectErrorView, "");
    }

    /* Disassembly is only deactivated */
    if (o && o->pDisasmbly) {
      SDEffectDisassemblyView->Enable();
      UpdateEffectText(SDEffectDisassemblyView, (char *)o->pDisasmbly->GetBufferPointer());
    }
    else {
      SDEffectDisassemblyView->Disable();
      UpdateEffectText(SDEffectDisassemblyView, "");
    }

#if 0
    SDEffectSourceEditor->ShowPosition(epos);

    if (SDEffectSourceEditor->ScrollLines(1))
      SDEffectSourceEditor->ScrollLines(-1);
#endif

    SetEffectConstantTable(o);
    SetEffectTextureTable(o);
  }

  /* --------------------------------------------------------------
   */

  void SDPaintRT(wxPaintEvent& event) {
    wxPaintDC dc(SDRendertargetView);

    /* refresh only if visible */
    if (SDViewSwitch->GetSelection() != SDVIEW_SCENES)
      return;
    if (SDSurfaceSwitch->GetCurrentPage() != SDRendertarget)
      return;

    wxRect rg = SDRendertargetView->GetUpdateRegion().GetBox();
    int dw = SDRendertargetView->GetClientSize().GetWidth();
    int dh = SDRendertargetView->GetClientSize().GetHeight();
    int sw = crt.GetSize().GetWidth();
    int sh = crt.GetSize().GetHeight();

#if 0
    if (crt.IsOk())
      dc.Blit(
	0,
	0,
	SDRendertargetView->GetClientSize().GetWidth(),
	SDRendertargetView->GetClientSize().GetHeight(),
	&crt,
	0,
	0,
	wxCOPY
      );
#elif 1
    if (crt.IsOk())
      dc.StretchBlit(
	0,
	0,
	dw,
	dh,
	&crt,
	0,
	0,
	sw,
	sh,
	wxCOPY
      );
#else
    if (crt.IsOk())
    if ((rg.GetWidth() > 0) &&
    	(rg.GetHeight() > 0))
      dc.StretchBlit(
	rg.GetLeft(),
	rg.GetTop(),
	rg.GetWidth(),
	rg.GetHeight(),
	&crt,
	(rg.GetLeft() * sw) / dw,
	(rg.GetTop() * sh) / dh,
	(rg.GetWidth() * sw) / dw,
	(rg.GetHeight() * sh) / dh,
	wxCOPY
      );
#endif
  }

  void SDPaintGrabbedRT(wxPaintEvent& event) {
    wxPaintDC dc(SDRendertargetGrabbedView);

    /* refresh only if visible */
    if (SDViewSwitch->GetSelection() != SDVIEW_SCENES)
      return;
    if (SDSurfaceSwitch->GetCurrentPage() != SDRendertargetGrabbed)
      return;

    wxRect rg = SDRendertargetGrabbedView->GetUpdateRegion().GetBox();
    int dw = SDRendertargetGrabbedView->GetClientSize().GetWidth();
    int dh = SDRendertargetGrabbedView->GetClientSize().GetHeight();
    int sw = grt.GetSize().GetWidth();
    int sh = grt.GetSize().GetHeight();

#if 0
    if (crt.IsOk())
      dc.Blit(
      0,
      0,
      SDRendertargetGrabbedView->GetClientSize().GetWidth(),
      SDRendertargetGrabbedView->GetClientSize().GetHeight(),
      &grt,
      0,
      0,
      wxCOPY
      );
#elif 1
    if (grt.IsOk())
      dc.StretchBlit(
      0,
      0,
      dw,
      dh,
      &grt,
      0,
      0,
      sw,
      sh,
      wxCOPY
      );
#else
    if (grt.IsOk())
      if ((rg.GetWidth() > 0) &&
	(rg.GetHeight() > 0))
	dc.StretchBlit(
	rg.GetLeft(),
	rg.GetTop(),
	rg.GetWidth(),
	rg.GetHeight(),
	&grt,
	(rg.GetLeft() * sw) / dw,
	(rg.GetTop() * sh) / dh,
	(rg.GetWidth() * sw) / dw,
	(rg.GetHeight() * sh) / dh,
	wxCOPY
	);
#endif
  }

  void SDPaintDS(wxPaintEvent& event) {
    wxPaintDC dc(SDDepthStencilView);

    /* refresh only if visible */
    if (SDViewSwitch->GetSelection() != SDVIEW_SCENES)
      return;
    if (SDSurfaceSwitch->GetCurrentPage() != SDDepthStencil)
      return;

    wxRect rg = SDDepthStencilView->GetUpdateRegion().GetBox();
    int dw = SDDepthStencilView->GetClientSize().GetWidth();
    int dh = SDDepthStencilView->GetClientSize().GetHeight();
    int sw = cds.GetSize().GetWidth();
    int sh = cds.GetSize().GetHeight();

#if 0
    if (cds.IsOk())
      dc.Blit(
	0,
	0,
	SDDepthStencilView->GetClientSize().GetWidth(),
	SDDepthStencilView->GetClientSize().GetHeight(),
	&cds,
	0,
	0,
	wxCOPY
      );
#elif 1
    if (cds.IsOk())
      dc.StretchBlit(
        0,
	0,
	dw,
	dh,
        &cds,
        0,
	0,
	sw,
	sh,
        wxCOPY
      );
#else
    if (cds.IsOk())
    if ((rg.GetWidth() > 0) &&
    	(rg.GetHeight() > 0))
      dc.StretchBlit(
	rg.GetLeft(),
	rg.GetTop(),
	rg.GetWidth(),
	rg.GetHeight(),
	&cds,
	(rg.GetLeft() * sw) / dw,
	(rg.GetTop() * sh) / dh,
	(rg.GetWidth() * sw) / dw,
	(rg.GetHeight() * sh) / dh,
	wxCOPY
      );
#endif
  }

  const char *GetViewStatus(int p, int o, IDirect3DSurface9 *pSurface, bool comma = false) {
    D3DSURFACE_DESC VDesc;
    static char buf[256];
    buf[0] = '\0';

    if (!pSurface && sm) {
      pSurface = sm->trackd[p].rt[o];
      if ((int)pSurface == -1) pSurface = 0;
    }

    if (pSurface && (pSurface->GetDesc(&VDesc) == D3D_OK)) {
      sprintf(buf, "%s%s, %dx%d", comma ? ", " : "", findFormat(VDesc.Format), VDesc.Width, VDesc.Height);

      if (surfaceTexture[pSurface]) {
	D3DTEXTUREFILTERTYPE af = surfaceTexture[pSurface]->tex->GetAutoGenFilterType();
	if (surfaceTexture[pSurface]->map->Usage & D3DUSAGE_AUTOGENMIPMAP) {
	  int v = surfaceTexture[pSurface]->tex->GetLevelCount();
	  int l = 0, s = (VDesc.Width < VDesc.Height ? VDesc.Width : VDesc.Height);
	  while ((s >> 1) >= 1)
	    s >>= 1, l++;

	  sprintf(buf, "%s, AutoMip %d levels", buf, l);
	}
      }
    }

#ifdef	OBGE_PROFILE
    if (sm && (sm->trackd[p].frame_time[o].QuadPart > 0)) {
      char tme[256];
      LARGE_INTEGER freq;

      QueryPerformanceFrequency(&freq);

      float a = (float)((1000.0 * sm->trackd[p].frame_time[o].QuadPart) / freq.QuadPart);
      float b = (float)((1.0000 * freq.QuadPart) / sm->trackd[p].frame_time[o].QuadPart);

      sprintf(tme, "%s%.2f ms, %.2f SPS", (comma || (buf[0] != '\0')) ? ", " : "", a, b); strcat(buf, tme);
    }
#endif

    return buf;
  }

  void SetView(int o) {
    int p = pass; FindScene(p, o);

    IDirect3DSurface9 *_rt = NULL;
    IDirect3DSurface9 *_gt = NULL;
    IDirect3DSurface9 *_ds = NULL;

    if (sm) {
      _rt = sm->trackd[p].rt[o];
      _gt = FindGrabbedRT(0);
      _ds = FindGrabbedDS(0);//sm->trackd[p].ds[o];
      if ((int)_rt == -1) _rt = NULL;
      if ((int)_gt == -1) _gt = NULL;
      if ((int)_ds == -1) _ds = NULL;
    }

    SDStatusRT->SetLabel(wxString(GetViewStatus(p, o, _rt)));

    SetImage(p, &rt, _rt);
    SetImage(p, &gt, _gt);
    SetImage(p, &ds, _ds);

    crt.SelectObject(wxBitmap(rt));
    grt.SelectObject(wxBitmap(gt));
    cds.SelectObject(wxBitmap(ds));

//  SetImage(&rt, _rt); wxImage __rt = rt.Rescale(SDRendertargetView->GetClientSize().GetWidth(), SDRendertargetView->GetClientSize().GetHeight(), wxIMAGE_QUALITY_HIGH);
//  SetImage(&ds, _ds); wxImage __ds = ds.Rescale(SDDepthStencilView->GetClientSize().GetWidth(), SDDepthStencilView->GetClientSize().GetHeight(), wxIMAGE_QUALITY_HIGH);
//
//  SDRendertargetView->SetBitmap(wxBitmap(__rt));
//  SDDepthStencilView->SetBitmap(wxBitmap(__ds));

    SetStatesTable(o);

    DoRefresh();
  }

  void SetStats(int o, bool forced = false) {
    //    st->SetS
    // And finally create chart panel to display it:
//  st = new wxChartPanel(SDPanelStats, wxID_ANY, chrt, wxPoint(0,0), wxSize(256, 256));

    if (normed != (SDStatsNormalize->GetValue() == wxCHK_CHECKED)) {
      normed = (SDStatsNormalize->GetValue() == wxCHK_CHECKED);

      if (!normed) {
	leftAxis->SetTitle(wxT("ms"));
	bottomAxis->SetTitle(wxT("Frame"));
      }
      else {
	leftAxis->SetTitle(wxT("%"));
	bottomAxis->SetTitle(wxT("Frame"));
      }

      /* reset history */
      hist[pass].hconsm = 0;
    }

    if ((hist[pass].hconsm != sm->frame_capt) || forced) {
      char buf[256];
      LARGE_INTEGER freq;
      QueryPerformanceFrequency(&freq);

      /* cap shifting-in new data to OBGESCENE_NUM */
      int toadd = sm->frame_capt - hist[pass].hconsm;
      if (toadd > OBGEFRAME_NUM)
	toadd = OBGEFRAME_NUM;
      hist[pass].hconsm = sm->frame_capt - toadd;

      if (pass == OBGEPASS_ANY) {
	XYDynamicSerie *serie;

	/* start updating */
	vset->BeginUpdate();
	hist[pass].dset->BeginUpdate();

	/* loop over passes */
	for (int p = OBGEPASS_NUM; p >= OBGEPASS_MIN; p--) {
	  if (!(serie = hist[pass].series[p])) {
	    serie = new XYDynamicSerie();

	    // set serie names to be displayed on legend
	    if (p == OBGEPASS_NUM)
	      serie->SetName(wxString("Total"));
	    else
	      serie->SetName(wxString(passNames[p]));

	    hist[pass].dset->AddSerie(serie);
	    hist[pass].series[p] = serie; int c =
	    hist[pass].dset->GetSerieCount() - 1;

	    switch(p) {
	      case OBGEPASS_REFLECTION:
		rndrP->SetSerieColour(c, new wxColour(0x0090E000UL)); break;
	      case OBGEPASS_WATER:
		rndrP->SetSerieColour(c, new wxColour(0x00FF0000UL)); break;
	      case OBGEPASS_WATERHEIGHTMAP:
		rndrP->SetSerieColour(c, new wxColour(0x007F7F7FUL)); break;
	      case OBGEPASS_WATERDISPLACEMENT:
		rndrP->SetSerieColour(c, new wxColour(0x00007F9FUL)); break;
	      case OBGEPASS_SHADOW:
		rndrP->SetSerieColour(c, new wxColour(0x00010101UL)); break;
	      case OBGEPASS_MAIN:
		rndrP->SetSerieColour(c, new wxColour(0x0010C000UL)); break;
	      case OBGEPASS_EFFECTS:
		rndrP->SetSerieColour(c, new wxColour(0x00E09000UL)); break;
	      case OBGEPASS_HDR:
		rndrP->SetSerieColour(c, new wxColour(0x00F0E000UL)); break;
	      case OBGEPASS_POST:
		rndrP->SetSerieColour(c, new wxColour(0x00CFCFCFUL)); break;

	      case OBGEPASS_VIDEO:
		rndrP->SetSerieColour(c, new wxColour(0x00CFCFCFUL)); break;
	      case OBGEPASS_UNKNOWN:
		rndrP->SetSerieColour(c, new wxColour(0x00CFCFCFUL)); break;

	      case OBGEPASS_NUM:
		rndrP->SetSerieColour(c, new wxColour(0x00FFFFFFUL)); break;
	    }
	  }
	}

	/* loop over passes */
	for (int p = OBGEPASS_NUM; p >= OBGEPASS_MIN; p--) {
	  serie = hist[pass].series[p];

	  /* cap the number of resulting data to OBGEFRAME_NUM */
	  int tokll = (serie->GetCount() + toadd) - OBGEFRAME_NUM;
	  if (tokll < 0)
	    tokll = 0;
	  if (tokll > serie->GetCount())
	    tokll = serie->GetCount();
	  if (tokll > 0)
	    serie->Remove(0, tokll);
	}

	LARGE_INTEGER xbase; int xmul; int smth = 0;
	LARGE_INTEGER fsum; fsum.QuadPart = 0;
	LARGE_INTEGER csum; csum.QuadPart = 0;
	for (int h = hist[pass].hconsm; h < sm->frame_capt; h++) {
	  int frame_wrp = h % OBGEFRAME_NUM;

	  /* we assume this is wrong ... */
	  if (sm->trackh[frame_wrp].frame_totl.QuadPart > freq.QuadPart)
	    continue;

	  /* define renormalization ranged */
	  if (!normed)
	    xmul = 1000, xbase.QuadPart = freq.QuadPart;
	  else
	    xmul =  100, xbase.QuadPart = sm->trackh[frame_wrp].frame_totl.QuadPart;

	  LARGE_INTEGER tsum; tsum.QuadPart = 0;
	  /* loop over passes */
	  for (int p = OBGEPASS_MIN; p < OBGEPASS_NUM; p++) {

	    LARGE_INTEGER psum; psum.QuadPart = 0;
	    /* loop over scenes */
	    for (int s = 0; s < sm->trackh[frame_wrp].trackd[p].frame_cntr; s++) {
	      psum.QuadPart += sm->trackh[frame_wrp].trackd[p].frame_hist[s].QuadPart;
	    }

	    /* sum up times */
	    tsum.QuadPart += psum.QuadPart;

	    serie = hist[pass].series[p];
	    serie->AddXY(h, (double)(tsum.QuadPart * xmul) / xbase.QuadPart);
	  }

	  /* total frame-time incl. code */
	  if (sm->trackh[frame_wrp].frame_totl.QuadPart < freq.QuadPart) {
	    smth++; /* take out frames that took more than a second */

	    fsum.QuadPart = fsum.QuadPart + tsum.QuadPart;
	    csum.QuadPart = csum.QuadPart + sm->trackh[frame_wrp].frame_totl.QuadPart;
	  }

	  tsum.QuadPart = sm->trackh[frame_wrp].frame_totl.QuadPart;

	  serie = hist[pass].series[OBGEPASS_NUM];
	  serie->AddXY(h, (double)(tsum.QuadPart * xmul) / xbase.QuadPart);
	}

	/* start updating */
	hist[pass].dset->EndUpdate();
	vset->EndUpdate();

	double fps = (double)freq.QuadPart / (fsum.QuadPart / smth);
	double prc = (100.0 * fsum.QuadPart) / csum.QuadPart;

	sprintf(buf, "Global: %f FPS, %f%% frame-time", (float)fps, (float)prc);

	SDStatusStats->SetLabel(buf);
      }
      else {
	XYDynamicSerie *serie;

	/* start updating */
	vset->BeginUpdate();
	hist[pass].dset->BeginUpdate();

	/* determine till where to go */
	int OBGESCENE_CNT = 0;
	for (int h = hist[pass].hconsm; h < sm->frame_capt; h++) {
	  int frame_wrp = h % OBGEFRAME_NUM;
	  if (OBGESCENE_CNT < sm->trackh[frame_wrp].trackd[pass].frame_cntr)
	    OBGESCENE_CNT = sm->trackh[frame_wrp].trackd[pass].frame_cntr;
	}

	if (hist[pass].maxcol < OBGESCENE_CNT)
	  hist[pass].maxcol = OBGESCENE_CNT;

	/* loop over scenes */
	for (int s = OBGESCENE_CNT/* - 1*/; s >= 0; s--) {
	  if (!(serie = hist[pass].series[s])) {
	    serie = new XYDynamicSerie();

	    hist[pass].dset->AddSerie(serie);
	    hist[pass].series[s] = serie; int c =
	    hist[pass].dset->GetSerieCount() - 1;
	  }

	  int col = POStoRGB(s, hist[pass].maxcol);
	  rndrS->SetSerieColour(s, new wxColour(col));

	  /* if we switch an effect off for example, we have to re-assign names */
	  if (1) {
	    struct ShaderManager::track *t = &sm->trackd[pass];

	    if (s == OBGESCENE_CNT)
	      serie->SetName(wxString("Total"));
	    else {
	      /* info with pass-id */
	      if ((t->frame_used[s] > 0) && t->frame_name[s])
		sprintf(buf, "Scene %d: %s", s, t->frame_name[s]);
	      else
		sprintf(buf, "Scene %d", s);

	      serie->SetName(wxString(buf));
	    }
	  }
	}

	/* loop over scenes */
	for (int s = OBGESCENE_CNT/* - 1*/; s >= 0; s--) {
	  serie = hist[pass].series[s];

	  /* cap the number of resulting data to OBGEFRAME_NUM */
	  int tokll = (serie->GetCount() + toadd) - OBGEFRAME_NUM;
	  if (tokll < 0)
	    tokll = 0;
	  if (tokll > serie->GetCount())
	    tokll = serie->GetCount();
	  if (tokll > 0)
	    serie->Remove(0, tokll);
	}

	LARGE_INTEGER xbase; int xmul; int smth = 0;
	LARGE_INTEGER fsum; fsum.QuadPart = 0;
	LARGE_INTEGER csum; csum.QuadPart = 0;
	for (int h = hist[pass].hconsm; h < sm->frame_capt; h++) {
	  int frame_wrp = h % OBGEFRAME_NUM;

	  /* we assume this is wrong ... */
	  if (sm->trackh[frame_wrp].frame_totl.QuadPart > freq.QuadPart)
	    continue;

	  /* loop over scenes */
	  LARGE_INTEGER totl; totl.QuadPart = 0;
	  for (int s = 0; s < sm->trackh[frame_wrp].trackd[pass].frame_cntr; s++)
	    totl.QuadPart += sm->trackh[frame_wrp].trackd[pass].frame_hist[s].QuadPart;

	  /* define renormalization ranged */
	  if (!normed)
	    xmul = 1000, xbase.QuadPart = freq.QuadPart;
	  else
	    xmul =  100, xbase.QuadPart = totl.QuadPart;

	  LARGE_INTEGER tsum; tsum.QuadPart = 0;
	  /* loop over scenes */
	  for (int s = 0; s < sm->trackh[frame_wrp].trackd[pass].frame_cntr; s++) {

	    LARGE_INTEGER psum; psum.QuadPart = 0;
	    psum.QuadPart += sm->trackh[frame_wrp].trackd[pass].frame_hist[s].QuadPart;

	    /* sum up times */
	    tsum.QuadPart += psum.QuadPart;

	    serie = hist[pass].series[s];
	    serie->AddXY(h, (double)(tsum.QuadPart * xmul) / xbase.QuadPart);
	  }

	  /* total frame-time incl. code */
	  if (sm->trackh[frame_wrp].frame_totl.QuadPart < freq.QuadPart) {
	    smth++; /* take out frames that took more than a second */

	    fsum.QuadPart = fsum.QuadPart + tsum.QuadPart;
	    csum.QuadPart = csum.QuadPart + sm->trackh[frame_wrp].frame_totl.QuadPart;
	  }

	  tsum.QuadPart = totl.QuadPart;

	  serie = hist[pass].series[OBGESCENE_CNT];
	  serie->AddXY(h, (double)(tsum.QuadPart * xmul) / xbase.QuadPart);
	}

	/* loop over scenes */
	for (int s = OBGESCENE_CNT/* - 1*/; s >= 0; s--) {
	  serie = hist[pass].series[s];

	  if (serie->GetCount() == 0)
	    serie->AddXY(hist[pass].hconsm % OBGEFRAME_NUM, 0);
	}

	/* start updating */
	hist[pass].dset->EndUpdate();
	vset->EndUpdate();

	double fps = (double)freq.QuadPart / (fsum.QuadPart / smth);
	double prc = (100.0 * fsum.QuadPart) / csum.QuadPart;

	sprintf(buf, "Global: %f FPS, %f%% frame-time", (float)fps, (float)prc);

	SDStatusStats->SetLabel(buf);
      }

      hist[pass].hconsm = sm->frame_capt;
    //st->ChartChanged(chrt);
    //st->SetChart(chrt);
    }

    /* replace active dataset */
    if (aset != hist[pass].dset) {
      vset->BeginUpdate();

      if (pass == OBGEPASS_ANY)
	vset->SetRenderer(rndrP);
      else
	vset->SetRenderer(rndrS);

      vset->TakeoverSeries(aset = hist[pass].dset);
      vset->EndUpdate();
    }
  }

  /* --------------------------------------------------------------
   */

  void UpdateFrameShaders() {
    /* ------------------------------------------------ */
    char buf[256];
    int xx = SDComboShader->GetCount();
    int x = 0;

    for (x = 0; x < xx; x++) {
      wxString oo = SDComboShader->GetString(x);
      ShaderRecord *o = FindShaderRecord(NULL, oo.GetData());

      if (o) {
	RuntimeShaderRecord *r = o->pAssociate;

	/* info with pass-id */
	if (r && (pass == OBGEPASS_ANY)) {
	  char num[256] = ""; bool has = false;
	  for (int p = OBGEPASS_MIN; p < OBGEPASS_NUM; p++) {
	    if (r->frame_used[p] >= 0) {
	      if (!has)
		sprintf(num, "%d", p);
	      else
		sprintf(num, "%s, %d", num, p);

	      has = true;
	    }
	  }

	  if (!has)
	    sprintf(buf, "%s", o->Name);
	  else
	    sprintf(buf, "%s [in pass %s]", o->Name, num);

	  SDComboShader->SetString(x, wxString(buf));
	}
	/* info with frame-number and scene-identifier */
	else if (r && (r->frame_used[pass] >= 0) && (r->frame_pass[pass] > 0)) {
	  sprintf(buf, "%s [frame %d, scene %d]", o->Name, r->frame_used[pass], r->frame_pass[pass]);
	  SDComboShader->SetString(x, wxString(buf));
	}
	/* info with frame-number */
	else if (r && (r->frame_used[pass] >= 0)) {
	  sprintf(buf, "%s [frame %d]", o->Name, r->frame_used[pass]);
	  SDComboShader->SetString(x, wxString(buf));
	}
	/* just name */
	else
	  SDComboShader->SetString(x, wxString(o->Name));
      }
    }
  }

  void UpdateFrameEffects() {
    /* ------------------------------------------------ */
    char buf[256];
    int xx = SDComboEffect->GetCount();
    int x = 0;

    for (x = 0; x < xx; x++) {
      wxString oo = SDComboEffect->GetString(x);
      EffectRecord *o = FindEffectRecord(NULL, oo.GetData());

      if (o) {
	if (o->Private)
	  sprintf(buf, "%s [private]", o->Name);
	else
	  sprintf(buf, "%s", o->Name);

	SDComboEffect->SetString(x, wxString(buf));
      }
    }
  }

  void UpdateFrameScene() {
    char buf[256];

    int o = SDChoiceScene->GetSelection();
    SDChoiceScene->Clear();

    if (sm) {
      /* info with pass-id */
      if ((pass == OBGEPASS_ANY) && sm) {
	for (int p = OBGEPASS_MIN; p < OBGEPASS_NUM; p++) {
	  for (int scene = 0; scene < OBGESCENE_NUM; scene++) {
	    struct ShaderManager::track *t = &sm->trackd[p];

	    /* verify if the scene (which is a global identifier) occured in this pass, filter */
	    if (t->frame_used[scene] > 0) {
	      const char *fmt = GetViewStatus(p, scene, NULL, true);

	      /* info with pass-id */
	      if (t->frame_name[scene])
		sprintf(buf, "Pass %d, Scene %d%s: %s", p, scene, fmt, t->frame_name[scene]);
	      else
		sprintf(buf, "Pass %d, Scene %d%s", p, scene, fmt);

	      SDChoiceScene->Append(wxString(buf));
	    }
	  }
	}
      }
      /* info with frame counter */
      else {
	struct ShaderManager::track *t = &sm->trackd[pass];

	for (int scene = 0; scene < t->frame_cntr; scene++) {
      	  assert(scene < 256);

      	  /* verify if the scene (which is a global identifier) occured in this pass, filter */
      	  if (t->frame_used[scene] > 0) {
	    const char *fmt = GetViewStatus(pass, scene, NULL, true);

	    /* info with frame-number */
	    if (t->frame_name[scene])
	      sprintf(buf, "Scene %d%s [frame %d]: %s", t->frame_pass[scene], fmt, t->frame_used[scene], t->frame_name[scene]);
	    else
	      sprintf(buf, "Scene %d%s [frame %d]", t->frame_pass[scene], fmt, t->frame_used[scene]);

	    SDChoiceScene->Append(wxString(buf));
	  }
	}
      }
    }

    if (!SDChoiceScene->IsEmpty()) {
      SDPanelScenes->Show();
      if (o <= SDChoiceScene->GetCount())
	SDChoiceScene->SetSelection(o);
      else
	SDChoiceScene->SetSelection(0);
    }
    else
      SDPanelScenes->Hide();
  }

  void UpdateFrameStats() {
  }

  void UpdateFrame() {
    /**/ if (SDViewSwitch->GetSelection() == SDVIEW_SHADER)
      UpdateFrameShaders();
    else if (SDViewSwitch->GetSelection() == SDVIEW_EFFECT)
      UpdateFrameEffects();
    else if (SDViewSwitch->GetSelection() == SDVIEW_SCENES)
      UpdateFrameScene();
    else if (SDViewSwitch->GetSelection() == SDVIEW_STATS)
      UpdateFrameStats();
  }

  /* --------------------------------------------------------------
   */

  void SetPanels(int sel, wxString *resel = NULL, bool force = false) {
    /* ------------------------------------------------ */
    if (sel == SDVIEW_SHADER) {
      /* not changeable */
      int j = SDComboShader->GetSelection();
      int c = SDComboShader->GetCount();
      if ((j == wxNOT_FOUND) || (c != sm->BuiltInShaders.size()) || force) {
	wxString jj = SDComboShader->GetStringSelection();
	SDComboShader->Clear();

	/* overwrite old for re-selection */
	if (resel)
	  jj = *resel;

	if (sm) {
	  BuiltInShaderList::iterator BShader = sm->BuiltInShaders.begin();
	  while (BShader != sm->BuiltInShaders.end()) {
	    if ((*BShader)->pErrorMsgs) {
	      char *buf = (char *)(*BShader)->pErrorMsgs->GetBufferPointer();

	      if (strstr(buf, "error")) {
		SDComboShader->Append(wxBName, wxBMError, (void *)(*BShader));
	      }
	      else if (strstr(buf, "warning")) {
		SDComboShader->Append(wxBName, wxBMWarning, (void *)(*BShader));
	      }
	      else {
		SDComboShader->Append(wxBName, wxBMApplied, (void *)(*BShader));
	      }
	    }
	    else {
	      if ((*BShader)->pShaderRuntime) {
		SDComboShader->Append(wxBName, wxBMApplied, (void *)(*BShader));
	      }
	      else if ((*BShader)->pShaderReplaced) {
		SDComboShader->Append(wxBName, wxBMApplied, (void *)(*BShader));
	      }
	      else if (!(*BShader)->pShaderOriginal) {
		SDComboShader->Append(wxBName, wxBMMissing, (void *)(*BShader));
	      }
	      else {
		SDComboShader->Append(wxBName, wxBMUnhooked, (void *)(*BShader));
	      }
	    }

	    BShader++;
	  }
	}

	if (!SDComboShader->IsEmpty()) {
	  SDPanelShaders->Show();
	  SDComboShader->SetSelection(0);
	  if (j != wxNOT_FOUND)
	    SDComboShader->SetStringSelection(jj);

	  DoShaderSwitch(true);
	}
	else
	  SDPanelShaders->Hide();

	SDShaderCompileAll->Enable(!CheckShaderRecords());
      }
    }

    /* ------------------------------------------------ */
    else if (sel == SDVIEW_EFFECT) {
      /* always changeable */
      int j = SDComboEffect->GetSelection();
      int c = SDComboEffect->GetCount();
      if ((j == wxNOT_FOUND) || (c != em->ManagedEffects.size()) || force) {
	wxString jj = SDComboEffect->GetStringSelection();
	SDComboEffect->Clear();

	/* overwrite old for re-selection */
	if (resel)
	  jj = *resel;

	if (em) {
	  ManagedEffectList::iterator MEffect = em->ManagedEffects.begin();
	  while (MEffect != em->ManagedEffects.end()) {
	    if ((*MEffect)->pErrorMsgs) {
	      char *buf = (char *)(*MEffect)->pErrorMsgs->GetBufferPointer();

	      if (strstr(buf, "error")) {
		SDComboEffect->Append(wxMName, wxBMError, (void *)(*MEffect));
	      }
	      else if (strstr(buf, "warning")) {
		SDComboEffect->Append(wxMName, wxBMWarning, (void *)(*MEffect));
	      }
	      else {
		SDComboEffect->Append(wxMName, wxBMApplied, (void *)(*MEffect));
	      }
	    }
	    else {
	      SDComboEffect->Append(wxMName, wxBMApplied, (void *)(*MEffect));
	    }

	    MEffect++;
	  }
	}

	if (!SDComboEffect->IsEmpty()) {
	  SDPanelEffects->Show();
	  SDComboEffect->SetSelection(0);
	  if ((j != wxNOT_FOUND) || resel)
	    SDComboEffect->SetStringSelection(jj);

	  DoEffectSwitch(true);
	}
	else
	  SDPanelEffects->Hide();
      }
    }

    /* ------------------------------------------------ */
    else if (sel == SDVIEW_SCENES) {
      int k = SDChoiceScene->GetSelection();
      int c = SDChoiceScene->GetCount();
      if ((k == wxNOT_FOUND) || force) {
	if (!SDChoiceScene->IsEmpty()) {
	  SDPanelScenes->Show();
	  SDChoiceScene->SetSelection(0);

	  DoScenesSwitch(true);
	}
	else
	  SDPanelScenes->Hide();
      }
    }

    /* ------------------------------------------------ */
    else if (sel == SDVIEW_STATS) {
      int k = SDChoiceStats->GetSelection();
      int c = SDChoiceStats->GetCount();
      if ((k == wxNOT_FOUND) || 1 || force) {
      //if (!SDChoiceStats->IsEmpty()) {
	  SDPanelStats->Show();
	//SDChoiceStats->SetSelection(0);

	  DoStatsSwitch(true);
      //}
      //else
	//SDPanelStats->Hide();
      }
    }
  }

  /* --------------------------------------------------------------
   */

  bool CheckShaderRecords() {
    if (sm) {
      BuiltInShaderList::iterator BShader = sm->BuiltInShaders.begin();
      while (BShader != sm->BuiltInShaders.end()) {
	if (!(*BShader)->pDisasmbly)
	  return false;

	BShader++;
      }
    }

    return true;
  }

  ShaderRecord *FindShaderRecord(ShaderRecord *o, wxString oo) {
    const char *ooo = oo.GetData();

    if (sm) {
      BuiltInShaderList::iterator BShader = sm->BuiltInShaders.begin();
      while (BShader != sm->BuiltInShaders.end()) {
	if (ooo == strstr(ooo, (*BShader)->Name)) {
	  o = (*BShader);
	  break;
	}

	BShader++;
      }
    }

    return o;
  }

  EffectRecord *FindEffectRecord(EffectRecord *o, wxString oo) {
    const char *ooo = oo.GetData();

    if (em) {
      ManagedEffectList::iterator MEffect = em->ManagedEffects.begin();
      while (MEffect != em->ManagedEffects.end()) {
	if (ooo == strstr(ooo, (*MEffect)->Name)) {
	  o = (*MEffect);
	  break;
	}

	MEffect++;
      }
    }

    return o;
  }

  int FindRenderpass(wxString jj) {
    const char *jjj = jj.GetData();

    for (int p = 0; p < OBGEPASS_NUM; p++) {
      if (jjj == strstr(jjj, passNames[p]))
	return p;
    }

    return NULL;
  }

  int FindView(wxString vv) {
    const char *vvv = vv.GetData();
    char buf[256];

    for (int p = 0; p < 256; p++) {
      sprintf(buf, "Scene %d", p);
      if (vvv == strstr(vvv, buf))
	return p;
    }

    return NULL;
  }

  void FindScene(int &p, int &o) {
    if ((p == OBGEPASS_ANY) && sm) {
      int s = 0;
      for (int _p = OBGEPASS_MIN; _p < OBGEPASS_NUM; _p++) {
	for (int _o = 0; _o < OBGESCENE_NUM; _o++) {
	  if (sm->trackd[_p].frame_used[_o] > 0) {
	    if (o == s) {
	      p = _p;
	      o = _o;
	      return;
	    }
	    s++;
	  }
	}
      }
    }
  }

  /* --------------------------------------------------------------
   */

  // Virtual event handlers, overide them in your derived class
  virtual void DoActivate(wxActivateEvent& event) {
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
    ::ShowCursor(TRUE);

    /* options have changed since last activation */
    sm = NULL;
    if ((sm = ShaderManager::GetSingleton())) {
      wxMenuItem *mi;

      mi = SDShaderOptions->FindChildItem(wxID_SCOMPILE,  NULL); mi->Check(sm->CompileSources());
      mi = SDShaderOptions->FindChildItem(wxID_SSAVEBIN,  NULL); mi->Check(sm->SaveShaderOverride());
      mi = SDShaderOptions->FindChildItem(wxID_SLEGACY,   NULL); mi->Check(sm->UseLegacyCompiler());
      mi = SDShaderOptions->FindChildItem(wxID_SOPTIMIZE, NULL); mi->Check(sm->Optimize());
      mi = SDShaderOptions->FindChildItem(wxID_SMAXIMUM,  NULL); mi->Check(sm->MaximumSM());
      mi = SDShaderOptions->FindChildItem(wxID_SUPGRADE,  NULL); mi->Check(sm->UpgradeSM());
      mi = SDShaderOptions->FindChildItem(wxID_SRUNTIME,  NULL); mi->Check(sm->RuntimeSources());
    }

    /* options have changed since last activation */
    em = NULL;
    if ((em = EffectManager::GetSingleton())) {
      wxMenuItem *mi;

      mi = SDEffectOptions->FindChildItem(wxID_ECOMPILE,  NULL); mi->Check(em->CompileSources());
//    mi = SDEffectOptions->FindChildItem(wxID_ESAVEBIN,  NULL); mi->Check(em->SaveEffectOverride());
      mi = SDEffectOptions->FindChildItem(wxID_ELEGACY,   NULL); mi->Check(em->UseLegacyCompiler());
      mi = SDEffectOptions->FindChildItem(wxID_EOPTIMIZE, NULL); mi->Check(em->Optimize());
    }

#if defined(OGBE_PROFILE) || 1
    if (1) {
      wxMenuItem *mi;

      mi = SDProfileOptions->FindChildItem(wxID_PROFILE, NULL); mi->Enable(true);
      mi = SDProfileOptions->FindChildItem(wxID_KILLTEX, NULL); mi->Enable(true);

#if defined(OGBE_TESSELATION) || 1
      mi = SDProfileOptions->FindChildItem(wxID_WIREFRAME, NULL); mi->Enable(true);
      mi = SDProfileOptions->FindChildItem(wxID_TESSELATION, NULL); mi->Enable(true);
#endif
    }
#endif

    SDShaderFlush->Enable(sm->ChangedShaders());

    /* renderpasses might have changed since last activation */
    int j = SDChoicePass->GetSelection(), h = 0;
    wxString jj = SDChoicePass->GetStringSelection();
    const char *jjj = jj.GetData();
    char buf[256];

    /* ------------------------------------------------ */
    SDChoicePass->Clear();

    for (int p = OBGEPASS_ANY; p < OBGEPASS_NUM; p++) {
      if (!p || (passFrames[p] >= 0)) {
      	if (sm && (sm->trackd[p].frame_cntr > 1))
	  sprintf(buf, "%s [%d scenes]", passNames[p], sm->trackd[p].frame_cntr);
	else
	  sprintf(buf, "%s", passNames[p]);

	SDChoicePass->Append(wxString(buf), (void *)p);

	if (jjj == strstr(jjj, passNames[p]))
	  pass = p, j = h;
	h++;
      }
    }

    if (!SDChoicePass->IsEmpty())
      SDChoicePass->SetSelection(j >= 0 ? j : 0);
    else
      pass = 0;

    DoRenderpassSwitch(true);
    event.Skip();
  }

  virtual void DoRenderpassSwitch(wxCommandEvent& event) {
    DoRenderpassSwitch();
    event.Skip();
  }

  void DoRenderpassSwitch(bool forced = false) {
    int p = pass;
    pass = FindRenderpass(SDChoicePass->GetStringSelection());

    /* validate/Invalidate panels */
    SetPanels(SDViewSwitch->GetSelection());

    /* we basically assume the frame/scene numbers change all the time */
    UpdateFrame();

    if (SDViewSwitch->GetSelection() == SDVIEW_SHADER)
      DoShaderSwitch(forced || (p != pass));
    else if (SDViewSwitch->GetSelection() == SDVIEW_EFFECT)
      DoEffectSwitch(forced || (p != pass));
    else if (SDViewSwitch->GetSelection() == SDVIEW_SCENES)
      DoScenesSwitch(forced || (p != pass));
    else if (SDViewSwitch->GetSelection() == SDVIEW_STATS)
      DoStatsSwitch(forced || (p != pass));
  }

  /* --------------------------------------------------------------
   */

  virtual void DoViewSwitch(wxNotebookEvent& event) {
    int o = event.GetSelection();
    wxObject *hit = event.GetEventObject();

    if (hit == SDViewSwitch) {
      SetPanels(o);

      if (o == SDVIEW_SHADER) {
	SDPanelShaders->Show();
	SDPanelEffects->Hide();
	SDPanelScenes->Hide();
	SDPanelStats->Hide();

	UpdateFrameShaders();
      }
      else if (o == SDVIEW_EFFECT) {
	SDPanelShaders->Hide();
	SDPanelEffects->Show();
	SDPanelScenes->Hide();
	SDPanelStats->Hide();

	UpdateFrameEffects();
      }
      else if (o == SDVIEW_SCENES) {
	SDPanelShaders->Hide();
	SDPanelEffects->Hide();
	SDPanelScenes->Show();
	SDPanelStats->Hide();

	UpdateFrameScene();
      }
      else if (o == SDVIEW_STATS) {
	SDPanelShaders->Hide();
	SDPanelEffects->Hide();
	SDPanelScenes->Hide();
	SDPanelStats->Show();

	UpdateFrameStats();
      }
    }

    event.Skip();
  }

  virtual void DoSurfaceSwitch(wxNotebookEvent& event) {
    int o = event.GetSelection();
    wxObject *hit = event.GetEventObject();

    if (hit == SDSurfaceSwitch) {
      wxWindow *ow = SDSurfaceSwitch->GetPage(o);

      if (ow == SDRendertarget) {
	SDRendertarget->Show();
	SDRendertargetGrabbed->Hide();
	SDDepthStencil->Hide();
      }
      else if (ow == SDRendertargetGrabbed) {
	SDRendertarget->Hide();
	SDRendertargetGrabbed->Hide();
	SDDepthStencil->Show();
      }
      else if (ow == SDDepthStencil) {
	SDRendertarget->Hide();
	SDRendertargetGrabbed->Hide();
	SDDepthStencil->Show();
      }
    }

    event.Skip();
  }

  /* --------------------------------------------------------------
   */

  virtual void DoShaderSwitch(wxCommandEvent& event) {
    DoShaderSwitch();
    event.Skip();
  }

  void DoShaderSwitch(bool force = false) {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    if ((currs != o) /*|| force*/) {
      currs = o;
      SetShaderRecord(o);
    }

    /* trace might have changed since last activation */
    if (currs) {
      SetShaderConstSetTable(currs);
      SetShaderSamplerTable(currs);
    }
  }

  virtual void DoEffectSwitch(wxCommandEvent& event) {
    DoEffectSwitch();
    event.Skip();
  }

  void DoEffectSwitch(bool force = false) {
    EffectRecord *o = FindEffectRecord(NULL, SDComboEffect->GetStringSelection());

    /* ------------------------------------------------ */
    if ((currx != o) /*|| force*/) {
      currx = o;
      SetEffectRecord(o);
    }

    /* trace might have changed since last activation */
    if (currx) {
      SetEffectConstSetTable(currx);
      SetEffectTextureTable(currx);
    }
  }

  virtual void DoScenesSwitch(wxCommandEvent& event) {
    DoScenesSwitch();
    event.Skip();
  }

  void DoScenesSwitch(bool force = false) {
    int o = SDChoiceScene->GetSelection();

    /* ------------------------------------------------ */
    if ((currv != o) || force) {
      currv = o;
      SetView(o);
    }

    /* trace might have changed since last activation */
    if (currv)
      SetStatesTable(currv);
  }

  virtual void DoStatsSwitch(wxCommandEvent& event) {
    DoStatsSwitch();
    event.Skip();
  }

  void DoStatsSwitch(bool force = false) {
    int o = SDChoiceStats->GetSelection();

    /* ------------------------------------------------ */
    if ((currh != o) || force) {
      currh = o;
      SetStats(o);
    }

    /* trace might have changed since last activation */
    if (currh)
      SetStatsTable(currh);
  }

  /* --------------------------------------------------------------
   */

  virtual void DoShaderUpdate(wxCommandEvent& event) {
    DoShaderUpdate();
    event.Skip();
  }

  void DoShaderUpdate() {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    UpdateShaderRecord(o);
  }

  virtual void DoShaderConstantChange(wxGridEvent& event) {
    DoShaderConstantChange();

    wxGrid *gd = SDShaderConstSetGrid;
    wxString label = gd->GetRowLabelValue(event.GetRow());
    wxString newvl = gd->GetCellValue(event.GetRow(), event.GetCol());
    char name[256]; int rw = 0; bool arr = false;

    if (sscanf(label.GetData(), "%s[%d]", name, rw) == 2)
      arr = true;
    else if (sscanf(label.GetData(), "%s", name) == 1)
      arr = false;
    else
      return;

    LPD3DXCONSTANTTABLE x = currs->pDX9ShaderCoTa;
    D3DXHANDLE handle;

    if ((handle = x->GetConstantByName(NULL, name))) {
      D3DXCONSTANT_DESC Description;
      UINT count = 1;
      x->GetConstantDesc(handle, &Description, &count);

      switch (Description.Type) {
	case D3DXPT_INT:
	  IntType IntData;

	  IntData.size = Description.Elements;
	  if (IntData.size == 0)
	    IntData.size = 1;
	  IntData.size *= Description.Columns;

//	  x->GetIntArray(handle, (int *)&IntData.data, IntData.size * Description.Rows);

	  switch (Description.Columns) {
	    case 4:
	      if (sscanf(newvl, "%g,%g,%g,%g",
		    &IntData.data[rw * IntData.size + 0],
		    &IntData.data[rw * IntData.size + 1],
		    &IntData.data[rw * IntData.size + 2],
		    &IntData.data[rw * IntData.size + 3]
		  ) == 4)
		break;
	    case 3:
	      if (sscanf(newvl, "%g,%g,%g",
		    &IntData.data[rw * IntData.size + 0],
		    &IntData.data[rw * IntData.size + 1],
		    &IntData.data[rw * IntData.size + 2]
		  ) == 3)
		break;
	    case 2:
	      if (sscanf(newvl, "%g,%g",
		    &IntData.data[rw * IntData.size + 0],
		    &IntData.data[rw * IntData.size + 1]
		  ) == 2)
		break;
	    case 1:
	      if (sscanf(newvl, "%g",
		    &IntData.data[rw * IntData.size + 0]
		  ) == 1)
		break;
	    default:
	      break;
	  }

	  currs->pAssociate->SetShaderConstantI(name, (int *)&IntData.data);
	  break;
	case D3DXPT_FLOAT:
	  FloatType FloatData;

	  FloatData.size = Description.Elements;
	  if (FloatData.size == 0)
	    FloatData.size = 1;
	  FloatData.size *= Description.Columns;

//	  x->GetFloatArray(handle, (float *)&FloatData.data, FloatData.size * Description.Rows);

	  switch (Description.Columns) {
	    case 4:
	      if (sscanf(newvl, "%g,%g,%g,%g",
		    &FloatData.data[rw * FloatData.size + 0],
		    &FloatData.data[rw * FloatData.size + 1],
		    &FloatData.data[rw * FloatData.size + 2],
		    &FloatData.data[rw * FloatData.size + 3]
		  ) == 4)
		break;
	    case 3:
	      if (sscanf(newvl, "%g,%g,%g",
		    &FloatData.data[rw * FloatData.size + 0],
		    &FloatData.data[rw * FloatData.size + 1],
		    &FloatData.data[rw * FloatData.size + 2]
		  ) == 3)
		break;
	    case 2:
	      if (sscanf(newvl, "%g,%g",
		&FloatData.data[rw * FloatData.size + 0],
		&FloatData.data[rw * FloatData.size + 1]
	      ) == 2)
		break;
	    case 1:
	      if (sscanf(newvl, "%g",
		    &FloatData.data[rw * FloatData.size + 0]
		  ) == 1)
		break;
	    default:
	      break;
	  }

	  currs->pAssociate->SetShaderConstantF(name, (float *)&FloatData.data);
	  break;
      }
    }

//  event.Skip();
  }

  void DoShaderConstantChange() {
  }

  virtual void DoShaderConstantSelect(wxGridEvent& event) {
    DoShaderConstantSelect();
    event.Skip();
  }

  void DoShaderConstantSelect() {
  }

  virtual void DoShaderSamplerChange(wxGridEvent& event) {
    DoShaderSamplerChange();
    event.Skip();
  }

  void DoShaderSamplerChange() {
  }

  virtual void DoShaderSamplerSelect(wxGridEvent& event) {
    DoShaderSamplerSelect();
    event.Skip();
  }

  void DoShaderSamplerSelect() {
  }

  virtual void DoShaderLoad(wxCommandEvent& event) {
    DoShaderLoad();
    event.Skip();
  }

  void DoShaderLoad() {
  }

  virtual void DoShaderFlush(wxCommandEvent& event) {
    DoShaderFlush();
//  event.Skip();
  }

  void DoShaderFlush() {
    /* read changes from disk
    if (sm->ChangedShaders()) {
      wxMessageDialog dlg(
	this,
	_T("Changes in the shaders on-disk have been detected. Do you want to reload the related shaders?"),
	_T("Confirmation"),
	wxYES_NO
	);

      if (dlg.ShowModal() == wxID_YES) { */
	if (sm->ReloadShaders()) {
	  ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());
	  if (o)
	    SetShaderRecord(o);
	}/*
      }
    }*/
  }

  virtual void DoShaderSave(wxCommandEvent& event) {
    DoShaderSave();
//  event.Skip();
  }

  void DoShaderSave() {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    wxString oo = SDShaderSourceEditor->GetValue();
    const wxChar *ooo = oo.GetData();
    int size = strlen(ooo);

    if (o) {
      char strFileFull[MAX_PATH];
      strcpy(strFileFull, o->Filepath);
      strcat(strFileFull, ".hlsl");

      FILE *f;
      if (!fopen_s(&f, strFileFull, "wb"/*"wt"*/)) {
	fwrite(ooo, 1, size, f);
	fclose(f);

	o->hlslStamp = time(NULL);
	if (o->RuntimeShader(ooo))
	  SetShaderRecord(o);
        else
	  UpdateShaderRecord(o);

	SDShaderCompileAll->Enable(!CheckShaderRecords());

	/* not a runtime-shader (like IDENTIFY, COPYQUAD etc. */
	if (!o->pAssociate) {
	  o->ConstructDX9Shader(SHADER_RUNTIME); return; }

	/* trigger re-creation of the DX9-class */
	if (SDShaderEnable->Get3StateValue() == wxCHK_CHECKED)
	  o->pAssociate->ActivateShader(SHADER_RUNTIME);
	else if (SDShaderEnable->Get3StateValue() == wxCHK_UNDETERMINED)
	  o->pAssociate->ActivateShader(SHADER_REPLACED);
	else //if (SDShaderEnable->Get3StateValue() == wxCHK_UNCHECKED)
	  o->pAssociate->ActivateShader(SHADER_ORIGINAL);
      }
    }
  }

  virtual void DoShaderSaveAs(wxCommandEvent& event) {
    DoShaderSaveAs();
    event.Skip();
  }

  void DoShaderSaveAs() {
  }

  virtual void DoShaderVersion(wxCommandEvent& event) {
    DoShaderVersion();
//  event.Skip();
  }

  void DoShaderVersion() {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    wxString oo = SDShaderSourceEditor->GetValue();
    const wxChar *ooo = oo.GetData();
    int size = strlen(ooo);

    if (o) {
      wxString ss = SDShaderVersion->GetStringSelection();
      const wxChar *sss = ss.GetData();

      /* the profile changed */
      if (!o->pProfile || strcmp(sss, o->pProfile)) {
	if (o->RuntimeShader(ooo, sss))
	  SetShaderRecord(o);
	else
	  UpdateShaderRecord(o);

	SDShaderCompileAll->Enable(!CheckShaderRecords());

	/* not a runtime-shader (like IDENTIFY, COPYQUAD etc. */
	if (!o->pAssociate) {
	  o->ConstructDX9Shader(SHADER_RUNTIME); return; }

	/* trigger re-creation of the DX9-class */
	if (SDShaderEnable->Get3StateValue() == wxCHK_CHECKED)
	  o->pAssociate->ActivateShader(SHADER_RUNTIME);
	else if (SDShaderEnable->Get3StateValue() == wxCHK_UNDETERMINED)
	  o->pAssociate->ActivateShader(SHADER_REPLACED);
	else //if (SDShaderEnable->Get3StateValue() == wxCHK_UNCHECKED)
	  o->pAssociate->ActivateShader(SHADER_ORIGINAL);
      }
    }
  }

  virtual void DoShaderCompile(wxCommandEvent& event) {
    DoShaderCompile();
//  event.Skip();
  }

  void DoShaderCompile() {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    wxString oo = SDShaderSourceEditor->GetValue();
    const wxChar *ooo = oo.GetData();
    int size = strlen(ooo);

    if (o) {
      if (o->RuntimeShader(ooo))
	SetShaderRecord(o);
      else
	UpdateShaderRecord(o);

      SDShaderCompileAll->Enable(!CheckShaderRecords());

      /* not a runtime-shader (like IDENTIFY, COPYQUAD etc. */
      if (!o->pAssociate) {
	o->ConstructDX9Shader(SHADER_RUNTIME); return; }

      /* trigger re-creation of the DX9-class */
      if (SDShaderEnable->Get3StateValue() == wxCHK_CHECKED)
	o->pAssociate->ActivateShader(SHADER_RUNTIME);
      else if (SDShaderEnable->Get3StateValue() == wxCHK_UNDETERMINED)
	o->pAssociate->ActivateShader(SHADER_REPLACED);
      else //if (SDShaderEnable->Get3StateValue() == wxCHK_UNCHECKED)
	o->pAssociate->ActivateShader(SHADER_ORIGINAL);
    }
  }

  virtual void DoShaderCompileAll(wxCommandEvent& event) {
    DoShaderCompileAll();
//  event.Skip();
  }

  void DoShaderCompileAll() {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    if (sm) {
      BuiltInShaderList::iterator BShader = sm->BuiltInShaders.begin();
      while (BShader != sm->BuiltInShaders.end()) {
	if ((*BShader != o) && !(*BShader)->pDisasmbly)
	  SetShaderRecord(*BShader);

	BShader++;
      }
    }

    /* ------------------------------------------------ */
    if (!o->pDisasmbly)
      DoShaderCompile();

    SDShaderCompileAll->Enable(!CheckShaderRecords());
    SetPanels(SDVIEW_SHADER, NULL, true);
  }

  virtual void DoShaderToggle(wxCommandEvent& event) {
    DoShaderToggle();
//  event.Skip();
  }

  void DoShaderToggle() {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    if (o && o->pAssociate) {
      if (SDShaderEnable->Get3StateValue() == wxCHK_CHECKED)
	o->pAssociate->ActivateShader(SHADER_RUNTIME);
      else if (SDShaderEnable->Get3StateValue() == wxCHK_UNDETERMINED)
	o->pAssociate->ActivateShader(SHADER_REPLACED);
      else //if (SDShaderEnable->Get3StateValue() == wxCHK_UNCHECKED)
	o->pAssociate->ActivateShader(SHADER_ORIGINAL);
    }
  }

  virtual void DoMarkToggle(wxCommandEvent& event) {
    DoMarkToggle();
//  event.Skip();
  }

  void DoMarkToggle() {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    if (o && o->pAssociate) {
      /* for identification */
      sm->GetBuiltInShader("IDENTIFY.pso");

      if (SDShaderMark->GetValue() == wxCHK_CHECKED)
	o->pAssociate->bMark = true;
      else //if (SDShaderEnable->Get3StateValue() == wxCHK_UNCHECKED)
	o->pAssociate->bMark = false;
    }
  }

  /* --------------------------------------------------------------
   */

  virtual void DoEffectUpdate(wxCommandEvent& event) {
    DoEffectUpdate();
    event.Skip();
  }

  void DoEffectUpdate() {
    EffectRecord *o = FindEffectRecord(NULL, SDComboEffect->GetStringSelection());

    /* ------------------------------------------------ */
    UpdateEffectRecord(o);
  }

  virtual void DoEffectConstantChange(wxGridEvent& event) {
    DoEffectConstantChange();

    wxGrid *gd = SDEffectConstSetGrid;
    wxString label = gd->GetRowLabelValue(event.GetRow());
    wxString newvl = gd->GetCellValue(event.GetRow(), event.GetCol());
    char name[256]; int rw = 0; bool arr = false;

    if (sscanf(label.GetData(), "%s[%d]", name, rw) == 2)
      arr = true;
    else if (sscanf(label.GetData(), "%s", name) == 1)
      arr = false;
    else
      return;

    ID3DXEffect *x = currx->GetEffect();
    D3DXHANDLE handle;

    if ((handle = x->GetParameterByName(NULL, name))) {
      D3DXPARAMETER_DESC Description;
      x->GetParameterDesc(handle, &Description);

      switch (Description.Type) {
	case D3DXPT_INT:
	  IntType IntData;

	  IntData.size = Description.Elements;
	  if (IntData.size == 0)
	    IntData.size = 1;
	  IntData.size *= Description.Columns;

	  x->GetIntArray(handle, (int *)&IntData.data, IntData.size * Description.Rows);

	  switch (Description.Columns) {
	    case 4:
	      if (sscanf(newvl, "%g,%g,%g,%g",
		    &IntData.data[rw * IntData.size + 0],
		    &IntData.data[rw * IntData.size + 1],
		    &IntData.data[rw * IntData.size + 2],
		    &IntData.data[rw * IntData.size + 3]
		  ) == 4)
		break;
	    case 3:
	      if (sscanf(newvl, "%g,%g,%g",
		    &IntData.data[rw * IntData.size + 0],
		    &IntData.data[rw * IntData.size + 1],
		    &IntData.data[rw * IntData.size + 2]
		  ) == 3)
		break;
	    case 2:
	      if (sscanf(newvl, "%g,%g",
		    &IntData.data[rw * IntData.size + 0],
		    &IntData.data[rw * IntData.size + 1]
		  ) == 2)
		break;
	    case 1:
	      if (sscanf(newvl, "%g",
		    &IntData.data[rw * IntData.size + 0]
		  ) == 1)
		break;
	    default:
	      break;
	  }

	  x->SetIntArray(handle, (int *)&IntData.data, IntData.size * Description.Rows);
	  break;
	case D3DXPT_FLOAT:
	  FloatType FloatData;

	  FloatData.size = Description.Elements;
	  if (FloatData.size == 0)
	    FloatData.size = 1;
	  FloatData.size *= Description.Columns;

	  x->GetFloatArray(handle, (float *)&FloatData.data, FloatData.size * Description.Rows);

	  switch (Description.Columns) {
	    case 4:
	      if (sscanf(newvl, "%g,%g,%g,%g",
		    &FloatData.data[rw * FloatData.size + 0],
		    &FloatData.data[rw * FloatData.size + 1],
		    &FloatData.data[rw * FloatData.size + 2],
		    &FloatData.data[rw * FloatData.size + 3]
		  ) == 4)
		break;
	    case 3:
	      if (sscanf(newvl, "%g,%g,%g",
		    &FloatData.data[rw * FloatData.size + 0],
		    &FloatData.data[rw * FloatData.size + 1],
		    &FloatData.data[rw * FloatData.size + 2]
		  ) == 3)
		break;
	    case 2:
	      if (sscanf(newvl, "%g,%g",
		    &FloatData.data[rw * FloatData.size + 0],
		    &FloatData.data[rw * FloatData.size + 1]
		  ) == 2)
		break;
	    case 1:
	      if (sscanf(newvl, "%g",
		    &FloatData.data[rw * FloatData.size + 0]
		  ) == 1)
		break;
	    default:
	      break;
	  }

	  x->SetFloatArray(handle, (float *)&FloatData.data, FloatData.size * Description.Rows);
	  break;
      }
    }

//  event.Skip();
  }

  void DoEffectConstantChange() {
  }

  virtual void DoEffectConstantSelect(wxGridEvent& event) {
    DoEffectConstantSelect();
    event.Skip();
  }

  void DoEffectConstantSelect() {
  }

  virtual void DoEffectTextureChange(wxGridEvent& event) {
    DoEffectTextureChange();
    event.Skip();
  }

  void DoEffectTextureChange() {
  }

  virtual void DoEffectTextureSelect(wxGridEvent& event) {
    DoEffectTextureSelect();
    event.Skip();
  }

  void DoEffectTextureSelect() {
  }

  virtual void DoEffectAdd(wxCommandEvent& event) {
    DoEffectAdd();
  //event.Skip();
  }

  void DoEffectAdd() {
    wxFileDialog dlg(
	this,
	_T("Open effect"),
	wxT(em->EffectDirectory()),
	wxEmptyString,
	_T("Effects file (*.fx)|*.fx"),
	wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if (dlg.ShowModal() != wxID_OK)
      return;

    // get filename
    wxString m_filename = dlg.GetFilename();
    wxString m_filepath = dlg.GetPath();

    // some feedback
    bool list = false;
    int eid = -1;
    if ((eid = em->FindEffect(m_filename)) != -1) {
      wxMessageDialog dlg(
	this,
	_T("Effect already exist. Do you want to overwrite it?"),
	_T("Confirmation"),
	wxYES_NO
	);

      if (dlg.ShowModal() != wxID_YES)
	return;
    }
    else if (em->UseEffectList()) {
      wxMessageDialog dlg(
	this,
	_T("Do you want to add the effect to the shaderlist?"),
	_T("Confirmation"),
	wxYES_NO
	);

      list = (dlg.ShowModal() == wxID_YES);
    }

    /* transfer data */
    struct stat sx;
    char strFileFull[MAX_PATH];
    FILE *f;

    strcpy(strFileFull, em->EffectDirectory());
    strcat(strFileFull, m_filename);

    /* overwrite */
    if (!stat((const char *)m_filepath, &sx)) {
      UINT size = sx.st_size;
      CHAR *ooo = new CHAR[size + 1];
      if (ooo != NULL) {
	/* reading in text-mode can yield any number of less characters */
	memset(ooo, 0, size + 1);

	if (!fopen_s(&f, m_filepath, "rb"/*"rt"*/)) {
	  fread(ooo, 1, size, f);
	  fclose(f);

	  if (!fopen_s(&f, strFileFull, "wb"/*"rt"*/)) {
	    fwrite(ooo, 1, size, f);
	    fclose(f);

	    /* insert new effect */
	    if (eid == -1) {
	      if (em->AddManagedEffect(m_filename, 0) != -1) {
		/* append to list */
		if (list) {
		  if (!fopen_s(&f, em->EffectListFile(), "ab"/*"rt"*/)) {
		    fprintf(f, "%s\n", m_filename);
		    fclose(f);
		  }
		}

		SetPanels(SDViewSwitch->GetSelection(), &m_filename);
	      }
	    }
	    /* update previous effect */
	    else {
	      /* get previous effect */
	      EffectRecord *o = em->GetEffect(eid);

	      if (o) {
		if (o->RuntimeEffect(ooo))
		  SetEffectRecord(o);
		else
		  UpdateEffectRecord(o);

		/* trigger re-creation of the DX9-class */
		if (SDEffectEnable->GetValue() == wxCHK_CHECKED)
		  o->Enable(true);
		else
		  o->Enable(false);
	      }
	    }
	  }
	}

	delete[] ooo;
      }
    }
  }

  virtual void DoEffectLoad(wxCommandEvent& event) {
    DoEffectLoad();
    event.Skip();
  }

  void DoEffectLoad() {
  }

  virtual void DoEffectSave(wxCommandEvent& event) {
    DoEffectSave();
//  event.Skip();
  }

  void DoEffectSave() {
    EffectRecord *o = FindEffectRecord(NULL, SDComboEffect->GetStringSelection());

    /* ------------------------------------------------ */
    wxString oo = SDEffectSourceEditor->GetValue();
    const wxChar *ooo = oo.GetData();
    int size = strlen(ooo);

    if (o) {
      FILE *f;
      if (!fopen_s(&f, o->Filepath, "wb"/*"wt"*/)) {
	fwrite(ooo, 1, size, f);
	fclose(f);

	if (o->RuntimeEffect(ooo))
	  SetEffectRecord(o);
        else
	  UpdateEffectRecord(o);

	/* trigger re-creation of the DX9-class */
	if (SDEffectEnable->GetValue() == wxCHK_CHECKED)
	  o->Enable(true);
	else
	  o->Enable(false);
      }
    }
  }

  virtual void DoEffectSaveAs(wxCommandEvent& event) {
    DoEffectSaveAs();
    event.Skip();
  }

  void DoEffectSaveAs() {
  }

  virtual void DoEffectCompile(wxCommandEvent& event) {
    DoEffectCompile();
    event.Skip();
  }

  void DoEffectCompile() {
    EffectRecord *o = FindEffectRecord(NULL, SDComboEffect->GetStringSelection());

    /* ------------------------------------------------ */
    wxString oo = SDEffectSourceEditor->GetValue();
    const wxChar *ooo = oo.GetData();
    int size = strlen(ooo);

    if (o) {
      if (o->RuntimeEffect(ooo))
	SetEffectRecord(o);
      else
	UpdateEffectRecord(o);

      /* trigger re-creation of the DX9-class */
      if (SDEffectEnable->GetValue() == wxCHK_CHECKED)
	o->Enable(true);
      else
	o->Enable(false);
    }
  }

  virtual void DoEffectToggle(wxCommandEvent& event) {
    DoEffectToggle();
    event.Skip();
  }

  void DoEffectToggle() {
    EffectRecord *o = FindEffectRecord(NULL, SDComboEffect->GetStringSelection());

    /* ------------------------------------------------ */
    if (o) {
      if (SDEffectEnable->GetValue() == wxCHK_CHECKED)
	o->Enable(true);
      else
	o->Enable(false);
    }
  }

  /* --------------------------------------------------------------
   */

  virtual void DoStatsToggle(wxCommandEvent& event) {
    DoStatsToggle();
//  event.Skip();
  }

  void DoStatsToggle() {
    /* ------------------------------------------------ */
    if (SDStatsNormalize->GetValue() == wxCHK_CHECKED)
      ;
    else //if (SDStatsNormalize->GetValue() == wxCHK_UNCHECKED)
      ;

    SetStats(currh, true);
  }

  /* --------------------------------------------------------------
   */

  virtual void DoShaderOptions(wxCommandEvent& event) {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    UpdateShaderOptions(o);

    if (sm) {
      wxMenuItem *mi;

      mi = SDShaderOptions->FindChildItem(wxID_SCOMPILE,  NULL); sm->CompileSources(mi->IsChecked());
      mi = SDShaderOptions->FindChildItem(wxID_SSAVEBIN,  NULL); sm->SaveShaderOverride(mi->IsChecked());
      mi = SDShaderOptions->FindChildItem(wxID_SOPTIMIZE, NULL); sm->Optimize(mi->IsChecked());
      mi = SDShaderOptions->FindChildItem(wxID_SLEGACY,   NULL); sm->UseLegacyCompiler(mi->IsChecked());
      mi = SDShaderOptions->FindChildItem(wxID_SMAXIMUM,  NULL); sm->MaximumSM(mi->IsChecked());
      mi = SDShaderOptions->FindChildItem(wxID_SUPGRADE,  NULL); sm->UpgradeSM(mi->IsChecked());
      mi = SDShaderOptions->FindChildItem(wxID_SRUNTIME,  NULL); sm->RuntimeSources(mi->IsChecked());

      GetShaderRecordStatus(o);
    }

//  event.Skip();
  }

  virtual void DoEffectOptions(wxCommandEvent& event) {
    EffectRecord *o = FindEffectRecord(NULL, SDComboEffect->GetStringSelection());

    /* ------------------------------------------------ */
    UpdateEffectOptions(o);

    if (em) {
      wxMenuItem *mi;

      mi = SDEffectOptions->FindChildItem(wxID_ECOMPILE,  NULL); em->CompileSources(mi->IsChecked());
//    mi = SDEffectOptions->FindChildItem(wxID_ESAVEBIN,  NULL); em->SaveEffectBinary(mi->IsChecked());
      mi = SDEffectOptions->FindChildItem(wxID_EOPTIMIZE, NULL); em->Optimize(mi->IsChecked());
      mi = SDEffectOptions->FindChildItem(wxID_ELEGACY,   NULL); em->UseLegacyCompiler(mi->IsChecked());

      GetEffectRecordStatus(o);
    }

    event.Skip();
  }

  virtual void DoProfileOptions(wxCommandEvent& event) {

    /* ------------------------------------------------ */
//  UpdateProfileOptions();

#if defined(OGBE_PROFILE) || 1
    if (1) {
      wxMenuItem *mi;

      mi = SDProfileOptions->FindChildItem(wxID_PROFILE, NULL); frame_prf = mi->IsChecked();
      mi = SDProfileOptions->FindChildItem(wxID_KILLTEX, NULL); frame_ntx = mi->IsChecked();

#if defined(OGBE_TESSELATION) || 1
      mi = SDProfileOptions->FindChildItem(wxID_WIREFRAME, NULL); frame_wre = mi->IsChecked();
      mi = SDProfileOptions->FindChildItem(wxID_TESSELATION, NULL); frame_tes = mi->IsChecked();

      if (frame_wre)
	lastOBGEDirect3DDevice9->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
      else
	lastOBGEDirect3DDevice9->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
#endif
    }
#endif

//  event.Skip();
  }

  virtual void DoToolPMtoQDMy(wxCommandEvent& event) {
    DoToolPMtoQDM(true);
//  event.Skip();
  }

  virtual void DoToolPMtoQDMn(wxCommandEvent& event) {
    DoToolPMtoQDM(false);
//  event.Skip();
  }

  void DoToolPMtoQDM(bool LODed) {
    wxMenuItem *
    mi = SDToolsSettings->FindChildItem(wxID_MIPGAMMA, NULL);
    bool gamma = mi->IsChecked();

    /* ------------------------------------------------ */
    wxFileDialog dlg1(
      this,
      _T("Select base-texture (RGB+Height)"),
      wxT(em->EffectDirectory()),
      wxEmptyString,
      _T(
	"Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
	"PNG images (*.png)|*.png|"
	"JPEG images (*.jpg)|*.jpg|"
	"Windows images (*.bmp)|*.bmp|"
	"DirextX images (*.dds)|*.dds"
      ),
      wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if (dlg1.ShowModal() != wxID_OK)
      return;

    // get filename
    wxString m_base_filename = dlg1.GetFilename();
    wxString m_base_filepath = dlg1.GetPath();

    wxFileDialog dlg2(
      this,
      _T("Select normal-texture (XYZ+Diffuse)"),
      wxT(em->EffectDirectory()),
      wxEmptyString,
      _T(
	"Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
	"PNG images (*.png)|*.png|"
	"JPEG images (*.jpg)|*.jpg|"
	"Windows images (*.bmp)|*.bmp|"
	"DirextX images (*.dds)|*.dds"
      ),
      wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if (dlg2.ShowModal() != wxID_OK)
      return;

    // get filename
    wxString m_norm_filename = dlg2.GetFilename();
    wxString m_norm_filepath = dlg2.GetPath();

    LPDIRECT3DTEXTURE9 base = NULL, norm = NULL;

    D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_base_filepath, &base);
    D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_norm_filepath, &norm);

    if (base && norm) {
      ProgressSubject = m_base_filename + " " + m_norm_filename;

//    if (TextureCompressPM(&base, &norm))
      if (TextureCompressQDM(&base, &norm, gamma, LODed)) {
	wxFileDialog dlg1(
	  this,
	  _T("Select base-texture (RGB+Height)"),
	  wxT(em->EffectDirectory()),
	  wxEmptyString,
	  _T(
	  "DirextX images (*.dds)|*.dds"
	  ),
	  wxFD_SAVE | wxFD_OVERWRITE_PROMPT
	);

	if (dlg1.ShowModal() != wxID_OK)
	  return;

	// get filename
	wxString m_base_filename = dlg1.GetFilename();
	wxString m_base_filepath = dlg1.GetPath();

	wxFileDialog dlg2(
	  this,
	  _T("Select normal-texture (XYZ+Diffuse)"),
	  wxT(em->EffectDirectory()),
	  wxEmptyString,
	  _T(
	  "DirextX images (*.dds)|*.dds"
	  ),
	  wxFD_SAVE | wxFD_OVERWRITE_PROMPT
	);

	if (dlg2.ShowModal() != wxID_OK)
	  return;

	// get filename
	wxString m_norm_filename = dlg2.GetFilename();
	wxString m_norm_filepath = dlg2.GetPath();

	HRESULT resb = D3DXSaveTextureToFile(m_base_filepath, D3DXIFF_DDS, base, NULL);
	HRESULT resn = D3DXSaveTextureToFile(m_norm_filepath, D3DXIFF_DDS, norm, NULL);

	base->Release();
	norm->Release();
      }
    }

    SDStatusBar->SetStatusText(wxT("Ready"), 0);
    ProgressSubject = "...";
//  event.Skip();
  }

  virtual void DoToolRemipRGBH(wxCommandEvent& event) {
    wxMenuItem *
    mi = SDToolsSettings->FindChildItem(wxID_MIPGAMMA, NULL);
    bool gamma = mi->IsChecked();
    mi = SDToolsSettings->FindChildItem(wxID_BATCH, NULL);
    bool batch = mi->IsChecked();

    /* ------------------------------------------------ */
    if (!batch) {
      wxFileDialog dlg1(
	this,
	_T("Select base-texture (RGB+Height)"),
	wxT(em->EffectDirectory()),
	wxEmptyString,
	_T(
	"Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
	"PNG images (*.png)|*.png|"
	"JPEG images (*.jpg)|*.jpg|"
	"Windows images (*.bmp)|*.bmp|"
	"DirextX images (*.dds)|*.dds"
	),
	wxFD_OPEN | wxFD_FILE_MUST_EXIST
      );

      if (dlg1.ShowModal() != wxID_OK)
	return;

      // get filename
      wxString m_base_filename = dlg1.GetFilename();
      wxString m_base_filepath = dlg1.GetPath();

      LPDIRECT3DTEXTURE9 base = NULL;

      D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_base_filepath, &base);

      if (base) {
	ProgressSubject = m_base_filename;

	if (TextureCompressRGBH(&base, gamma)) {
	  wxFileDialog dlg1(
	    this,
	    _T("Select base-texture (RGB+Height)"),
	    wxT(em->EffectDirectory()),
	    wxEmptyString,
	    _T(
	    "DirextX images (*.dds)|*.dds"
	    ),
	    wxFD_SAVE | wxFD_OVERWRITE_PROMPT
	  );

	  if (dlg1.ShowModal() != wxID_OK)
	    return;

	  // get filename
	  wxString m_base_filename = dlg1.GetFilename();
	  wxString m_base_filepath = dlg1.GetPath();

	  HRESULT res = D3DXSaveTextureToFile(m_base_filepath, D3DXIFF_DDS, base, NULL);

	  base->Release();
	}
      }
    }
    else {
      wxFileDialog dlg2(
	this,
	_T("Select base-textures (RGB+Height)"),
	wxT(em->EffectDirectory()),
	wxEmptyString,
	_T(
	"Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
	"PNG images (*.png)|*.png|"
	"JPEG images (*.jpg)|*.jpg|"
	"Windows images (*.bmp)|*.bmp|"
	"DirextX images (*.dds)|*.dds"
	),
	wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE
      );

      if (dlg2.ShowModal() != wxID_OK)
	return;

      wxDirDialog dlg1(
	this,
	_T("Select base-textures (RGB+Height) destination"),
	wxT(em->EffectDirectory()),
	wxDD_DIR_MUST_EXIST
      );

      if (dlg1.ShowModal() != wxID_OK)
	return;

      // get filenames
      wxArrayString m_base_filenames; dlg2.GetFilenames(m_base_filenames);
      wxArrayString m_base_filepaths; dlg2.GetPaths(m_base_filepaths);
      wxString m_base_filedestn = dlg1.GetPath();

      for (int f = 0; f < m_base_filepaths.GetCount(); f++) {
	wxString m_base_filename = m_base_filenames[f];
	wxString m_base_filepath = m_base_filepaths[f];

	LPDIRECT3DTEXTURE9 base = NULL;
	D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_base_filepath, &base);

	if (base) {
	  ProgressSubject = m_base_filename;

	  if (TextureCompressRGBH(&base, gamma)) {
	    wxFileName n_base_filename(m_base_filename);
	    wxFileName n_base_filepath(m_base_filepath);
	    wxFileName n_base_filedest(m_base_filedestn, m_base_filename);

	    // get filename
	    n_base_filename.ClearExt(); n_base_filename.SetExt("dds");
	    n_base_filepath.ClearExt(); n_base_filepath.SetExt("dds");
	    n_base_filedest.ClearExt(); n_base_filedest.SetExt("dds");

	    wxString path = n_base_filedest.GetFullPath();
	    HRESULT res = D3DXSaveTextureToFile(path, D3DXIFF_DDS, base, NULL);
	    base->Release();
	  }
	}
      }
    }

    SDStatusBar->SetStatusText(wxT("Ready"), 0);
    ProgressSubject = "...";
//  event.Skip();
  }

  virtual void DoToolRemipRGBA(wxCommandEvent& event) {
    wxMenuItem *
    mi = SDToolsSettings->FindChildItem(wxID_MIPGAMMA, NULL);
    bool gamma = mi->IsChecked();
    mi = SDToolsSettings->FindChildItem(wxID_BATCH, NULL);
    bool batch = mi->IsChecked();

    /* ------------------------------------------------ */
    if (!batch) {
      wxFileDialog dlg1(
	this,
	_T("Select base-texture (RGB+Alpha)"),
	wxT(em->EffectDirectory()),
	wxEmptyString,
	_T(
	"Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
	"PNG images (*.png)|*.png|"
	"JPEG images (*.jpg)|*.jpg|"
	"Windows images (*.bmp)|*.bmp|"
	"DirextX images (*.dds)|*.dds"
	),
	wxFD_OPEN | wxFD_FILE_MUST_EXIST
      );

      if (dlg1.ShowModal() != wxID_OK)
	return;

      // get filename
      wxString m_base_filename = dlg1.GetFilename();
      wxString m_base_filepath = dlg1.GetPath();

      LPDIRECT3DTEXTURE9 base = NULL;

      D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_base_filepath, &base);

      if (base) {
	ProgressSubject = m_base_filename;

	if (TextureCompressRGBA(&base, gamma)) {
	  wxFileDialog dlg1(
	    this,
	    _T("Select base-texture (RGB+Alpha)"),
	    wxT(em->EffectDirectory()),
	    wxEmptyString,
	    _T(
	    "DirextX images (*.dds)|*.dds"
	    ),
	    wxFD_SAVE | wxFD_OVERWRITE_PROMPT
	  );

	  if (dlg1.ShowModal() != wxID_OK)
	    return;

	  // get filename
	  wxString m_base_filename = dlg1.GetFilename();
	  wxString m_base_filepath = dlg1.GetPath();

	  HRESULT res = D3DXSaveTextureToFile(m_base_filepath, D3DXIFF_DDS, base, NULL);

	  base->Release();
	}
      }
    }
    else {
      wxFileDialog dlg2(
	this,
	_T("Select base-textures (RGB+Alpha)"),
	wxT(em->EffectDirectory()),
	wxEmptyString,
	_T(
	"Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
	"PNG images (*.png)|*.png|"
	"JPEG images (*.jpg)|*.jpg|"
	"Windows images (*.bmp)|*.bmp|"
	"DirextX images (*.dds)|*.dds"
	),
	wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE
      );

      if (dlg2.ShowModal() != wxID_OK)
	return;

      wxDirDialog dlg1(
	this,
	_T("Select base-textures (RGB+Alpha) destination"),
	wxT(em->EffectDirectory()),
	wxDD_DIR_MUST_EXIST
      );

      if (dlg1.ShowModal() != wxID_OK)
	return;

      // get filenames
      wxArrayString m_base_filenames; dlg2.GetFilenames(m_base_filenames);
      wxArrayString m_base_filepaths; dlg2.GetPaths(m_base_filepaths);
      wxString m_base_filedestn = dlg1.GetPath();

      for (int f = 0; f < m_base_filepaths.GetCount(); f++) {
	wxString m_base_filename = m_base_filenames[f];
	wxString m_base_filepath = m_base_filepaths[f];

	LPDIRECT3DTEXTURE9 base = NULL;
	D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_base_filepath, &base);

	if (base) {
	  ProgressSubject = m_base_filename;

	  if (TextureCompressRGBA(&base, gamma)) {
	    wxFileName n_base_filename(m_base_filename);
	    wxFileName n_base_filepath(m_base_filepath);
	    wxFileName n_base_filedest(m_base_filedestn, m_base_filename);

	    // get filename
	    n_base_filename.ClearExt(); n_base_filename.SetExt("dds");
	    n_base_filepath.ClearExt(); n_base_filepath.SetExt("dds");
	    n_base_filedest.ClearExt(); n_base_filedest.SetExt("dds");

	    wxString path = n_base_filedest.GetFullPath();
	    HRESULT res = D3DXSaveTextureToFile(path, D3DXIFF_DDS, base, NULL);
	    base->Release();
	  }
	}
      }
    }

    SDStatusBar->SetStatusText(wxT("Ready"), 0);
    ProgressSubject = "...";
//  event.Skip();
  }

  virtual void DoToolRemipRGB(wxCommandEvent& event) {
    wxMenuItem *
    mi = SDToolsSettings->FindChildItem(wxID_MIPGAMMA, NULL);
    bool gamma = mi->IsChecked();
    mi = SDToolsSettings->FindChildItem(wxID_BATCH, NULL);
    bool batch = mi->IsChecked();

    /* ------------------------------------------------ */
    if (!batch) {
      wxFileDialog dlg1(
	this,
	_T("Select base-texture (RGB)"),
	wxT(em->EffectDirectory()),
	wxEmptyString,
	_T(
	"Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
	"PNG images (*.png)|*.png|"
	"JPEG images (*.jpg)|*.jpg|"
	"Windows images (*.bmp)|*.bmp|"
	"DirextX images (*.dds)|*.dds"
	),
	wxFD_OPEN | wxFD_FILE_MUST_EXIST
      );

      if (dlg1.ShowModal() != wxID_OK)
	return;

      // get filename
      wxString m_base_filename = dlg1.GetFilename();
      wxString m_base_filepath = dlg1.GetPath();

      LPDIRECT3DTEXTURE9 base = NULL;

      D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_base_filepath, &base);

      if (base) {
	ProgressSubject = m_base_filename;

	if (TextureCompressRGB(&base, gamma)) {
	  wxFileDialog dlg1(
	    this,
	    _T("Select base-texture (RGB)"),
	    wxT(em->EffectDirectory()),
	    wxEmptyString,
	    _T(
	    "DirextX images (*.dds)|*.dds"
	    ),
	    wxFD_SAVE | wxFD_OVERWRITE_PROMPT
	  );

	  if (dlg1.ShowModal() != wxID_OK)
	    return;

	  // get filename
	  wxString m_base_filename = dlg1.GetFilename();
	  wxString m_base_filepath = dlg1.GetPath();

	  HRESULT res = D3DXSaveTextureToFile(m_base_filepath, D3DXIFF_DDS, base, NULL);

	  base->Release();
	}
      }
    }
    else {
      wxFileDialog dlg2(
	this,
	_T("Select base-textures (RGB)"),
	wxT(em->EffectDirectory()),
	wxEmptyString,
	_T(
	"Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
	"PNG images (*.png)|*.png|"
	"JPEG images (*.jpg)|*.jpg|"
	"Windows images (*.bmp)|*.bmp|"
	"DirextX images (*.dds)|*.dds"
	),
	wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE
      );

      if (dlg2.ShowModal() != wxID_OK)
	return;

      wxDirDialog dlg1(
	this,
	_T("Select base-textures (RGB) destination"),
	wxT(em->EffectDirectory()),
	wxDD_DIR_MUST_EXIST
      );

      if (dlg1.ShowModal() != wxID_OK)
	return;

      // get filenames
      wxArrayString m_base_filenames; dlg2.GetFilenames(m_base_filenames);
      wxArrayString m_base_filepaths; dlg2.GetPaths(m_base_filepaths);
      wxString m_base_filedestn = dlg1.GetPath();

      for (int f = 0; f < m_base_filepaths.GetCount(); f++) {
	wxString m_base_filename = m_base_filenames[f];
	wxString m_base_filepath = m_base_filepaths[f];

	LPDIRECT3DTEXTURE9 base = NULL;
	D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_base_filepath, &base);

	if (base) {
	  ProgressSubject = m_base_filename;

	  if (TextureCompressRGB(&base, gamma)) {
	    wxFileName n_base_filename(m_base_filename);
	    wxFileName n_base_filepath(m_base_filepath);
	    wxFileName n_base_filedest(m_base_filedestn, m_base_filename);

	    // get filename
	    n_base_filename.ClearExt(); n_base_filename.SetExt("dds");
	    n_base_filepath.ClearExt(); n_base_filepath.SetExt("dds");
	    n_base_filedest.ClearExt(); n_base_filedest.SetExt("dds");

	    wxString path = n_base_filedest.GetFullPath();
	    HRESULT res = D3DXSaveTextureToFile(path, D3DXIFF_DDS, base, NULL);
	    base->Release();
	  }
	}
      }
    }

    SDStatusBar->SetStatusText(wxT("Ready"), 0);
    ProgressSubject = "...";
//  event.Skip();
  }

  virtual void DoToolRemipLA(wxCommandEvent& event) {
    wxMenuItem *
    mi = SDToolsSettings->FindChildItem(wxID_BATCH, NULL);
    bool batch = mi->IsChecked();

    /* ------------------------------------------------ */
    wxFileDialog dlg1(
      this,
      _T("Select base-texture (--L+alpha)"),
      wxT(em->EffectDirectory()),
      wxEmptyString,
      _T(
      "Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
      "PNG images (*.png)|*.png|"
      "JPEG images (*.jpg)|*.jpg|"
      "Windows images (*.bmp)|*.bmp|"
      "DirextX images (*.dds)|*.dds"
      ),
      wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if (dlg1.ShowModal() != wxID_OK)
      return;

    // get filename
    wxString m_base_filename = dlg1.GetFilename();
    wxString m_base_filepath = dlg1.GetPath();

    LPDIRECT3DTEXTURE9 base = NULL;

    D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_base_filepath, &base);

    if (base) {
      ProgressSubject = m_base_filename;

      if (TextureCompressLA(&base)) {
	wxFileDialog dlg1(
	  this,
	  _T("Select base-texture (L+alpha)"),
	  wxT(em->EffectDirectory()),
	  wxEmptyString,
	  _T(
	  "DirextX images (*.dds)|*.dds"
	  ),
	  wxFD_SAVE | wxFD_OVERWRITE_PROMPT
	);

	if (dlg1.ShowModal() != wxID_OK)
	  return;

	// get filename
	wxString m_base_filename = dlg1.GetFilename();
	wxString m_base_filepath = dlg1.GetPath();

	HRESULT res = D3DXSaveTextureToFile(m_base_filepath, D3DXIFF_DDS, base, NULL);

	base->Release();
      }
    }

    SDStatusBar->SetStatusText(wxT("Ready"), 0);
    ProgressSubject = "...";
//  event.Skip();
  }

  virtual void DoToolRemipA(wxCommandEvent& event) {
    wxMenuItem *
    mi = SDToolsSettings->FindChildItem(wxID_BATCH, NULL);
    bool batch = mi->IsChecked();

    /* ------------------------------------------------ */
    wxFileDialog dlg1(
      this,
      _T("Select alpha-texture (RGB+alpha)"),
      wxT(em->EffectDirectory()),
      wxEmptyString,
      _T(
      "Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
      "PNG images (*.png)|*.png|"
      "JPEG images (*.jpg)|*.jpg|"
      "Windows images (*.bmp)|*.bmp|"
      "DirextX images (*.dds)|*.dds"
      ),
      wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if (dlg1.ShowModal() != wxID_OK)
      return;

    // get filename
    wxString m_base_filename = dlg1.GetFilename();
    wxString m_base_filepath = dlg1.GetPath();

    LPDIRECT3DTEXTURE9 base = NULL;

    D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_base_filepath, &base);

    if (base) {
      ProgressSubject = m_base_filename;

      if (TextureCompressA(&base)) {
	wxFileDialog dlg1(
	  this,
	  _T("Select alpha-texture (alpha)"),
	  wxT(em->EffectDirectory()),
	  wxEmptyString,
	  _T(
	  "DirextX images (*.dds)|*.dds"
	  ),
	  wxFD_SAVE | wxFD_OVERWRITE_PROMPT
	);

	if (dlg1.ShowModal() != wxID_OK)
	  return;

	// get filename
	wxString m_base_filename = dlg1.GetFilename();
	wxString m_base_filepath = dlg1.GetPath();

	HRESULT res = D3DXSaveTextureToFile(m_base_filepath, D3DXIFF_DDS, base, NULL);

	base->Release();
      }
    }

    SDStatusBar->SetStatusText(wxT("Ready"), 0);
    ProgressSubject = "...";
//  event.Skip();
  }

  virtual void DoToolRemipXYZD(wxCommandEvent& event) {
    wxMenuItem *
    mi = SDToolsSettings->FindChildItem(wxID_BATCH, NULL);
    bool batch = mi->IsChecked();

    /* ------------------------------------------------ */
    if (!batch) {
      wxFileDialog dlg2(
        this,
        _T("Select normal-texture (XYZ+Specular)"),
        wxT(em->EffectDirectory()),
        wxEmptyString,
        _T(
	  "Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
	  "PNG images (*.png)|*.png|"
	  "JPEG images (*.jpg)|*.jpg|"
	  "Windows images (*.bmp)|*.bmp|"
	  "DirextX images (*.dds)|*.dds"
        ),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST
      );

      if (dlg2.ShowModal() != wxID_OK)
        return;

      // get filename
      wxString m_norm_filename = dlg2.GetFilename();
      wxString m_norm_filepath = dlg2.GetPath();

      LPDIRECT3DTEXTURE9 norm = NULL;

      D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_norm_filepath, &norm);

      if (norm) {
        ProgressSubject = m_norm_filename;

        if (TextureCompressXYZD(&norm)) {
	  wxFileDialog dlg2(
	    this,
	    _T("Select normal-texture (XYZ+Specular)"),
	    wxT(em->EffectDirectory()),
	    wxEmptyString,
	    _T(
	    "DirextX images (*.dds)|*.dds"
	    ),
	    wxFD_SAVE | wxFD_OVERWRITE_PROMPT
	  );

	  if (dlg2.ShowModal() != wxID_OK)
	    return;

	  // get filename
	  wxString m_norm_filename = dlg2.GetFilename();
	  wxString m_norm_filepath = dlg2.GetPath();

	  HRESULT res = D3DXSaveTextureToFile(m_norm_filepath, D3DXIFF_DDS, norm, NULL);

	  norm->Release();
        }
      }
    }
    else {
      wxFileDialog dlg2(
        this,
        _T("Select normal-textures (XYZ+Specular)"),
        wxT(em->EffectDirectory()),
        wxEmptyString,
        _T(
	  "Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
	  "PNG images (*.png)|*.png|"
	  "JPEG images (*.jpg)|*.jpg|"
	  "Windows images (*.bmp)|*.bmp|"
	  "DirextX images (*.dds)|*.dds"
        ),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE
      );

      if (dlg2.ShowModal() != wxID_OK)
        return;

      wxDirDialog dlg1(
        this,
        _T("Select normal-texture (XYZ+Specular) destination"),
        wxT(em->EffectDirectory()),
        wxDD_DIR_MUST_EXIST
      );

      if (dlg1.ShowModal() != wxID_OK)
        return;

      // get filenames
      wxArrayString m_norm_filenames; dlg2.GetFilenames(m_norm_filenames);
      wxArrayString m_norm_filepaths; dlg2.GetPaths(m_norm_filepaths);
      wxString m_norm_filedestn = dlg1.GetPath();

      for (int f = 0; f < m_norm_filepaths.GetCount(); f++) {
        wxString m_norm_filename = m_norm_filenames[f];
        wxString m_norm_filepath = m_norm_filepaths[f];

        LPDIRECT3DTEXTURE9 norm = NULL;
        D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_norm_filepath, &norm);

        if (norm) {
	  ProgressSubject = m_norm_filename;

          if (TextureCompressXYZD(&norm)) {
            wxFileName n_norm_filename(m_norm_filename);
            wxFileName n_norm_filepath(m_norm_filepath);
            wxFileName n_norm_filedest(m_norm_filedestn, m_norm_filename);

	    // get filename
	    n_norm_filename.ClearExt(); n_norm_filename.SetExt("dds");
	    n_norm_filepath.ClearExt(); n_norm_filepath.SetExt("dds");
	    n_norm_filedest.ClearExt(); n_norm_filedest.SetExt("dds");

	    wxString path = n_norm_filedest.GetFullPath();
	    HRESULT res = D3DXSaveTextureToFile(path, D3DXIFF_DDS, norm, NULL);
	    norm->Release();
          }
        }
      }
    }

    SDStatusBar->SetStatusText(wxT("Ready"), 0);
    ProgressSubject = "...";
//  event.Skip();
  }

  virtual void DoToolRemipXYZ(wxCommandEvent& event) {
    wxMenuItem *
    mi = SDToolsSettings->FindChildItem(wxID_BATCH, NULL);
    bool batch = mi->IsChecked();

    /* ------------------------------------------------ */
    if (!batch) {
      wxFileDialog dlg2(
        this,
        _T("Select normal-texture (XYZ)"),
        wxT(em->EffectDirectory()),
        wxEmptyString,
        _T(
	  "Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
	  "PNG images (*.png)|*.png|"
	  "JPEG images (*.jpg)|*.jpg|"
	  "Windows images (*.bmp)|*.bmp|"
	  "DirextX images (*.dds)|*.dds"
        ),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST
      );

      if (dlg2.ShowModal() != wxID_OK)
        return;

      // get filename
      wxString m_norm_filename = dlg2.GetFilename();
      wxString m_norm_filepath = dlg2.GetPath();

      LPDIRECT3DTEXTURE9 norm = NULL;
      D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_norm_filepath, &norm);

      if (norm) {
	ProgressSubject = m_norm_filename;

        if (TextureCompressXYZ(&norm)) {
	  wxFileDialog dlg2(
	    this,
	    _T("Select normal-texture (XYZ)"),
	    wxT(em->EffectDirectory()),
	    wxEmptyString,
	    _T(
	      "DirextX images (*.dds)|*.dds"
	    ),
	    wxFD_SAVE | wxFD_OVERWRITE_PROMPT
	  );

	  if (dlg2.ShowModal() != wxID_OK)
	    return;

	  // get filename
	  wxString m_norm_filename = dlg2.GetFilename();
	  wxString m_norm_filepath = dlg2.GetPath();

	  HRESULT res = D3DXSaveTextureToFile(m_norm_filepath, D3DXIFF_DDS, norm, NULL);
	  norm->Release();
        }
      }
    }
    else {
      wxFileDialog dlg2(
        this,
        _T("Select normal-textures (XYZ)"),
        wxT(em->EffectDirectory()),
        wxEmptyString,
        _T(
	  "Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
	  "PNG images (*.png)|*.png|"
	  "JPEG images (*.jpg)|*.jpg|"
	  "Windows images (*.bmp)|*.bmp|"
	  "DirextX images (*.dds)|*.dds"
        ),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE
      );

      if (dlg2.ShowModal() != wxID_OK)
        return;

      wxDirDialog dlg1(
        this,
        _T("Select normal-texture (XYZ) destination"),
        wxT(em->EffectDirectory()),
        wxDD_DIR_MUST_EXIST
      );

      if (dlg1.ShowModal() != wxID_OK)
        return;

      // get filenames
      wxArrayString m_norm_filenames; dlg2.GetFilenames(m_norm_filenames);
      wxArrayString m_norm_filepaths; dlg2.GetPaths(m_norm_filepaths);
      wxString m_norm_filedestn = dlg1.GetPath();

      for (int f = 0; f < m_norm_filepaths.GetCount(); f++) {
        wxString m_norm_filename = m_norm_filenames[f];
        wxString m_norm_filepath = m_norm_filepaths[f];

        LPDIRECT3DTEXTURE9 norm = NULL;
        D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_norm_filepath, &norm);

        if (norm) {
	  ProgressSubject = m_norm_filename;

          if (TextureCompressXYZ(&norm)) {
            wxFileName n_norm_filename(m_norm_filename);
            wxFileName n_norm_filepath(m_norm_filepath);
            wxFileName n_norm_filedest(m_norm_filedestn, m_norm_filename);

	    // get filename
	    n_norm_filename.ClearExt(); n_norm_filename.SetExt("dds");
	    n_norm_filepath.ClearExt(); n_norm_filepath.SetExt("dds");
	    n_norm_filedest.ClearExt(); n_norm_filedest.SetExt("dds");

	    wxString path = n_norm_filedest.GetFullPath();
	    HRESULT res = D3DXSaveTextureToFile(path, D3DXIFF_DDS, norm, NULL);
	    norm->Release();
          }
        }
      }
    }

    SDStatusBar->SetStatusText(wxT("Ready"), 0);
    ProgressSubject = "...";
//  event.Skip();
  }

  virtual void DoToolRemipXY_Z(wxCommandEvent& event) {
    wxMenuItem *
    mi = SDToolsSettings->FindChildItem(wxID_BATCH, NULL);
    bool batch = mi->IsChecked();

    /* ------------------------------------------------ */
    wxFileDialog dlg2(
      this,
      _T("Select normal-texture (XYZ+Specular)"),
      wxT(em->EffectDirectory()),
      wxEmptyString,
      _T(
      "Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
      "PNG images (*.png)|*.png|"
      "JPEG images (*.jpg)|*.jpg|"
      "Windows images (*.bmp)|*.bmp|"
      "DirextX images (*.dds)|*.dds"
      ),
      wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if (dlg2.ShowModal() != wxID_OK)
      return;

    // get filename
    wxString m_norm_filename = dlg2.GetFilename();
    wxString m_norm_filepath = dlg2.GetPath();

    LPDIRECT3DTEXTURE9 norm = NULL, z = NULL;

    D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_norm_filepath, &norm);

    if (norm) {
      ProgressSubject = m_norm_filename;

      if (TextureCompressXY_Z(&norm, &z)) {
	wxFileDialog dlg1(
	  this,
	  _T("Select normal-texture (XY)"),
	  wxT(em->EffectDirectory()),
	  wxEmptyString,
	  _T(
	  "DirextX images (*.dds)|*.dds"
	  ),
	  wxFD_SAVE | wxFD_OVERWRITE_PROMPT
	);

	if (dlg1.ShowModal() != wxID_OK)
	  return;

	// get filename
	wxString m_norm1_filename = dlg1.GetFilename();
	wxString m_norm1_filepath = dlg1.GetPath();

	wxFileDialog dlg2(
	  this,
	  _T("Select normal-texture (Z)"),
	  wxT(em->EffectDirectory()),
	  wxEmptyString,
	  _T(
	  "DirextX images (*.dds)|*.dds"
	  ),
	  wxFD_SAVE | wxFD_OVERWRITE_PROMPT
	);

	if (dlg2.ShowModal() != wxID_OK)
	  return;

	// get filename
	wxString m_norm2_filename = dlg2.GetFilename();
	wxString m_norm2_filepath = dlg2.GetPath();

	HRESULT res1 = D3DXSaveTextureToFile(m_norm1_filepath, D3DXIFF_DDS, norm, NULL);
	HRESULT res2 = D3DXSaveTextureToFile(m_norm2_filepath, D3DXIFF_DDS,   z , NULL);

	norm->Release();
	z->Release();
      }
    }

    SDStatusBar->SetStatusText(wxT("Ready"), 0);
    ProgressSubject = "...";
//  event.Skip();
  }

  virtual void DoToolRemipXY(wxCommandEvent& event) {
    wxMenuItem *
    mi = SDToolsSettings->FindChildItem(wxID_BATCH, NULL);
    bool batch = mi->IsChecked();

    /* ------------------------------------------------ */
    wxFileDialog dlg2(
      this,
      _T("Select normal-texture (XYZ)"),
      wxT(em->EffectDirectory()),
      wxEmptyString,
      _T(
      "Image files (*.png;*.jpg;*.bmp;*.dds)|*.png;*.jpg;*.bmp;*.dds|"
      "PNG images (*.png)|*.png|"
      "JPEG images (*.jpg)|*.jpg|"
      "Windows images (*.bmp)|*.bmp|"
      "DirextX images (*.dds)|*.dds"
      ),
      wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if (dlg2.ShowModal() != wxID_OK)
      return;

    // get filename
    wxString m_norm_filename = dlg2.GetFilename();
    wxString m_norm_filepath = dlg2.GetPath();

    LPDIRECT3DTEXTURE9 norm = NULL;

    D3DXCreateTextureFromFile(lastOBGEDirect3DDevice9, m_norm_filepath, &norm);

    if (norm) {
      ProgressSubject = m_norm_filename;

      if (TextureCompressXY(&norm)) {
	wxFileDialog dlg2(
	  this,
	  _T("Select normal-texture (XYZ)"),
	  wxT(em->EffectDirectory()),
	  wxEmptyString,
	  _T(
	  "DirextX images (*.dds)|*.dds"
	  ),
	  wxFD_SAVE | wxFD_OVERWRITE_PROMPT
	);

	if (dlg2.ShowModal() != wxID_OK)
	  return;

	// get filename
	wxString m_norm_filename = dlg2.GetFilename();
	wxString m_norm_filepath = dlg2.GetPath();

	HRESULT res = D3DXSaveTextureToFile(m_norm_filepath, D3DXIFF_DDS, norm, NULL);

	norm->Release();
      }
    }

    SDStatusBar->SetStatusText(wxT("Ready"), 0);
    ProgressSubject = "...";
//  event.Skip();
  }

  /* --------------------------------------------------------------
   */

  virtual void DoAF(wxCommandEvent& event) {
    TextureManager *em = TextureManager::GetSingleton();
    wxMenuItem *mi;

    mi = SDTweaksAF->FindChildItem(wxID_AF1, NULL);
    if (mi->IsChecked()) em->SetAnisotropy(1);
    mi = SDTweaksAF->FindChildItem(wxID_AF2, NULL);
    if (mi->IsChecked()) em->SetAnisotropy(2);
    mi = SDTweaksAF->FindChildItem(wxID_AF4, NULL);
    if (mi->IsChecked()) em->SetAnisotropy(4);
    mi = SDTweaksAF->FindChildItem(wxID_AF8, NULL);
    if (mi->IsChecked()) em->SetAnisotropy(8);
    mi = SDTweaksAF->FindChildItem(wxID_AF16, NULL);
    if (mi->IsChecked()) em->SetAnisotropy(16);
    mi = SDTweaksAF->FindChildItem(wxID_AF32, NULL);
    if (mi->IsChecked()) em->SetAnisotropy(32);

//  event.Skip();
  }

  /* --------------------------------------------------------------
   */

  virtual void DoLinear(wxCommandEvent& event) {
    TextureManager *em = TextureManager::GetSingleton();
    wxMenuItem *mi;

    mi = SDTweaks->FindChildItem(wxID_LINEAR, NULL);
    em->DoDeGamma(mi->IsChecked());
    em->DoReGamma(mi->IsChecked());
//  event.Skip();wxID_LINEAR
  }

  /* --------------------------------------------------------------
   */

  void SetProgress(int a, int amax, int b, int bmax) {
    char buf[1024];

    if (bmax)
      sprintf(buf, "Processing %s: %.3f%%, %.3f%%", ProgressSubject.GetData(), 100.0f * a / amax, 100.0f * b / bmax);
    else if (amax)
      sprintf(buf, "Processing %s: %.3f%%", ProgressSubject.GetData(), 100.0f * a / amax);
    else
      sprintf(buf, "Processing %s", ProgressSubject.GetData());

    SDStatusBar->SetStatusText(wxString(buf), 0);
  }

  /* --------------------------------------------------------------
   */

  virtual void DoResize(wxSizeEvent& event) {
    DoRefresh();
    event.Skip();
  }

  void DoRefresh() {
    /* refresh only if visible */
    if (SDViewSwitch->GetSelection() == SDVIEW_SCENES) {
      if (SDSurfaceSwitch->GetCurrentPage() == SDRendertarget)
	SDRendertarget->Refresh();
      else if (SDSurfaceSwitch->GetCurrentPage() == SDRendertargetGrabbed)
	SDRendertargetGrabbed->Refresh();
      else if (SDSurfaceSwitch->GetCurrentPage() == SDDepthStencil)
	SDDepthStencil->Refresh();
    }
  }

  virtual void DoClose(wxCloseEvent& event) {
  }
};

DebugWindow::DebugWindow() {
  assert(NULL);

  GUIs_ShaderDeveloper *
  sdev = new GUIs_ShaderDeveloper(NULL, wxID_ANY, "OBGE «Shader Developer»", wxPoint(-100, 0), wxSize(630, 704));
  sdev->Show();
  sdev->SetPosition(wxPoint(/*-70*/0, 0));
  this->sdev = (void *)sdev;

//assert(NULL);
}

DebugWindow::~DebugWindow() {
  GUIs_ShaderDeveloper *
  sdev = (GUIs_ShaderDeveloper *)this->sdev;
  sdev->Close();
  sdev->Destroy();
//delete sdev;
}

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

bool da = false;

// this is in any .cpp in library
class GUIs_App: public wxApp
{
public:
  bool OnInit()
  {
    da = true; return true;
  }
  int OnExit()
  {
    da = false; return wxApp::OnExit();
  }
};

static wxAppConsole *wxCreateApp()
{
  wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "my app");
  return new GUIs_App;
}

WXDLLIMPEXP_BASE void wxSetInstance(HINSTANCE hInst);

DebugWindow *dw = NULL;
DebugWindow *DebugWindow::Create() {
	if (!FullScreen.data && !dw) {
	  if (dw = new DebugWindow())
	    _MESSAGE("ShaderDeveloper did open sucessfully");
	  else
	    _MESSAGE("ShaderDeveloper didn't open sucessfully");
	}
	else
	  _MESSAGE("ShaderDeveloper can't start because of \"bFull Screen=1\"");

	return dw;
}

void DebugWindow::Destroy() {
	if (!FullScreen.data && dw) {
	  delete dw; dw = NULL; }
}

DebugWindow *DebugWindow::Expunge() {
	if (DWEnabled.data) {
	  if (!da) {
	    int argc = 0; char** argv = NULL;

	    HINSTANCE exe = GetModuleHandle(NULL);
	    HINSTANCE dll = GetModuleHandle("OBGEv2.dll");

	    wxSetInstance(dll);
	    wxApp::SetInitializerFunction(wxCreateApp);
	    wxInitialize(argc, argv);
	  }

	  return DebugWindow::Create();
	}
	else
	  _MESSAGE("ShaderDeveloper can't start because of \"bEnableDW=0\"");

	return NULL;
}

DebugWindow *DebugWindow::Get() {
	return dw;
}

void DebugWindow::Exit() {
	if (DWEnabled.data) {
	  Destroy();

	  if (da) {
	    wxUninitialize();
	  }
	}
}

void DebugWindow::SetProgress(int a, int amax, int b, int bmax) {
	GUIs_ShaderDeveloper *
	sdev = (GUIs_ShaderDeveloper *)this->sdev;

	sdev->SetProgress(a, amax, b, bmax);
}

#endif
#endif
#endif

#include "obse/Utilities.h"
#include "GlobalSettings.h"
#include <assert.h>

#include "D3D9.hpp"
#include "D3D9Device.hpp"
#include "GUIs_DebugWindow.hpp"
#define	OBGE_DEVLING
#ifdef	OBGE_DEVLING

#include "ShaderManager.h"
#include "Half.hpp"

static global<bool> DWEnabled(false, NULL, "General", "bEnabledDW");
static global<bool> FullScreen(0, "Oblivion.ini", "Display", "bFull Screen");

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
#include <wx/string.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#pragma comment(lib,"Comctl32")
#pragma comment(lib,"Rpcrt4")

#include "EffectManager.h"

#include "GUIs_ShaderDeveloper.h"
#include "GUIs_ShaderDeveloper.cpp"

#define wxBName	      wxString((*BShader)->Name)
#define wxBError      wxBitmap(wxT("#107"), wxBITMAP_TYPE_RESOURCE)
#define wxBWarning    wxBitmap(wxT("#108"), wxBITMAP_TYPE_RESOURCE)
#define wxBMissing    wxBitmap(wxT("#110"), wxBITMAP_TYPE_RESOURCE)
#define wxBUnhooked   wxBitmap(wxT("#109"), wxBITMAP_TYPE_RESOURCE)
#define wxBApplied    wxBitmap(wxT("#106"), wxBITMAP_TYPE_RESOURCE)

#if 0
D3DXPT_VOID,
D3DXPT_BOOL,
D3DXPT_INT,
D3DXPT_FLOAT,
D3DXPT_STRING,
D3DXPT_TEXTURE,
D3DXPT_TEXTURE1D,
D3DXPT_TEXTURE2D,
D3DXPT_TEXTURE3D,
D3DXPT_TEXTURECUBE,
D3DXPT_SAMPLER,
D3DXPT_SAMPLER1D,
D3DXPT_SAMPLER2D,
D3DXPT_SAMPLER3D,
D3DXPT_SAMPLERCUBE,
D3DXPT_PIXELSHADER,
D3DXPT_VERTEXSHADER,
D3DXPT_PIXELFRAGMENT,
D3DXPT_VERTEXFRAGMENT,
D3DXPT_UNSUPPORTED,
#endif

///////////////////////////////////////////////////////////////////////////////
/// Class wxShaderDeveloper
///////////////////////////////////////////////////////////////////////////////
class GUIs_ShaderDeveloper : public wxShaderDeveloper
{
public:
  GUIs_ShaderDeveloper(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 630,704 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL) :
      wxShaderDeveloper(parent, id, title, pos, size, style) {
  }

  ShaderManager *sm;
  wxImage rt, ds;
  wxMemoryDC crt, cds;
  ShaderRecord *currs;
  int currv;
  int fs[4];
  int pass;
  int scene;

  /* --------------------------------------------------------------
   */
  const char *DescribeTexture(IDirect3DBaseTexture9 *tex) {
    static char buf[256];
    buf[0] = '\0';

    if (tex) {
      /* search render targets */
      if (sm) {
        for (int p = 0; p < OBGEPASS_NUM; p++)
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
      }

      sprintf(buf, "0x%08x", tex);
    }
    else
      strcpy(buf, "cleared");

    return buf;
  }

  /* --------------------------------------------------------------
   */

  void SetConstantTable(wxGrid *gt, bool prefix, LPD3DXCONSTANTTABLE c, int len, int col, int offs) {
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

	    if (cnst.RegisterSet != f)
	      continue;

	    char c = ' ';
	    wxColour clr;
	    switch (f) {
	      case D3DXRS_BOOL: c = 'b'; clr = wxColour("GREY"); break;
	      case D3DXRS_INT4: c = 'i'; clr = wxColour("SIENNA"); break;
	      case D3DXRS_FLOAT4: c = 'c'; clr = wxColour("BLUE"); break;
	      case D3DXRS_SAMPLER: c = 's'; clr = wxColour("DARK GREEN"); break;
	    }

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

  void SetConstSetTable(wxGrid *gt, bool prefix, struct RuntimeShaderRecord::trace *t, int len, int col, int offs) {
    char buf[256];

    if (gt && t) {
      {
	int f = D3DXRS_BOOL; {
	  int pos = offs + (prefix ? 4 : 0);
	  for (int s = D3DXRS_BOOL; s < f; s++)
	    pos += fs[s];

	  wxColour clr = wxColour("GREY");
	  for (int x = 0; x < fs[f]; x++) {
	    assert(x < OBGESAMPLER_NUM);
	    if (*((int *)&t->values_b[x][0]) != -1) {
	      sprintf(buf, "%d, %d, %d, %d", t->values_b[x][0], t->values_b[x][1], t->values_b[x][2], t->values_b[x][3]);

	      gt->SetCellTextColour(pos + x, col, clr);
	      gt->SetCellValue(pos + x, col, wxString(buf));
	    }
	  }
	}

	f = D3DXRS_INT4; {
	  int pos = offs + (prefix ? 4 : 0);
	  for (int s = D3DXRS_BOOL; s < f; s++)
	    pos += fs[s];

	  wxColour clr = wxColour("SIENNA");
	  for (int x = 0; x < fs[f]; x++) {
	    assert(x < OBGESAMPLER_NUM);
	    if (*((int *)&t->values_i[x][0]) != -1) {
	      sprintf(buf, "%d, %d, %d, %d", t->values_i[x][0], t->values_i[x][1], t->values_i[x][2], t->values_i[x][3]);

	      gt->SetCellTextColour(pos + x, col, clr);
	      gt->SetCellValue(pos + x, col, wxString(buf));
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

	      wxString meaning = gt->GetCellValue(pos + x, col - 1);
	      if (strstr(meaning.GetData(), "color") ||
		  strstr(meaning.GetData(), "Color")) {
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

	      gt->SetCellValue(pos + x, col, wxString(buf));
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
	    }
	  }
	}
      }
    }
  }

  void SetSamplerTable(wxGrid *gt, bool prefix, struct RuntimeShaderRecord::trace *t, int len, int col, int offs) {
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

	    const char *dt = DescribeTexture(t->values_s[x]);
	    gt->SetCellValue(pos + d, col + 1, wxString(dt));

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

	      switch ((D3DSAMPLERSTATETYPE)y) {
	      	case D3DSAMP_ADDRESSU:
	      	case D3DSAMP_ADDRESSV:
		case D3DSAMP_ADDRESSW:
		  switch ((D3DTEXTUREADDRESS)t->states_s[x][y]) {
		    case D3DTADDRESS_WRAP:       strcpy(buf, "WRAP"); break;
		    case D3DTADDRESS_MIRROR:     strcpy(buf, "MIRROR"); break;
		    case D3DTADDRESS_CLAMP:      strcpy(buf, "CLAMP"); break;
		    case D3DTADDRESS_BORDER:     strcpy(buf, "BORDER"); break;
		    case D3DTADDRESS_MIRRORONCE: strcpy(buf, "MIRRORONCE"); break;
		    default: sprintf(buf, "%d", t->states_s[x][y]); break;
		  }
	      	  break;
	      	// hex
	      	case D3DSAMP_BORDERCOLOR:
	          sprintf(buf, "0x%08x", t->states_s[x][y]);
	      	  break;
	      	case D3DSAMP_MAGFILTER:
	      	case D3DSAMP_MINFILTER:
	      	case D3DSAMP_MIPFILTER:
		  switch ((D3DTEXTUREFILTERTYPE)t->states_s[x][y]) {
		    case D3DTEXF_NONE:          strcpy(buf, "NONE"); break;
		    case D3DTEXF_POINT:         strcpy(buf, "POINT"); break;
		    case D3DTEXF_LINEAR:        strcpy(buf, "LINEAR"); break;
		    case D3DTEXF_ANISOTROPIC:   strcpy(buf, "ANISOTROPIC"); break;
		    case D3DTEXF_PYRAMIDALQUAD: strcpy(buf, "PYRAMIDALQUAD"); break;
		    case D3DTEXF_GAUSSIANQUAD:  strcpy(buf, "GAUSSIANQUAD"); break;
		    default: sprintf(buf, "%d", t->states_s[x][y]); break;
		  }
	      	  break;
	      	// float
	      	case D3DSAMP_MIPMAPLODBIAS:	// deviation <>1.0
	      	case D3DSAMP_SRGBTEXTURE:	// gamma <>1.0
	          sprintf(buf, "%f", *((float *)&t->states_s[x][y]));
	      	  break;
	      	// int
	      	case D3DSAMP_MAXMIPLEVEL:
	      	case D3DSAMP_MAXANISOTROPY:
	      	case D3DSAMP_ELEMENTINDEX:
	          sprintf(buf, "%d", t->states_s[x][y]);
	      	  break;
	      	// long
	      	case D3DSAMP_DMAPOFFSET:
	          sprintf(buf, "%d", t->states_s[x][y]);
	      	  break;
	      	// unknown
	      	default:
	          sprintf(buf, "0x%08x", t->states_s[x][y]);
	      	  break;
	      }

	      gt->SetCellValue(pos + d, col + 1, wxString(buf));

	      d++;
	    }
	  }
	}
      }
    }
  }

  void SetRenderTable(wxGrid *gt, bool prefix, int o, struct ShaderManager::track *t, int len, int col, int offs) {
    char buf[256];

    if (gt && t) {
      int pos = offs + 0, d = 0;

      for (int v = 0; v < 2; v++)
      for (int x = 0; x < 4; x++) {
        gt->InsertRows(pos + d, 1);

        sprintf(buf, "%d", d);
        gt->SetRowLabelValue(pos + d, wxString(buf));

        wxColour clr = wxColour("BLUE");

	sprintf(buf, "%s[%d]", v ? "Projection" : "View", x);
        gt->SetCellTextColour(pos + d, col + 0, clr);
        gt->SetCellValue(pos + d, col + 0, wxString(buf));

	sprintf(buf, "%f, %f, %f, %f", t->transf[o][v].m[x][0], t->transf[o][v].m[x][1], t->transf[o][v].m[x][2], t->transf[o][v].m[x][3]);
        gt->SetCellTextColour(pos + d, col + 1, clr);
        gt->SetCellValue(pos + d, col + 1, wxString(buf));

        d++;
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

	  switch ((D3DRENDERSTATETYPE)y) {
	    case D3DRS_ZENABLE:
	      switch ((D3DZBUFFERTYPE)t->states[o][y]) {
		case D3DZB_FALSE: strcpy(buf, "FALSE"); break;
		case D3DZB_TRUE:  strcpy(buf, "TRUE"); break;
		case D3DZB_USEW:  strcpy(buf, "USEW"); break;
		default: sprintf(buf, "%d", t->states[o][y]); break;
	      }
	      break;
	    case D3DRS_FILLMODE:
	      switch ((D3DFILLMODE)t->states[o][y]) {
		case D3DFILL_POINT:     strcpy(buf, "POINT"); break;
		case D3DFILL_WIREFRAME: strcpy(buf, "WIREFRAME"); break;
		case D3DFILL_SOLID:     strcpy(buf, "SOLID"); break;
		default: sprintf(buf, "%d", t->states[o][y]); break;
	      }
	      break;
	    case D3DRS_SHADEMODE:
	      switch ((D3DSHADEMODE)t->states[o][y]) {
		case D3DSHADE_FLAT:    strcpy(buf, "FLAT"); break;
		case D3DSHADE_GOURAUD: strcpy(buf, "GOURAUD"); break;
		case D3DSHADE_PHONG:   strcpy(buf, "PHONG"); break;
		default: sprintf(buf, "%d", t->states[o][y]); break;
	      }
	      break;
	    case D3DRS_SRCBLEND:
	    case D3DRS_DESTBLEND:
	      switch ((D3DBLEND)t->states[o][y]) {
	        case D3DBLEND_ZERO           : strcpy(buf, "ZERO"); break;
	        case D3DBLEND_ONE            : strcpy(buf, "ONE"); break;
	        case D3DBLEND_SRCCOLOR       : strcpy(buf, "SRCCOLOR"); break;
	        case D3DBLEND_INVSRCCOLOR    : strcpy(buf, "INVSRCCOLOR"); break;
	        case D3DBLEND_SRCALPHA       : strcpy(buf, "SRCALPHA"); break;
	        case D3DBLEND_INVSRCALPHA    : strcpy(buf, "INVSRCALPHA"); break;
	        case D3DBLEND_DESTALPHA      : strcpy(buf, "DESTALPHA"); break;
	        case D3DBLEND_INVDESTALPHA   : strcpy(buf, "INVDESTALPHA"); break;
	        case D3DBLEND_DESTCOLOR      : strcpy(buf, "DESTCOLOR"); break;
	        case D3DBLEND_INVDESTCOLOR   : strcpy(buf, "INVDESTCOLOR"); break;
	        case D3DBLEND_SRCALPHASAT    : strcpy(buf, "SRCALPHASAT"); break;
	        case D3DBLEND_BOTHSRCALPHA   : strcpy(buf, "BOTHSRCALPHA"); break;
	        case D3DBLEND_BOTHINVSRCALPHA: strcpy(buf, "BOTHINVSRCALPHA"); break;
	        case D3DBLEND_BLENDFACTOR    : strcpy(buf, "BLENDFACTOR"); break;
	        case D3DBLEND_INVBLENDFACTOR : strcpy(buf, "INVBLENDFACTOR"); break;
	        case D3DBLEND_SRCCOLOR2      : strcpy(buf, "SRCCOLOR2"); break;
	        case D3DBLEND_INVSRCCOLOR2   : strcpy(buf, "INVSRCCOLOR2"); break;
		default: sprintf(buf, "%d", t->states[o][y]); break;
	      }
	      break;
	    case D3DRS_CULLMODE:
	      switch ((D3DCULL)t->states[o][y]) {
		case D3DCULL_NONE: strcpy(buf, "NONE"); break;
		case D3DCULL_CW:   strcpy(buf, "CW"); break;
		case D3DCULL_CCW:  strcpy(buf, "CCW"); break;
		default: sprintf(buf, "%d", t->states[o][y]); break;
	      }
	      break;
	    case D3DRS_FOGTABLEMODE:
	    case D3DRS_FOGVERTEXMODE:
	      switch ((D3DFOGMODE)t->states[o][y]) {
		case D3DFOG_NONE:   strcpy(buf, "NONE"); break;
		case D3DFOG_EXP:    strcpy(buf, "EXP"); break;
		case D3DFOG_EXP2:   strcpy(buf, "EXP2"); break;
		case D3DFOG_LINEAR: strcpy(buf, "LINEAR"); break;
		default: sprintf(buf, "%d", t->states[o][y]); break;
	      }
	      break;
	    case D3DRS_ZFUNC:
	    case D3DRS_ALPHAFUNC:
	    case D3DRS_STENCILFUNC:
	    case D3DRS_CCW_STENCILFUNC:
	      switch ((D3DCMPFUNC)t->states[o][y]) {
		case D3DCMP_NEVER       : strcpy(buf, "NEVER"); break;
		case D3DCMP_LESS        : strcpy(buf, "LESS"); break;
		case D3DCMP_EQUAL       : strcpy(buf, "EQUAL"); break;
		case D3DCMP_LESSEQUAL   : strcpy(buf, "LESSEQUAL"); break;
		case D3DCMP_GREATER     : strcpy(buf, "GREATER"); break;
		case D3DCMP_NOTEQUAL    : strcpy(buf, "NOTEQUAL"); break;
		case D3DCMP_GREATEREQUAL: strcpy(buf, "GREATEREQUAL"); break;
		case D3DCMP_ALWAYS      : strcpy(buf, "ALWAYS"); break;
		default: sprintf(buf, "%d", t->states[o][y]); break;
	      }
	      break;
	    case D3DRS_BLENDOP:
	    case D3DRS_SRCBLENDALPHA:
	    case D3DRS_DESTBLENDALPHA:
	    case D3DRS_BLENDOPALPHA:
	      switch ((D3DBLENDOP)t->states[o][y]) {
	        case D3DBLENDOP_ADD        : strcpy(buf, "ADD"); break;
	        case D3DBLENDOP_SUBTRACT   : strcpy(buf, "SUBTRACT"); break;
	        case D3DBLENDOP_REVSUBTRACT: strcpy(buf, "REVSUBTRACT"); break;
	        case D3DBLENDOP_MIN        : strcpy(buf, "MIN"); break;
	        case D3DBLENDOP_MAX        : strcpy(buf, "MAX"); break;
		default: sprintf(buf, "%d", t->states[o][y]); break;
	      }
	      break;
	    case D3DRS_STENCILFAIL:
	    case D3DRS_STENCILZFAIL:
	    case D3DRS_STENCILPASS:
	    case D3DRS_CCW_STENCILFAIL:
	    case D3DRS_CCW_STENCILZFAIL:
	    case D3DRS_CCW_STENCILPASS:
	      switch ((D3DSTENCILOP)t->states[o][y]) {
	        case D3DSTENCILOP_KEEP   : strcpy(buf, "KEEP"); break;
	        case D3DSTENCILOP_ZERO   : strcpy(buf, "ZERO"); break;
	        case D3DSTENCILOP_REPLACE: strcpy(buf, "REPLACE"); break;
	        case D3DSTENCILOP_INCRSAT: strcpy(buf, "INCRSAT"); break;
	        case D3DSTENCILOP_DECRSAT: strcpy(buf, "DECRSAT"); break;
	        case D3DSTENCILOP_INVERT : strcpy(buf, "INVERT"); break;
	        case D3DSTENCILOP_INCR   : strcpy(buf, "INCR"); break;
	        case D3DSTENCILOP_DECR   : strcpy(buf, "DECR"); break;
		default: sprintf(buf, "%d", t->states[o][y]); break;
	      }
	      break;
	    // bool
	    case D3DRS_ZWRITEENABLE:
	    case D3DRS_ALPHATESTENABLE:
	    case D3DRS_LASTPIXEL:
	    case D3DRS_DITHERENABLE:
	    case D3DRS_ALPHABLENDENABLE:
	    case D3DRS_FOGENABLE:
	    case D3DRS_SPECULARENABLE:
	    case D3DRS_RANGEFOGENABLE:
	    case D3DRS_STENCILENABLE:
	    case D3DRS_CLIPPING:
	    case D3DRS_LIGHTING:
	    case D3DRS_COLORVERTEX:
	    case D3DRS_LOCALVIEWER:
	    case D3DRS_NORMALIZENORMALS:
	    case D3DRS_POINTSPRITEENABLE:
	    case D3DRS_POINTSCALEENABLE:
	    case D3DRS_MULTISAMPLEANTIALIAS:
	    case D3DRS_INDEXEDVERTEXBLENDENABLE:
	    case D3DRS_ANTIALIASEDLINEENABLE:
	    case D3DRS_TWOSIDEDSTENCILMODE:
	    case D3DRS_SRGBWRITEENABLE:
	    case D3DRS_SEPARATEALPHABLENDENABLE:
	      strcpy(buf, t->states[o][y] ? "TRUE" : "FALSE");
	      break;
	    // float
	    case D3DRS_FOGSTART:
	    case D3DRS_FOGEND:
	    case D3DRS_FOGDENSITY:
	    case D3DRS_POINTSIZE:
	    case D3DRS_POINTSIZE_MIN:
	    case D3DRS_POINTSCALE_A:
	    case D3DRS_POINTSCALE_B:
	    case D3DRS_POINTSCALE_C:
	    case D3DRS_POINTSIZE_MAX:
	    case D3DRS_TWEENFACTOR:
	    case D3DRS_DEPTHBIAS:
	      sprintf(buf, "%f", *((float *)&t->states[o][y]));
	      break;
	    // long
	    case D3DRS_ALPHAREF:
	    case D3DRS_STENCILREF:
	    case D3DRS_VERTEXBLEND:
	      sprintf(buf, "%d", t->states[o][y]);
	      break;
	    // mask
	    case D3DRS_COLORWRITEENABLE:
	      sprintf(buf, "0x%02x", t->states[o][y]);
	      break;
	    // hex
	    case D3DRS_STENCILMASK:
	    case D3DRS_STENCILWRITEMASK:
	    case D3DRS_CLIPPLANEENABLE:
	    case D3DRS_MULTISAMPLEMASK:
	      sprintf(buf, "0x%08x", t->states[o][y]);
	      break;
	    // unknown
	    case D3DRS_FOGCOLOR:
	    case D3DRS_BLENDFACTOR:
	    case D3DRS_TEXTUREFACTOR:
	    case D3DRS_AMBIENT:
            default:
              sprintf(buf, "%d", t->states[o][y]);
              break;
          }

          gt->SetCellValue(pos + d, col + 1, wxString(buf));

          d++;
        }
      }
    }
  }

  /* --------------------------------------------------------------
   */

  void SetConstantTable(ShaderRecord *o) {
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

      SetConstantTable(gt, true, o->pConstsOriginal, o->pShaderOriginal ? o->pShaderOriginal->GetBufferSize() : -1, 0, 0);
      SetConstantTable(gt, true, o->pConstsReplaced, o->pShaderReplaced ? o->pShaderReplaced->GetBufferSize() : -1, 1, 0);
      SetConstantTable(gt, true, o->pConstsRuntime , o->pShaderRuntime  ? o->pShaderRuntime->GetBufferSize()  : -1, 2, 0);
    }

    wxSize sz = gt->GetClientSize();
    int dv = sz.GetWidth() - gt->GetRowLabelSize();
    gt->SetColSize(0, dv / 3);
    gt->SetColSize(1, dv / 3);
    gt->SetColSize(2, dv / 3);
  }

  void SetConstSetTable(ShaderRecord *o) {
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

      SetConstantTable(gt, false, o->pConstsOriginal, o->pShaderOriginal ? o->pShaderOriginal->GetBufferSize() : -1, 0, 0);

      switch (o->pDX9ShaderType) {
	default:
	case SHADER_REPLACED: SetConstantTable(gt, false, o->pConstsReplaced, o->pShaderReplaced ? o->pShaderReplaced->GetBufferSize() : -1, 1, 0); break;
	case SHADER_ORIGINAL: SetConstantTable(gt, false, o->pConstsOriginal, o->pShaderOriginal ? o->pShaderOriginal->GetBufferSize() : -1, 1, 0); break;
	case SHADER_RUNTIME : SetConstantTable(gt, false, o->pConstsRuntime , o->pShaderRuntime  ? o->pShaderRuntime->GetBufferSize()  : -1, 1, 0); break;
      }

      RuntimeShaderRecord *r = o->pAssociate;
      if (r->frame_used[pass] >= 0)
	SetConstSetTable(gt, false, &r->traced[pass], -1, 2, 0);
    }

    wxSize sz = gt->GetClientSize();
    int dv = sz.GetWidth() - gt->GetRowLabelSize();
    gt->SetColSize(0, dv / 4);
    gt->SetColSize(1, dv / 4);
    gt->SetColSize(2, dv / 2);
  }

  void SetSamplerTable(ShaderRecord *o) {
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
	SetSamplerTable(gt, false, &r->traced[pass], -1, 0, 0);
    }

    wxSize sz = gt->GetClientSize();
    int dv = sz.GetWidth() - gt->GetRowLabelSize();
    gt->SetColSize(0, dv / 2);
    gt->SetColSize(1, dv / 2);
  }

  void SetStatesTable(int o) {
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

      if (sm->trackd[pass].frame_cntr > 0)
        SetRenderTable(gt, false, o, &sm->trackd[pass], -1, 0, 0);
    }

    wxSize sz = gt->GetClientSize();
    int dv = sz.GetWidth() - gt->GetRowLabelSize();
    gt->SetColSize(0, dv / 2);
    gt->SetColSize(1, dv / 2);
  }

  /* --------------------------------------------------------------
   */

  void SetImage(wxImage *im, IDirect3DSurface9 *pSurface) {
    bool valid = false;

    if (im && pSurface) {
      IDirect3DSurface9 *pBuf = NULL, *rBuf = NULL;
      D3DSURFACE_DESC VDesc;
      D3DLOCKED_RECT surf;

      // GetRenderTarget(0, &pRenderTarget);
      if (pSurface->GetDesc(&VDesc) == D3D_OK) {
	if (pSurface->LockRect(&surf, NULL, D3DLOCK_NOSYSLOCK) == D3D_OK) {
	  pSurface->UnlockRect();
	  pBuf = pSurface;

	  valid = true;
	}

	else if (VDesc.Usage & D3DUSAGE_DEPTHSTENCIL) {
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
		    case D3DFMT_D16_LOCKABLE: def[0].Name = "D16_LOCKABLE"; break;
		    case D3DFMT_D32: def[0].Name = "D32"; break;
		    case D3DFMT_D15S1: def[0].Name = "D15S1"; break;
		    case D3DFMT_D24S8: def[0].Name = "D24S8"; break;
		    case D3DFMT_D24X8: def[0].Name = "D24X8"; break;
		    case D3DFMT_D24X4S4: def[0].Name = "D24X4S4"; break;
		    case D3DFMT_D16: def[0].Name = "D16"; break;

		    case D3DFMT_D32F_LOCKABLE: def[0].Name = "D32F_LOCKABLE"; break;
		    case D3DFMT_D24FS8: def[0].Name = "D24FS8"; break;
		    case D3DFMT_D32_LOCKABLE: def[0].Name = "D32_LOCKABLE"; break;

		    case (D3DFORMAT)MAKEFOURCC('I','N','T','Z'): def[0].Name = "INTZ"; break;
		    case (D3DFORMAT)MAKEFOURCC('D','F','2','4'): def[0].Name = "DF24"; break;
		    case (D3DFORMAT)MAKEFOURCC('D','F','1','6'): def[0].Name = "DF16"; break;
		    case (D3DFORMAT)MAKEFOURCC('R','A','W','Z'): def[0].Name = "RAWZ"; break;
	      }

	      if (D3D_Effect.LoadEffect("TransferZ.fx", (D3DXMACRO *)&def)) {
		frame_trk = false;

		if (lastOBGEDirect3DDevice9->CreateRenderTarget(VDesc.Width, VDesc.Height, VDesc.Format, D3DMULTISAMPLE_NONE, 0, FALSE, &rBuf, NULL) == D3D_OK) {
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

		  lastOBGEDirect3DDevice9->SetStreamSource(0, D3D_EffectBuffer, 0, sizeof(D3D_sShaderVertex));
		  lastOBGEDirect3DDevice9->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);

		  lastOBGEDirect3DDevice9->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);
		  lastOBGEDirect3DDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		  lastOBGEDirect3DDevice9->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		  lastOBGEDirect3DDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		  lastOBGEDirect3DDevice9->BeginScene();

		  // Set up world/view/proj matrices to identity in case there's no vertex effect.
		  D3DXMATRIX mIdent;
		  D3DXMatrixIdentity(&mIdent);
		  // Sets up the viewport.
		  D3DVIEWPORT9 vp = { 0, 0, VDesc.Width, VDesc.Height, 0.0, 1.0 };

		  lastOBGEDirect3DDevice9->SetViewport(&vp);
		  lastOBGEDirect3DDevice9->SetTransform(D3DTS_PROJECTION, &mIdent);
		  lastOBGEDirect3DDevice9->SetTransform(D3DTS_VIEW, &mIdent);
		  lastOBGEDirect3DDevice9->SetTransform(D3DTS_WORLD, &mIdent);

		  D3D_Effect.Effect->SetTexture("zbufferTexture", surfaceTexture[pSurface]->tex);

		  UINT passes;
		  D3D_Effect.Effect->Begin(&passes, NULL);

		  for (UINT pass = 0; pass < passes; pass) {
		    D3D_Effect.Effect->BeginPass(pass);
		    lastOBGEDirect3DDevice9->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		    D3D_Effect.Effect->EndPass();
		  }

		  D3D_Effect.Effect->End();
		  lastOBGEDirect3DDevice9->EndScene();

		  if (lastOBGEDirect3DDevice9->CreateOffscreenPlainSurface(VDesc.Width, VDesc.Height, VDesc.Format, D3DPOOL_SYSTEMMEM, &pBuf, NULL) == D3D_OK) {
		    if (lastOBGEDirect3DDevice9->GetRenderTargetData(rBuf, pBuf) == D3D_OK) {
		      valid = true;
		    }
		  }
		}

		frame_trk = true;
	      }
	    }
	  }
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
		  if (pass == OBGEPASS_REFLECTION)
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
			int a = _argb[((hh * w) + ww) * 4 + 0];
			int b = _argb[((hh * w) + ww) * 4 + 1];
			int g = _argb[((hh * w) + ww) * 4 + 2];
			int r = _argb[((hh * w) + ww) * 4 + 3];
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
		  if (pass == OBGEPASS_WATERHEIGHTMAP)
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
#endif
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

  void UpdateOptions(ShaderRecord *o) {
    SDShaderVersion->Clear();

    if (o) {
      wxMenuItem *mi; mi = SDOptions->FindChildItem(wxID_UPGRADE, NULL);
      int idx = 0, sel = wxNOT_FOUND;

      if (o->iType == SHADER_PIXEL) {
	if (!mi || !mi->IsChecked()) {
	  SDShaderVersion->Append(wxString("ps_1_1")); if (!strcmp(o->pProfile, "ps_1_1")) sel = idx; idx++;
	  SDShaderVersion->Append(wxString("ps_1_2")); if (!strcmp(o->pProfile, "ps_1_2")) sel = idx; idx++;
	  SDShaderVersion->Append(wxString("ps_1_3")); if (!strcmp(o->pProfile, "ps_1_3")) sel = idx; idx++;
	  SDShaderVersion->Append(wxString("ps_1_4")); if (!strcmp(o->pProfile, "ps_1_4")) sel = idx; idx++;
	}

	SDShaderVersion->Append(wxString("ps_2_0")); if (!strcmp(o->pProfile, "ps_2_0")) sel = idx; idx++;
	SDShaderVersion->Append(wxString("ps_2_a")); if (!strcmp(o->pProfile, "ps_2_a")) sel = idx; idx++;
	SDShaderVersion->Append(wxString("ps_2_b")); if (!strcmp(o->pProfile, "ps_2_b")) sel = idx; idx++;
	SDShaderVersion->Append(wxString("ps_2_sw")); if (!strcmp(o->pProfile, "ps_2_sw")) sel = idx; idx++;
	SDShaderVersion->Append(wxString("ps_3_0")); if (!strcmp(o->pProfile, "ps_3_0")) sel = idx; idx++;
	SDShaderVersion->Append(wxString("ps_3_sw")); if (!strcmp(o->pProfile, "ps_3_sw")) sel = idx; idx++;
      }

      else if (o->iType == SHADER_VERTEX) {
	if (!mi || !mi->IsChecked()) {
	  SDShaderVersion->Append(wxString("vs_1_1")); if (!strcmp(o->pProfile, "vs_1_1")) sel = idx; idx++;
	}

	SDShaderVersion->Append(wxString("vs_2_0")); if (!strcmp(o->pProfile, "vs_2_0")) sel = idx; idx++;
	SDShaderVersion->Append(wxString("vs_2_a")); if (!strcmp(o->pProfile, "vs_2_a")) sel = idx; idx++;
	SDShaderVersion->Append(wxString("vs_2_sw")); if (!strcmp(o->pProfile, "vs_2_sw")) sel = idx; idx++;
	SDShaderVersion->Append(wxString("vs_3_0")); if (!strcmp(o->pProfile, "vs_3_0")) sel = idx; idx++;
	SDShaderVersion->Append(wxString("vs_3_sw")); if (!strcmp(o->pProfile, "vs_3_sw")) sel = idx; idx++;
      }

      if (sel != wxNOT_FOUND)
	SDShaderVersion->SetSelection(sel);
      else if (idx > 0)
	SDShaderVersion->SetSelection(0);
    }
  }

  /* --------------------------------------------------------------
   */

  void GetRecordStatus(ShaderRecord *o) {
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
	  SDComboShader->SetItemBitmap(x, wxBError);
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
	  SDComboShader->SetItemBitmap(x, wxBWarning);
      }
      else {
	status = "Status: compiled as %s, enabled";
	SDShaderCompile->Disable();
	if (x != wxNOT_FOUND)
	  SDComboShader->SetItemBitmap(x, wxBApplied);
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
	  SDComboShader->SetItemBitmap(x, wxBApplied);
      }
      else if (o && !o->pShaderOriginal) {
	status = "Status: missing";
	SDShaderCompile->Disable();
	if (x != wxNOT_FOUND)
	  SDComboShader->SetItemBitmap(x, wxBMissing);
      }
      else {
	status = "Status: not compiled";
	SDShaderCompile->Enable();
	if (x != wxNOT_FOUND)
	  SDComboShader->SetItemBitmap(x, wxBUnhooked);
      }
    }

    sprintf(buf, status, o ? o->pProfile : "-");
    SDStatusShader->SetLabel(buf);

    if (o && o->pSourceRuntime  && !o->pShaderRuntime )
      SDShaderCompile->Enable();
    if (o && o->pSourceReplaced && !o->pShaderReplaced)
      SDShaderCompile->Enable();
    if (o && o->pAsmblyReplaced && !o->pShaderReplaced)
      SDShaderCompile->Enable();
  }

  void UpdateRecord(ShaderRecord *o) {
    GetRecordStatus(o);

    /* source has changed */
    wxString oo = SDSourceEditor->GetValue();
    const wxChar *ooo = oo.GetData();
    int size = strlen(ooo);

    SDButtonShaderSave->Disable();

    if (o) {
      if (o->pSourceRuntime) {
	if (strcmp(oo.GetData(), o->pSourceRuntime))
	  SDButtonShaderSave->Enable();
      }
      else if (o->pSourceReplaced) {
	if (strcmp(oo.GetData(), o->pSourceReplaced))
	  SDButtonShaderSave->Enable();
      }
      else {
	SDButtonShaderSave->Enable();
      }
    }
  }

  void SetRecord(ShaderRecord *o) {
    if (o && !o->pDisasmbly)
      o->DisassembleShader();

    GetRecordStatus(o);
    UpdateOptions(o);

    SDButtonShaderSave->Disable();

    /* HLSL goes always (load/save/saveas) */
    if (o && o->pSourceRuntime)
      SDSourceEditor->SetValue(wxString(o->pSourceRuntime));
    else if (o && o->pSourceReplaced)
      SDSourceEditor->SetValue(wxString(o->pSourceReplaced));
    else
      SDSourceEditor->SetValue(wxString(""));

    /* Assembler goes always (load/save/saveas) */
    if (o && o->pAsmblyReplaced)
      SDAssemblyEditor->SetValue(wxString(o->pAsmblyReplaced));
    else
      SDAssemblyEditor->SetValue(wxString(""));

    /* Errors are only deactivated */
    if (o && o->pErrorMsgs) {
      SDErrorView->Enable();
      SDErrorView->SetValue(wxString((char *)o->pErrorMsgs->GetBufferPointer()));
    }
    else {
      SDErrorView->SetValue(wxString(""));
      SDErrorView->Disable();
    }

    /* Disassembly is only deactivated */
    if (o && o->pDisasmbly) {
      SDDisassemblyView->Enable();
      SDDisassemblyView->SetValue(wxString((char *)o->pDisasmbly->GetBufferPointer()));
    }
    else {
      SDDisassemblyView->SetValue(wxString(""));
      SDDisassemblyView->Disable();
    }

    SetConstantTable(o);
    SetSamplerTable(o);
  }

  /* --------------------------------------------------------------
   */

  void SDPaintRT(wxPaintEvent& event) {
    wxPaintDC dc(SDRendertargetView);

    /* refresh only if visible */
    if (SDViewSwitch->GetSelection() != 1)
      return;
    if (SDSurfaceSwitch->GetSelection() != 0)
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

  void SDPaintDS(wxPaintEvent& event) {
    wxPaintDC dc(SDDepthStencilView);

    /* refresh only if visible */
    if (SDViewSwitch->GetSelection() != 1)
      return;
    if (SDSurfaceSwitch->GetSelection() != 1)
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

  const char *GetViewStatus(int o, IDirect3DSurface9 *pSurface, bool comma = false) {
    D3DSURFACE_DESC VDesc;
    static char buf[256];
    buf[0] = '\0';

    if (!pSurface && sm) {
      pSurface = sm->trackd[pass].rt[o];
      if ((int)pSurface == -1) pSurface = 0;
    }
    if (pSurface && (pSurface->GetDesc(&VDesc) == D3D_OK)) {
      sprintf(buf, "%s%s, %dx%d", comma ? ", " : "", findFormat(VDesc.Format), VDesc.Width, VDesc.Height);
    }

    return buf;
  }

  void SetView(int o) {
    IDirect3DSurface9 *_rt = NULL;
    IDirect3DSurface9 *_ds = NULL;

    if (sm) {
      _rt = sm->trackd[pass].rt[o];
      _ds = sm->trackd[pass].ds[o];
      if ((int)_rt == -1) _rt = 0;
      if ((int)_ds == -1) _ds = 0;
    }

    SDStatusRT->SetLabel(wxString(GetViewStatus(o, _rt)));

    SetImage(&rt, _rt);
    SetImage(&ds, _ds);

    crt.SelectObject(wxBitmap(rt));
    cds.SelectObject(wxBitmap(ds));

//  SetImage(&rt, _rt); wxImage __rt = rt.Rescale(SDRendertargetView->GetClientSize().GetWidth(), SDRendertargetView->GetClientSize().GetHeight(), wxIMAGE_QUALITY_HIGH);
//  SetImage(&ds, _ds); wxImage __ds = ds.Rescale(SDDepthStencilView->GetClientSize().GetWidth(), SDDepthStencilView->GetClientSize().GetHeight(), wxIMAGE_QUALITY_HIGH);
//
//  SDRendertargetView->SetBitmap(wxBitmap(__rt));
//  SDDepthStencilView->SetBitmap(wxBitmap(__ds));

    SetStatesTable(o);

    DoRefresh();
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
	if (pass == OBGEPASS_ANY) {
	  char num[256] = ""; bool has = false;
	  for (int p = 0; p < OBGEPASS_NUM; p++) {
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

  void UpdateFrameScene() {
    char buf[256];

    int o = SDChoiceScene->GetSelection();
    SDChoiceScene->Clear();

    if (sm) {
      struct ShaderManager::track *t = &sm->trackd[pass];

      for (int scene = 0; scene < t->frame_cntr; scene++) {
      	assert(scene < 256);

      	/* verify if the scene (which is a global identifier) occured in this pass, filter */
      	if (t->frame_used[scene] > 0) {
          const char *fmt = GetViewStatus(scene, NULL, true);

	  /* info with frame-number */
          sprintf(buf, "Scene %d%s [frame %d]", t->frame_pass[scene], fmt, t->frame_used[scene]);

          SDChoiceScene->Append(wxString(buf));
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

  void UpdateFrame() {
    if (SDViewSwitch->GetSelection() == 0)
      UpdateFrameShaders();
    else if (SDViewSwitch->GetSelection() == 1)
      UpdateFrameScene();
  }

  /* --------------------------------------------------------------
   */

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

      mi = SDOptions->FindChildItem(wxID_COMPILE, NULL); mi->Check(sm->CompileSources());
      mi = SDOptions->FindChildItem(wxID_SAVEBIN, NULL); mi->Check(sm->SaveShaderOverride());
      mi = SDOptions->FindChildItem(wxID_LEGACY, NULL); mi->Check(sm->UseLegacyCompiler());
      mi = SDOptions->FindChildItem(wxID_OPTIMIZE, NULL); mi->Check(sm->Optimize());
      mi = SDOptions->FindChildItem(wxID_MAXIMUM, NULL); mi->Check(sm->MaximumSM());
      mi = SDOptions->FindChildItem(wxID_UPGRADE, NULL); mi->Check(sm->UpgradeSM());
      mi = SDOptions->FindChildItem(wxID_RUNTIME, NULL); mi->Check(sm->RuntimeSources());
    }

    /* renderpasses might have changed since last activation */
    int j = SDChoicePass->GetSelection(), h = 0;
    wxString jj = SDChoicePass->GetStringSelection();
    const char *jjj = jj.GetData();
    char buf[256];

    /* ------------------------------------------------ */
    SDChoicePass->Clear();

    for (int p = 0; p < OBGEPASS_NUM; p++) {
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

    /* ------------------------------------------------ */
    if (SDViewSwitch->GetSelection() == 0) {
      int j = SDComboShader->GetSelection();
      if (j == wxNOT_FOUND) {
	SDComboShader->Clear();

	if (sm) {
	  BuiltInShaderList::iterator BShader = sm->BuiltInShaders.begin();
	  while (BShader != sm->BuiltInShaders.end()) {
	    if ((*BShader)->pErrorMsgs) {
	      char *buf = (char *)(*BShader)->pErrorMsgs->GetBufferPointer();

	      if (strstr(buf, "error")) {
		SDComboShader->Append(wxBName, wxBError, (void *)(*BShader));
	      }
	      else if (strstr(buf, "warning")) {
		SDComboShader->Append(wxBName, wxBWarning, (void *)(*BShader));
	      }
	      else {
		SDComboShader->Append(wxBName, wxBApplied, (void *)(*BShader));
	      }
	    }
	    else {
	      if ((*BShader)->pShaderRuntime) {
		SDComboShader->Append(wxBName, wxBApplied, (void *)(*BShader));
	      }
	      else if ((*BShader)->pShaderReplaced) {
		SDComboShader->Append(wxBName, wxBApplied, (void *)(*BShader));
	      }
	      else if (!(*BShader)->pShaderOriginal) {
		SDComboShader->Append(wxBName, wxBMissing, (void *)(*BShader));
	      }
	      else {
		SDComboShader->Append(wxBName, wxBUnhooked, (void *)(*BShader));
	      }
	    }

	    BShader++;
	  }
	}

	if (!SDComboShader->IsEmpty()) {
	  SDPanelShaders->Show();
	  SDComboShader->SetSelection(0);
	}
	else
	  SDPanelShaders->Hide();
      }
    }

    /* ------------------------------------------------ */
    else if (SDViewSwitch->GetSelection() == 1) {
      int k = SDChoiceScene->GetSelection();
      if (k == wxNOT_FOUND) {
	if (!SDChoiceScene->IsEmpty()) {
	  SDPanelScenes->Show();
	  SDChoiceScene->SetSelection(0);
	}
	else
	  SDPanelScenes->Hide();
      }
    }

    /* we basically assume the frame/scene numbers change all the time */
    UpdateFrame();

    if (SDViewSwitch->GetSelection() == 0)
      DoShaderSwitch(forced || (p != pass));
    else if (SDViewSwitch->GetSelection() == 1)
      DoScenesSwitch(forced || (p != pass));
  }

  /* --------------------------------------------------------------
   */

  virtual void DoViewSwitch(wxNotebookEvent& event) {
    int o = event.GetSelection();
    wxObject *hit = event.GetEventObject();

    if (hit == SDViewSwitch) {
      if (o == 0) {
	SDPanelShaders->Show();
	SDPanelScenes->Hide();

	UpdateFrameShaders();
      }
      else if (o == 1) {
	SDPanelShaders->Hide();
	SDPanelScenes->Show();

	UpdateFrameScene();
      }
    }

    event.Skip();
  }

  virtual void DoSurfaceSwitch(wxNotebookEvent& event) {
    int o = event.GetSelection();
    wxObject *hit = event.GetEventObject();

    if (hit == SDSurfaceSwitch) {
      if (o == 0) {
	SDRendertarget->Show();
	SDDepthStencil->Hide();
      }
      else if (o == 1) {
	SDRendertarget->Hide();
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
    if ((currs != o) || force) {
      currs = o;
      SetRecord(o);
    }

    /* trace might have changed since last activation */
    if (currs) {
      SetConstSetTable(currs);
      SetSamplerTable(currs);
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

  /* --------------------------------------------------------------
   */

  virtual void DoShaderUpdate(wxCommandEvent& event) {
    DoShaderUpdate();
    event.Skip();
  }

  void DoShaderUpdate() {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    UpdateRecord(o);
  }

  virtual void DoShaderSave(wxCommandEvent& event) {
    DoShaderSave();
    event.Skip();
  }

  void DoShaderSave() {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    wxString oo = SDSourceEditor->GetValue();
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

	if (o->RuntimeShader(ooo))
	  SetRecord(o);
        else
	  UpdateRecord(o);

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

  virtual void DoShaderVersion(wxCommandEvent& event) {
    DoShaderVersion();
    event.Skip();
  }

  void DoShaderVersion() {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    wxString oo = SDSourceEditor->GetValue();
    const wxChar *ooo = oo.GetData();
    int size = strlen(ooo);

    if (o) {
      wxString ss = SDShaderVersion->GetStringSelection();
      const wxChar *sss = ss.GetData();

      /* the profile changed */
      if (!o->pProfile || strcmp(sss, o->pProfile)) {
	if (o->RuntimeShader(ooo, sss))
	  SetRecord(o);
	else
	  UpdateRecord(o);

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
    event.Skip();
  }

  void DoShaderCompile() {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    wxString oo = SDSourceEditor->GetValue();
    const wxChar *ooo = oo.GetData();
    int size = strlen(ooo);

    if (o) {
      if (o->RuntimeShader(ooo))
	SetRecord(o);
      else
	UpdateRecord(o);

      /* trigger re-creation of the DX9-class */
      if (SDShaderEnable->Get3StateValue() == wxCHK_CHECKED)
	o->pAssociate->ActivateShader(SHADER_RUNTIME);
      else if (SDShaderEnable->Get3StateValue() == wxCHK_UNDETERMINED)
	o->pAssociate->ActivateShader(SHADER_REPLACED);
      else //if (SDShaderEnable->Get3StateValue() == wxCHK_UNCHECKED)
	o->pAssociate->ActivateShader(SHADER_ORIGINAL);
    }
  }

  virtual void DoShaderToggle(wxCommandEvent& event) {
    DoShaderToggle();
    event.Skip();
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

  /* --------------------------------------------------------------
   */

  virtual void DoOptions(wxCommandEvent& event) {
    ShaderRecord *o = FindShaderRecord(NULL, SDComboShader->GetStringSelection());

    /* ------------------------------------------------ */
    UpdateOptions(o);

    if (sm) {
      wxMenuItem *mi;

      mi = SDOptions->FindChildItem(wxID_COMPILE,  NULL); sm->CompileSources(mi->IsChecked());
      mi = SDOptions->FindChildItem(wxID_SAVEBIN,  NULL); sm->SaveShaderOverride(mi->IsChecked());
      mi = SDOptions->FindChildItem(wxID_OPTIMIZE, NULL); sm->Optimize(mi->IsChecked());
      mi = SDOptions->FindChildItem(wxID_LEGACY,   NULL); sm->UseLegacyCompiler(mi->IsChecked());
      mi = SDOptions->FindChildItem(wxID_MAXIMUM,  NULL); sm->MaximumSM(mi->IsChecked());
      mi = SDOptions->FindChildItem(wxID_UPGRADE,  NULL); sm->UpgradeSM(mi->IsChecked());
      mi = SDOptions->FindChildItem(wxID_RUNTIME,  NULL); sm->RuntimeSources(mi->IsChecked());

      GetRecordStatus(o);
    }

    event.Skip();
  }

  virtual void DoResize(wxSizeEvent& event) {
    DoRefresh();
    event.Skip();
  }

  void DoRefresh() {
    /* refresh only if visible */
    if (SDViewSwitch->GetSelection() == 1) {
      if (SDSurfaceSwitch->GetSelection() == 0)
        SDRendertarget->Refresh();
      else if (SDSurfaceSwitch->GetSelection() == 1)
        SDDepthStencil->Refresh();
    }
  }

  virtual void DoClose(wxCloseEvent& event) {
  }
};

// this is in any .cpp in library
class GUIs_App: public wxApp
{
public:
  bool OnInit()
  {
 	return true;
  }
};

static wxAppConsole *wxCreateApp()
{
  wxAppConsole::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE,"my app");
  return new GUIs_App;
}

WXDLLIMPEXP_BASE void wxSetInstance(HINSTANCE hInst);

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

DebugWindow::DebugWindow() {
  int argc = 0; char** argv = NULL;

//assert(NULL);

  HINSTANCE exe = GetModuleHandle(NULL);
  HINSTANCE dll = GetModuleHandle("OBGEv2.dll");

  wxSetInstance(dll);
  wxApp::SetInitializerFunction(wxCreateApp);
  wxInitialize(argc, argv);

  GUIs_ShaderDeveloper *
  sdev = new GUIs_ShaderDeveloper(NULL, wxID_ANY, "OBGE Frame-Walker", wxPoint(-100, 0), wxSize(630, 704));
  sdev->Show();
  sdev->SetPosition(wxPoint(/*-70*/0, 0));
  this->sdev = (void *)sdev;

//assert(NULL);
}

DebugWindow::~DebugWindow() {
  GUIs_ShaderDeveloper *
  sdev = (GUIs_ShaderDeveloper *)this->sdev;
  delete sdev;

  wxUninitialize();
}

#endif
#endif

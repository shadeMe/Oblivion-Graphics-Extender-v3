///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  4 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __GUIs_ShaderDeveloper__
#define __GUIs_ShaderDeveloper__

#include <wx/string.h>
#include <wx/choice.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/toolbar.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/panel.h>
#include <wx/notebook.h>
#include <wx/statusbr.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class GUIs_ShaderDeveloper
///////////////////////////////////////////////////////////////////////////////
class GUIs_ShaderDeveloper : public wxFrame 
{
	private:
	
	protected:
		wxChoice* SDChoicePass;
		wxNotebook* SDNotebook;
		wxPanel* SDPanelShaders;
		wxChoice* SDChoiceShader;
		wxToolBar* SDToolbarShader;
		wxBitmapButton* SDButtonShaderLoad;
		wxBitmapButton* SDButtonShaderSave;
		wxBitmapButton* SDButtonShaderSaveAs;
		wxButton* SDShaderCompile;
		wxTextCtrl* SDEditorShader;
		wxStaticText* SDStatusShader;
		wxCheckBox* SDShaderEnable;
		wxNotebook* SDShaderVariables;
		wxPanel* SDShaderMatrices;
		wxStaticText* m_staticText2;
		wxStaticText* m_staticText3;
		wxStaticText* m_staticText4;
		wxStaticText* m_staticText5;
		wxStaticText* m_staticText6;
		wxStaticText* m_staticText7;
		wxStaticText* m_staticText8;
		wxStaticText* m_staticText9;
		wxStaticText* m_staticText10;
		wxStaticText* m_staticText11;
		wxStaticText* m_staticText12;
		wxStaticText* m_staticText13;
		wxStaticText* m_staticText14;
		wxStaticText* m_staticText15;
		wxStaticText* m_staticText16;
		wxStaticText* m_staticText17;
		wxStaticText* m_staticText18;
		wxStaticLine* m_staticline1;
		wxPanel* SDShaderParameters;
		wxPanel* SDShaderInputs;
		wxPanel* SDShaderOutputs;
		wxPanel* SDPanelRendertargets;
		wxStatusBar* SDStatusBar;
	
	public:
		
		GUIs_ShaderDeveloper( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 630,704 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~GUIs_ShaderDeveloper();
	
};

#endif //__GUIs_ShaderDeveloper__

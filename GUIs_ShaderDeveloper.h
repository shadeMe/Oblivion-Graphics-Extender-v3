///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  4 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __GUIs_ShaderDeveloper__
#define __GUIs_ShaderDeveloper__

#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/bmpcbox.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/toolbar.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/notebook.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/grid.h>
#include <wx/splitter.h>
#include <wx/slider.h>
#include <wx/statusbr.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

#define wxID_COMPILE 1000
#define wxID_SAVEBIN 1001
#define wxID_LEGACY 1002
#define wxID_UPGRADE 1003
#define wxID_MAXIMUM 1004
#define wxID_OPTIMIZE 1005
#define wxID_RUNTIME 1006

///////////////////////////////////////////////////////////////////////////////
/// Class wxShaderDeveloper
///////////////////////////////////////////////////////////////////////////////
class wxShaderDeveloper : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* SDMenubar;
		wxMenu* SDOptions;
		wxChoice* SDChoicePass;
		wxNotebook* SDViewSwitch;
		wxPanel* SDPanelShaders;
		wxSplitterWindow* SDSpitterShaders;
		wxPanel* SDPanelShadersTop;
		wxBitmapComboBox* SDComboShader;
		wxToolBar* SDToolbarShader;
		wxBitmapButton* SDButtonShaderLoad;
		wxBitmapButton* SDButtonShaderSave;
		wxBitmapButton* SDButtonShaderSaveAs;
		wxChoice* SDShaderVersion;
		wxButton* SDShaderCompile;
		wxNotebook* SDCodeSwitch;
		wxPanel* SDSource;
		wxTextCtrl* SDSourceEditor;
		wxPanel* SDAssembly;
		wxTextCtrl* SDAssemblyEditor;
		wxPanel* SDErrors;
		wxTextCtrl* SDErrorView;
		wxPanel* Disassembly;
		wxTextCtrl* SDDisassemblyView;
		wxStaticText* SDStatusShader;
		wxCheckBox* SDShaderEnable;
		wxPanel* SDPanelShadersBottom;
		wxNotebook* SDShaderVariables;
		wxPanel* SDShaderConstantTable;
		wxGrid* SDShaderConstantGrid;
		wxPanel* SDShaderConstants;
		wxGrid* SDShaderConstSetGrid;
		wxPanel* SDShaderSamplers;
		wxGrid* SDShaderSamplerGrid;
		wxPanel* SDPanelScenes;
		wxSplitterWindow* SDSplitterScenes;
		wxPanel* SDPanelScenesTop;
		wxChoice* SDChoiceScene;
		wxToolBar* SDToolbarRendertarget;
		wxBitmapButton* SDButtonRTCopy;
		wxBitmapButton* SDButtonRTSaveAs;
		wxNotebook* SDSurfaceSwitch;
		wxPanel* SDRendertarget;
		wxPanel* SDRendertargetView;
		wxPanel* SDDepthStencil;
		wxPanel* SDDepthStencilView;
		wxSlider* SDDepthStencilDepth;
		wxStaticText* SDStatusRT;
		wxPanel* SDPanelScenesBottom;
		wxNotebook* SDSceneVariables;
		wxPanel* SDSceneStates;
		wxGrid* SDSceneStateGrid;
		wxStatusBar* SDStatusBar;
		
		// Virtual event handlers, overide them in your derived class
		virtual void DoActivate( wxActivateEvent& event ) { event.Skip(); }
		virtual void DoClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void DoResize( wxSizeEvent& event ) { event.Skip(); }
		virtual void DoUpdate( wxUpdateUIEvent& event ) { event.Skip(); }
		virtual void DoOptions( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoRenderpassSwitch( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoViewSwitch( wxNotebookEvent& event ) { event.Skip(); }
		virtual void DoShaderSwitch( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderLoad( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderSave( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderSaveAs( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderVersion( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderCompile( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderUpdate( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderToggle( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoScenesSwitch( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoSurfaceSwitch( wxNotebookEvent& event ) { event.Skip(); }
		virtual void SDPaintRT( wxPaintEvent& event ) { event.Skip(); }
		virtual void SDPaintDS( wxPaintEvent& event ) { event.Skip(); }
		
	
	public:
		
		wxShaderDeveloper( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 630,704 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~wxShaderDeveloper();
		
		void SDSpitterShadersOnIdle( wxIdleEvent& )
		{
			SDSpitterShaders->SetSashPosition( 0 );
			SDSpitterShaders->Disconnect( wxEVT_IDLE, wxIdleEventHandler( wxShaderDeveloper::SDSpitterShadersOnIdle ), NULL, this );
		}
		
		void SDSplitterScenesOnIdle( wxIdleEvent& )
		{
			SDSplitterScenes->SetSashPosition( 410 );
			SDSplitterScenes->Disconnect( wxEVT_IDLE, wxIdleEventHandler( wxShaderDeveloper::SDSplitterScenesOnIdle ), NULL, this );
		}
	
};

#endif //__GUIs_ShaderDeveloper__

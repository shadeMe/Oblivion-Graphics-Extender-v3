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

#define wxID_SCOMPILE 1000
#define wxID_SSAVEBIN 1001
#define wxID_SLEGACY 1002
#define wxID_SUPGRADE 1003
#define wxID_SMAXIMUM 1004
#define wxID_SOPTIMIZE 1005
#define wxID_SRUNTIME 1006
#define wxID_ECOMPILE 1007
#define wxID_ESAVEBIN 1008
#define wxID_ELEGACY 1009
#define wxID_EOPTIMIZE 1010
#define wxID_PROFILE 1011
#define wxID_KILLTEX 1012
#define wxID_AMPLIFY 1013
#define wxID_MIPGAMMA 1014
#define wxID_AF1 1015
#define wxID_AF2 1016
#define wxID_AF4 1017
#define wxID_AF8 1018
#define wxID_AF16 1019
#define wxID_AF32 1020
#define wxID_LINEAR 1021

///////////////////////////////////////////////////////////////////////////////
/// Class wxShaderDeveloper
///////////////////////////////////////////////////////////////////////////////
class wxShaderDeveloper : public wxFrame 
{
	private:
	
	protected:
		wxMenuBar* SDMenubar;
		wxMenu* SDShaderOptions;
		wxMenu* SDEffectOptions;
		wxMenu* SDProfileOptions;
		wxMenu* SDTools;
		wxMenu* SDToolsSettings;
		wxMenu* SDTweaks;
		wxMenu* SDTweaksAF;
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
		wxButton* SDShaderCompileAll;
		wxButton* SDShaderFlush;
		wxNotebook* SDShaderCodeSwitch;
		wxPanel* SDShaderSource;
		wxTextCtrl* SDShaderSourceEditor;
		wxPanel* SDShaderAssembly;
		wxTextCtrl* SDShaderAssemblyEditor;
		wxPanel* SDShaderErrors;
		wxTextCtrl* SDShaderErrorView;
		wxPanel* SDShaderDisassembly;
		wxTextCtrl* SDShaderDisassemblyView;
		wxStaticText* SDStatusShader;
		wxCheckBox* SDShaderEnable;
		wxStaticText* SDShaderPairing;
		wxCheckBox* SDShaderMark;
		wxPanel* SDPanelShadersBottom;
		wxNotebook* SDShaderVariables;
		wxPanel* SDShaderConstantTable;
		wxGrid* SDShaderConstantGrid;
		wxPanel* SDShaderConstants;
		wxGrid* SDShaderConstSetGrid;
		wxPanel* SDShaderSamplers;
		wxGrid* SDShaderSamplerGrid;
		wxPanel* SDPanelEffects;
		wxSplitterWindow* SDSpitterEffects;
		wxPanel* SDPanelEffectsTop;
		wxBitmapComboBox* SDComboEffect;
		wxToolBar* SDToolbarEffect;
		wxBitmapButton* SDButtonEffectNew;
		wxBitmapButton* SDButtonEffectLoad;
		wxBitmapButton* SDButtonEffectSave;
		wxBitmapButton* SDButtonEffectSaveAs;
		wxButton* SDEffectCompile;
		wxNotebook* SDEffectCodeSwitch;
		wxPanel* SDEffectSource;
		wxTextCtrl* SDEffectSourceEditor;
		wxPanel* SDEffectErrors;
		wxTextCtrl* SDEffectErrorView;
		wxPanel* SDEffectDisassembly;
		wxTextCtrl* SDEffectDisassemblyView;
		wxStaticText* SDStatusEffect;
		wxCheckBox* SDEffectEnable;
		wxPanel* SDPanelEffectsBottom;
		wxNotebook* SDEffectVariables;
		wxPanel* SDEffectConstants;
		wxGrid* SDEffectConstSetGrid;
		wxPanel* SDEffectTextures;
		wxGrid* SDEffectTexturesGrid;
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
		wxPanel* SDRendertargetGrabbed;
		wxPanel* SDRendertargetGrabbedView;
		wxPanel* SDDepthStencil;
		wxPanel* SDDepthStencilView;
		wxSlider* SDDepthStencilDepth;
		wxStaticText* SDStatusRT;
		wxPanel* SDPanelScenesBottom;
		wxNotebook* SDSceneVariables;
		wxPanel* SDSceneStates;
		wxGrid* SDSceneStateGrid;
		wxPanel* SDPanelStats;
		wxSplitterWindow* SDSplitterStats;
		wxPanel* SDPanelStatsTop;
		wxPanel* SDStatsView;
		wxStaticText* SDStatusStats;
		wxCheckBox* SDStatsNormalize;
		wxPanel* SDPanelStatsBottom;
		wxChoice* SDChoiceStats;
		wxNotebook* SDStatsVariables;
		wxPanel* SDSceneStats;
		wxGrid* SDSceneStatsGrid;
		wxStatusBar* SDStatusBar;
		
		// Virtual event handlers, overide them in your derived class
		virtual void DoActivate( wxActivateEvent& event ) { event.Skip(); }
		virtual void DoClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void DoResize( wxSizeEvent& event ) { event.Skip(); }
		virtual void DoUpdate( wxUpdateUIEvent& event ) { event.Skip(); }
		virtual void DoShaderOptions( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoEffectOptions( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoProfileOptions( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoToolPMtoQDM( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoToolRemipCLR( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoToolRemipNM( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoAF( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoLinear( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoRenderpassSwitch( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoViewSwitch( wxNotebookEvent& event ) { event.Skip(); }
		virtual void DoShaderSwitch( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderLoad( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderSave( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderSaveAs( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderVersion( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderCompile( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderCompileAll( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderFlush( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderHighlight( wxKeyEvent& event ) { event.Skip(); }
		virtual void DoShaderUpdate( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoAssemblerHighlight( wxKeyEvent& event ) { event.Skip(); }
		virtual void DoShaderToggle( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoMarkToggle( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoShaderConstantChange( wxGridEvent& event ) { event.Skip(); }
		virtual void DoShaderConstantSelect( wxGridEvent& event ) { event.Skip(); }
		virtual void DoShaderSamplerChange( wxGridEvent& event ) { event.Skip(); }
		virtual void DoShaderSamplerSelect( wxGridEvent& event ) { event.Skip(); }
		virtual void DoEffectSwitch( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoEffectAdd( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoEffectLoad( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoEffectSave( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoEffectSaveAs( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoEffectCompile( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoEffectHighlight( wxKeyEvent& event ) { event.Skip(); }
		virtual void DoEffectUpdate( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoEffectToggle( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoEffectConstantChange( wxGridEvent& event ) { event.Skip(); }
		virtual void DoEffectConstantSelect( wxGridEvent& event ) { event.Skip(); }
		virtual void DoEffectTextureChange( wxGridEvent& event ) { event.Skip(); }
		virtual void DoEffectTextureSelect( wxGridEvent& event ) { event.Skip(); }
		virtual void DoScenesSwitch( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoSurfaceSwitch( wxNotebookEvent& event ) { event.Skip(); }
		virtual void SDPaintRT( wxPaintEvent& event ) { event.Skip(); }
		virtual void SDPaintGrabbedRT( wxPaintEvent& event ) { event.Skip(); }
		virtual void SDPaintDS( wxPaintEvent& event ) { event.Skip(); }
		virtual void SDPaintProfile( wxPaintEvent& event ) { event.Skip(); }
		virtual void DoStatsToggle( wxCommandEvent& event ) { event.Skip(); }
		virtual void DoStatsSwitch( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		wxShaderDeveloper( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 630,704 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~wxShaderDeveloper();
		
		void SDSpitterShadersOnIdle( wxIdleEvent& )
		{
			SDSpitterShaders->SetSashPosition( 0 );
			SDSpitterShaders->Disconnect( wxEVT_IDLE, wxIdleEventHandler( wxShaderDeveloper::SDSpitterShadersOnIdle ), NULL, this );
		}
		
		void SDSpitterEffectsOnIdle( wxIdleEvent& )
		{
			SDSpitterEffects->SetSashPosition( 0 );
			SDSpitterEffects->Disconnect( wxEVT_IDLE, wxIdleEventHandler( wxShaderDeveloper::SDSpitterEffectsOnIdle ), NULL, this );
		}
		
		void SDSplitterScenesOnIdle( wxIdleEvent& )
		{
			SDSplitterScenes->SetSashPosition( 410 );
			SDSplitterScenes->Disconnect( wxEVT_IDLE, wxIdleEventHandler( wxShaderDeveloper::SDSplitterScenesOnIdle ), NULL, this );
		}
		
		void SDSplitterStatsOnIdle( wxIdleEvent& )
		{
			SDSplitterStats->SetSashPosition( 410 );
			SDSplitterStats->Disconnect( wxEVT_IDLE, wxIdleEventHandler( wxShaderDeveloper::SDSplitterStatsOnIdle ), NULL, this );
		}
	
};

#endif //__GUIs_ShaderDeveloper__

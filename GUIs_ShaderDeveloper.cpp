///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  4 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "GUIs_ShaderDeveloper.h"

///////////////////////////////////////////////////////////////////////////

GUIs_ShaderDeveloper::GUIs_ShaderDeveloper( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_MENU ) );
	
	wxBoxSizer* SDSizer1;
	SDSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxArrayString SDChoicePassChoices;
	SDChoicePass = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, SDChoicePassChoices, 0 );
	SDChoicePass->SetSelection( 0 );
	SDChoicePass->SetToolTip( wxT("Renderpass") );
	
	SDSizer1->Add( SDChoicePass, 0, wxALL|wxEXPAND, 5 );
	
	SDNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	SDPanelShaders = new wxPanel( SDNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* SDSizer2;
	SDSizer2 = new wxBoxSizer( wxVERTICAL );
	
	wxArrayString SDChoiceShaderChoices;
	SDChoiceShader = new wxChoice( SDPanelShaders, wxID_ANY, wxDefaultPosition, wxDefaultSize, SDChoiceShaderChoices, 0 );
	SDChoiceShader->SetSelection( 0 );
	SDChoiceShader->SetToolTip( wxT("Shader") );
	
	SDSizer2->Add( SDChoiceShader, 0, wxALL|wxEXPAND, 5 );
	
	SDToolbarShader = new wxToolBar( SDPanelShaders, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL ); 
	SDButtonShaderLoad = new wxBitmapButton( SDToolbarShader, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	SDToolbarShader->AddControl( SDButtonShaderLoad );
	SDButtonShaderSave = new wxBitmapButton( SDToolbarShader, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	SDToolbarShader->AddControl( SDButtonShaderSave );
	SDButtonShaderSaveAs = new wxBitmapButton( SDToolbarShader, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	SDToolbarShader->AddControl( SDButtonShaderSaveAs );
	SDShaderCompile = new wxButton( SDToolbarShader, wxID_ANY, wxT("Compile"), wxDefaultPosition, wxDefaultSize, 0 );
	SDToolbarShader->AddControl( SDShaderCompile );
	SDToolbarShader->Realize();
	
	SDSizer2->Add( SDToolbarShader, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	SDEditorShader = new wxTextCtrl( SDPanelShaders, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_NOHIDESEL|wxTE_PROCESS_TAB|wxTE_RICH|wxTE_RICH2 );
	SDEditorShader->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	SDSizer2->Add( SDEditorShader, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* SDSizer3;
	SDSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	SDStatusShader = new wxStaticText( SDPanelShaders, wxID_ANY, wxT("Status: compiled, enabled, in use"), wxDefaultPosition, wxDefaultSize, 0 );
	SDStatusShader->Wrap( -1 );
	SDSizer3->Add( SDStatusShader, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	SDShaderEnable = new wxCheckBox( SDPanelShaders, wxID_ANY, wxT("Enabled"), wxPoint( -1,-1 ), wxDefaultSize, wxALIGN_RIGHT );
	SDShaderEnable->SetValue(true); 
	SDSizer3->Add( SDShaderEnable, 0, wxALL, 5 );
	
	SDSizer2->Add( SDSizer3, 0, wxEXPAND|wxLEFT|wxRIGHT, 2 );
	
	SDShaderVariables = new wxNotebook( SDPanelShaders, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	SDShaderMatrices = new wxPanel( SDShaderVariables, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER|wxTAB_TRAVERSAL|wxVSCROLL );
	SDShaderMatrices->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText2 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("WorldMatrix:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer5->Add( m_staticText2, 1, wxALL, 5 );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 4, 4, 0, 0 );
	
	m_staticText3 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	gSizer1->Add( m_staticText3, 0, wxALL, 5 );
	
	m_staticText4 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	gSizer1->Add( m_staticText4, 0, wxALL, 5 );
	
	m_staticText5 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	gSizer1->Add( m_staticText5, 0, wxALL, 5 );
	
	m_staticText6 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	gSizer1->Add( m_staticText6, 0, wxALL, 5 );
	
	m_staticText7 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	gSizer1->Add( m_staticText7, 0, wxALL, 5 );
	
	m_staticText8 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	gSizer1->Add( m_staticText8, 0, wxALL, 5 );
	
	m_staticText9 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	gSizer1->Add( m_staticText9, 0, wxALL, 5 );
	
	m_staticText10 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	gSizer1->Add( m_staticText10, 0, wxALL, 5 );
	
	m_staticText11 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	gSizer1->Add( m_staticText11, 0, wxALL, 5 );
	
	m_staticText12 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	gSizer1->Add( m_staticText12, 0, wxALL, 5 );
	
	m_staticText13 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	gSizer1->Add( m_staticText13, 0, wxALL, 5 );
	
	m_staticText14 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	gSizer1->Add( m_staticText14, 0, wxALL, 5 );
	
	m_staticText15 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText15->Wrap( -1 );
	gSizer1->Add( m_staticText15, 0, wxALL, 5 );
	
	m_staticText16 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	gSizer1->Add( m_staticText16, 0, wxALL, 5 );
	
	m_staticText17 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	gSizer1->Add( m_staticText17, 0, wxALL, 5 );
	
	m_staticText18 = new wxStaticText( SDShaderMatrices, wxID_ANY, wxT("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	gSizer1->Add( m_staticText18, 0, wxALL, 5 );
	
	bSizer5->Add( gSizer1, 0, 0, 5 );
	
	bSizer4->Add( bSizer5, 0, wxEXPAND, 5 );
	
	m_staticline1 = new wxStaticLine( SDShaderMatrices, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer4->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );
	
	SDShaderMatrices->SetSizer( bSizer4 );
	SDShaderMatrices->Layout();
	bSizer4->Fit( SDShaderMatrices );
	SDShaderVariables->AddPage( SDShaderMatrices, wxT("States"), true );
	SDShaderParameters = new wxPanel( SDShaderVariables, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER|wxTAB_TRAVERSAL );
	SDShaderParameters->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	
	SDShaderVariables->AddPage( SDShaderParameters, wxT("Parameters"), false );
	SDShaderInputs = new wxPanel( SDShaderVariables, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER|wxTAB_TRAVERSAL );
	SDShaderInputs->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	
	SDShaderVariables->AddPage( SDShaderInputs, wxT("Inputs"), false );
	SDShaderOutputs = new wxPanel( SDShaderVariables, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	SDShaderOutputs->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	
	SDShaderVariables->AddPage( SDShaderOutputs, wxT("Outputs"), false );
	
	SDSizer2->Add( SDShaderVariables, 1, wxEXPAND | wxALL, 5 );
	
	SDPanelShaders->SetSizer( SDSizer2 );
	SDPanelShaders->Layout();
	SDSizer2->Fit( SDPanelShaders );
	SDNotebook->AddPage( SDPanelShaders, wxT("Shaders"), true );
	SDPanelRendertargets = new wxPanel( SDNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	SDNotebook->AddPage( SDPanelRendertargets, wxT("Rendertargets"), false );
	
	SDSizer1->Add( SDNotebook, 1, wxEXPAND | wxALL, 5 );
	
	this->SetSizer( SDSizer1 );
	this->Layout();
	SDStatusBar = this->CreateStatusBar( 1, wxST_SIZEGRIP|wxNO_BORDER, wxID_ANY );
	SDStatusBar->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_MENU ) );
	
	
	this->Centre( wxBOTH );
}

GUIs_ShaderDeveloper::~GUIs_ShaderDeveloper()
{
}

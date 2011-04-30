///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  4 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "GUIs_ShaderDeveloper.h"

///////////////////////////////////////////////////////////////////////////

wxShaderDeveloper::wxShaderDeveloper( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_MENU ) );
	
	SDMenubar = new wxMenuBar( 0 );
	SDOptions = new wxMenu();
	wxMenuItem* SDCompileSources;
	SDCompileSources = new wxMenuItem( SDOptions, wxID_COMPILE, wxString( wxT("Compile Sources") ) , wxEmptyString, wxITEM_CHECK );
	SDOptions->Append( SDCompileSources );
	
	wxMenuItem* SDSaveBinary;
	SDSaveBinary = new wxMenuItem( SDOptions, wxID_SAVEBIN, wxString( wxT("Save Binary") ) , wxEmptyString, wxITEM_CHECK );
	SDOptions->Append( SDSaveBinary );
	
	wxMenuItem* m_separator1;
	m_separator1 = SDOptions->AppendSeparator();
	
	wxMenuItem* SDLegacy;
	SDLegacy = new wxMenuItem( SDOptions, wxID_LEGACY, wxString( wxT("Legacy Compiler") ) , wxEmptyString, wxITEM_CHECK );
	SDOptions->Append( SDLegacy );
	
	wxMenuItem* SDUpgradeSM1X;
	SDUpgradeSM1X = new wxMenuItem( SDOptions, wxID_UPGRADE, wxString( wxT("Upgrade Legacy SM") ) , wxEmptyString, wxITEM_CHECK );
	SDOptions->Append( SDUpgradeSM1X );
	
	wxMenuItem* SDMaximumSM;
	SDMaximumSM = new wxMenuItem( SDOptions, wxID_MAXIMUM, wxString( wxT("Maximize SM") ) , wxEmptyString, wxITEM_CHECK );
	SDOptions->Append( SDMaximumSM );
	
	wxMenuItem* SDOptimize;
	SDOptimize = new wxMenuItem( SDOptions, wxID_OPTIMIZE, wxString( wxT("Optimize") ) , wxEmptyString, wxITEM_CHECK );
	SDOptions->Append( SDOptimize );
	
	wxMenuItem* m_separator2;
	m_separator2 = SDOptions->AppendSeparator();
	
	wxMenuItem* SDRuntime;
	SDRuntime = new wxMenuItem( SDOptions, wxID_RUNTIME, wxString( wxT("Runtime Replacement") ) , wxEmptyString, wxITEM_CHECK );
	SDOptions->Append( SDRuntime );
	
	SDMenubar->Append( SDOptions, wxT("Options") ); 
	
	this->SetMenuBar( SDMenubar );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxArrayString SDChoicePassChoices;
	SDChoicePass = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, SDChoicePassChoices, 0 );
	SDChoicePass->SetSelection( 0 );
	SDChoicePass->SetToolTip( wxT("Renderpass") );
	
	bSizer1->Add( SDChoicePass, 0, wxALL|wxEXPAND, 5 );
	
	SDViewSwitch = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	SDPanelShaders = new wxPanel( SDViewSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );
	
	SDSpitterShaders = new wxSplitterWindow( SDPanelShaders, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE );
	SDSpitterShaders->SetSashGravity( 1 );
	SDSpitterShaders->Connect( wxEVT_IDLE, wxIdleEventHandler( wxShaderDeveloper::SDSpitterShadersOnIdle ), NULL, this );
	
	SDPanelShadersTop = new wxPanel( SDSpitterShaders, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );
	
	SDComboShader = new wxBitmapComboBox( SDPanelShadersTop, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY|wxCB_SORT ); 
	bSizer111->Add( SDComboShader, 0, wxALL|wxEXPAND, 5 );
	
	SDToolbarShader = new wxToolBar( SDPanelShadersTop, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL ); 
	SDButtonShaderLoad = new wxBitmapButton( SDToolbarShader, wxID_ANY, wxBitmap( wxT("#103"), wxBITMAP_TYPE_RESOURCE ), wxDefaultPosition, wxSize( -1,-1 ), wxBU_AUTODRAW );
	SDButtonShaderLoad->Enable( false );
	
	SDButtonShaderLoad->Enable( false );
	
	SDToolbarShader->AddControl( SDButtonShaderLoad );
	SDButtonShaderSave = new wxBitmapButton( SDToolbarShader, wxID_ANY, wxBitmap( wxT("#104"), wxBITMAP_TYPE_RESOURCE ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	SDToolbarShader->AddControl( SDButtonShaderSave );
	SDButtonShaderSaveAs = new wxBitmapButton( SDToolbarShader, wxID_ANY, wxBitmap( wxT("#105"), wxBITMAP_TYPE_RESOURCE ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	SDButtonShaderSaveAs->Enable( false );
	
	SDButtonShaderSaveAs->Enable( false );
	
	SDToolbarShader->AddControl( SDButtonShaderSaveAs );
	wxString SDShaderVersionChoices[] = { wxT("vs_1_0") };
	int SDShaderVersionNChoices = sizeof( SDShaderVersionChoices ) / sizeof( wxString );
	SDShaderVersion = new wxChoice( SDToolbarShader, wxID_ANY, wxDefaultPosition, wxSize( 70,-1 ), SDShaderVersionNChoices, SDShaderVersionChoices, 0 );
	SDShaderVersion->SetSelection( 0 );
	SDToolbarShader->AddControl( SDShaderVersion );
	SDShaderCompile = new wxButton( SDToolbarShader, wxID_ANY, wxT("Compile"), wxDefaultPosition, wxDefaultSize, 0 );
	SDToolbarShader->AddControl( SDShaderCompile );
	SDToolbarShader->Realize();
	
	bSizer111->Add( SDToolbarShader, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	SDCodeSwitch = new wxNotebook( SDPanelShadersTop, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	SDSource = new wxPanel( SDCodeSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );
	
	SDSourceEditor = new wxTextCtrl( SDSource, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_NOHIDESEL|wxTE_PROCESS_TAB|wxTE_RICH|wxTE_RICH2 );
	SDSourceEditor->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer7->Add( SDSourceEditor, 1, wxALL|wxEXPAND, 5 );
	
	SDSource->SetSizer( bSizer7 );
	SDSource->Layout();
	bSizer7->Fit( SDSource );
	SDCodeSwitch->AddPage( SDSource, wxT("HLSL"), false );
	SDAssembly = new wxPanel( SDCodeSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );
	
	SDAssemblyEditor = new wxTextCtrl( SDAssembly, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_NOHIDESEL|wxTE_PROCESS_TAB|wxTE_RICH|wxTE_RICH2 );
	SDAssemblyEditor->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer8->Add( SDAssemblyEditor, 1, wxALL|wxEXPAND, 5 );
	
	SDAssembly->SetSizer( bSizer8 );
	SDAssembly->Layout();
	bSizer8->Fit( SDAssembly );
	SDCodeSwitch->AddPage( SDAssembly, wxT("Assembly"), false );
	SDErrors = new wxPanel( SDCodeSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );
	
	SDErrorView = new wxTextCtrl( SDErrors, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_NOHIDESEL|wxTE_PROCESS_TAB|wxTE_READONLY|wxTE_RICH|wxTE_RICH2 );
	SDErrorView->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer9->Add( SDErrorView, 1, wxALL|wxEXPAND, 5 );
	
	SDErrors->SetSizer( bSizer9 );
	SDErrors->Layout();
	bSizer9->Fit( SDErrors );
	SDCodeSwitch->AddPage( SDErrors, wxT("Errors and Warnings"), false );
	Disassembly = new wxPanel( SDCodeSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );
	
	SDDisassemblyView = new wxTextCtrl( Disassembly, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_NOHIDESEL|wxTE_PROCESS_TAB|wxTE_READONLY|wxTE_RICH|wxTE_RICH2 );
	SDDisassemblyView->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer10->Add( SDDisassemblyView, 1, wxALL|wxEXPAND, 5 );
	
	Disassembly->SetSizer( bSizer10 );
	Disassembly->Layout();
	bSizer10->Fit( Disassembly );
	SDCodeSwitch->AddPage( Disassembly, wxT("Disassembly"), false );
	
	bSizer111->Add( SDCodeSwitch, 1, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	SDStatusShader = new wxStaticText( SDPanelShadersTop, wxID_ANY, wxT("Status: compiled, enabled, in use"), wxDefaultPosition, wxDefaultSize, 0 );
	SDStatusShader->Wrap( -1 );
	bSizer3->Add( SDStatusShader, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	SDShaderEnable = new wxCheckBox( SDPanelShadersTop, wxID_ANY, wxT("Enabled"), wxPoint( -1,-1 ), wxDefaultSize, wxALIGN_RIGHT|wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER );
	SDShaderEnable->SetValue(true); 
	bSizer3->Add( SDShaderEnable, 0, wxALL, 5 );
	
	bSizer111->Add( bSizer3, 0, wxEXPAND|wxLEFT|wxRIGHT, 2 );
	
	SDPanelShadersTop->SetSizer( bSizer111 );
	SDPanelShadersTop->Layout();
	bSizer111->Fit( SDPanelShadersTop );
	SDPanelShadersBottom = new wxPanel( SDSpitterShaders, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );
	
	SDShaderVariables = new wxNotebook( SDPanelShadersBottom, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	SDShaderConstantTable = new wxPanel( SDShaderVariables, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER|wxTAB_TRAVERSAL );
	SDShaderConstantTable->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxVERTICAL );
	
	SDShaderConstantGrid = new wxGrid( SDShaderConstantTable, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	SDShaderConstantGrid->CreateGrid( 3, 3 );
	SDShaderConstantGrid->EnableEditing( false );
	SDShaderConstantGrid->EnableGridLines( true );
	SDShaderConstantGrid->EnableDragGridSize( true );
	SDShaderConstantGrid->SetMargins( 0, 0 );
	
	// Columns
	SDShaderConstantGrid->AutoSizeColumns();
	SDShaderConstantGrid->EnableDragColMove( false );
	SDShaderConstantGrid->EnableDragColSize( true );
	SDShaderConstantGrid->SetColLabelSize( 20 );
	SDShaderConstantGrid->SetColLabelValue( 0, wxT("Original") );
	SDShaderConstantGrid->SetColLabelValue( 1, wxT("Replaced") );
	SDShaderConstantGrid->SetColLabelValue( 2, wxT("Runtime") );
	SDShaderConstantGrid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_BOTTOM );
	
	// Rows
	SDShaderConstantGrid->AutoSizeRows();
	SDShaderConstantGrid->EnableDragRowSize( false );
	SDShaderConstantGrid->SetRowLabelSize( 80 );
	SDShaderConstantGrid->SetRowLabelAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	
	// Label Appearance
	SDShaderConstantGrid->SetLabelBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDShaderConstantGrid->SetLabelFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 91, false, wxEmptyString ) );
	
	// Cell Defaults
	SDShaderConstantGrid->SetDefaultCellBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDShaderConstantGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	bSizer13->Add( SDShaderConstantGrid, 1, wxALL|wxEXPAND, 0 );
	
	SDShaderConstantTable->SetSizer( bSizer13 );
	SDShaderConstantTable->Layout();
	bSizer13->Fit( SDShaderConstantTable );
	SDShaderVariables->AddPage( SDShaderConstantTable, wxT("Constant-Table"), true );
	SDShaderConstants = new wxPanel( SDShaderVariables, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER|wxTAB_TRAVERSAL );
	SDShaderConstants->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxVERTICAL );
	
	SDShaderConstSetGrid = new wxGrid( SDShaderConstants, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	SDShaderConstSetGrid->CreateGrid( 3, 3 );
	SDShaderConstSetGrid->EnableEditing( false );
	SDShaderConstSetGrid->EnableGridLines( true );
	SDShaderConstSetGrid->EnableDragGridSize( true );
	SDShaderConstSetGrid->SetMargins( 0, 0 );
	
	// Columns
	SDShaderConstSetGrid->AutoSizeColumns();
	SDShaderConstSetGrid->EnableDragColMove( false );
	SDShaderConstSetGrid->EnableDragColSize( true );
	SDShaderConstSetGrid->SetColLabelSize( 20 );
	SDShaderConstSetGrid->SetColLabelValue( 0, wxT("Original") );
	SDShaderConstSetGrid->SetColLabelValue( 1, wxT("Replaced") );
	SDShaderConstSetGrid->SetColLabelValue( 2, wxT("Runtime") );
	SDShaderConstSetGrid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_BOTTOM );
	
	// Rows
	SDShaderConstSetGrid->AutoSizeRows();
	SDShaderConstSetGrid->EnableDragRowSize( false );
	SDShaderConstSetGrid->SetRowLabelSize( 80 );
	SDShaderConstSetGrid->SetRowLabelAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	
	// Label Appearance
	SDShaderConstSetGrid->SetLabelBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDShaderConstSetGrid->SetLabelFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 91, false, wxEmptyString ) );
	
	// Cell Defaults
	SDShaderConstSetGrid->SetDefaultCellBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDShaderConstSetGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	bSizer14->Add( SDShaderConstSetGrid, 1, wxALL|wxEXPAND, 0 );
	
	SDShaderConstants->SetSizer( bSizer14 );
	SDShaderConstants->Layout();
	bSizer14->Fit( SDShaderConstants );
	SDShaderVariables->AddPage( SDShaderConstants, wxT("Constants"), false );
	SDShaderSamplers = new wxPanel( SDShaderVariables, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER|wxTAB_TRAVERSAL );
	SDShaderSamplers->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	
	wxBoxSizer* bSizer131;
	bSizer131 = new wxBoxSizer( wxVERTICAL );
	
	SDShaderSamplerGrid = new wxGrid( SDShaderSamplers, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	SDShaderSamplerGrid->CreateGrid( 2, 2 );
	SDShaderSamplerGrid->EnableEditing( false );
	SDShaderSamplerGrid->EnableGridLines( true );
	SDShaderSamplerGrid->EnableDragGridSize( true );
	SDShaderSamplerGrid->SetMargins( 0, 0 );
	
	// Columns
	SDShaderSamplerGrid->AutoSizeColumns();
	SDShaderSamplerGrid->EnableDragColMove( false );
	SDShaderSamplerGrid->EnableDragColSize( true );
	SDShaderSamplerGrid->SetColLabelSize( 20 );
	SDShaderSamplerGrid->SetColLabelValue( 0, wxT("State") );
	SDShaderSamplerGrid->SetColLabelValue( 1, wxT("Value") );
	SDShaderSamplerGrid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_BOTTOM );
	
	// Rows
	SDShaderSamplerGrid->AutoSizeRows();
	SDShaderSamplerGrid->EnableDragRowSize( false );
	SDShaderSamplerGrid->SetRowLabelSize( 80 );
	SDShaderSamplerGrid->SetRowLabelAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	
	// Label Appearance
	SDShaderSamplerGrid->SetLabelBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDShaderSamplerGrid->SetLabelFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 91, false, wxEmptyString ) );
	
	// Cell Defaults
	SDShaderSamplerGrid->SetDefaultCellBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDShaderSamplerGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	bSizer131->Add( SDShaderSamplerGrid, 1, wxALL|wxEXPAND, 0 );
	
	SDShaderSamplers->SetSizer( bSizer131 );
	SDShaderSamplers->Layout();
	bSizer131->Fit( SDShaderSamplers );
	SDShaderVariables->AddPage( SDShaderSamplers, wxT("Samplers"), false );
	
	bSizer12->Add( SDShaderVariables, 1, wxEXPAND | wxALL, 5 );
	
	SDPanelShadersBottom->SetSizer( bSizer12 );
	SDPanelShadersBottom->Layout();
	bSizer12->Fit( SDPanelShadersBottom );
	SDSpitterShaders->SplitHorizontally( SDPanelShadersTop, SDPanelShadersBottom, 0 );
	bSizer2->Add( SDSpitterShaders, 1, wxEXPAND, 5 );
	
	SDPanelShaders->SetSizer( bSizer2 );
	SDPanelShaders->Layout();
	bSizer2->Fit( SDPanelShaders );
	SDViewSwitch->AddPage( SDPanelShaders, wxT("Shaders"), true );
	SDPanelScenes = new wxPanel( SDViewSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );
	
	SDSplitterScenes = new wxSplitterWindow( SDPanelScenes, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE );
	SDSplitterScenes->SetSashGravity( 1 );
	SDSplitterScenes->Connect( wxEVT_IDLE, wxIdleEventHandler( wxShaderDeveloper::SDSplitterScenesOnIdle ), NULL, this );
	
	SDPanelScenesTop = new wxPanel( SDSplitterScenes, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	wxArrayString SDChoiceSceneChoices;
	SDChoiceScene = new wxChoice( SDPanelScenesTop, wxID_ANY, wxDefaultPosition, wxDefaultSize, SDChoiceSceneChoices, 0 );
	SDChoiceScene->SetSelection( 0 );
	SDChoiceScene->SetToolTip( wxT("Surface Pass") );
	
	bSizer11->Add( SDChoiceScene, 0, wxALL|wxEXPAND, 5 );
	
	SDToolbarRendertarget = new wxToolBar( SDPanelScenesTop, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL ); 
	SDButtonRTCopy = new wxBitmapButton( SDToolbarRendertarget, wxID_ANY, wxBitmap( wxT("#104"), wxBITMAP_TYPE_RESOURCE ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	SDButtonRTCopy->Enable( false );
	
	SDButtonRTCopy->Enable( false );
	
	SDToolbarRendertarget->AddControl( SDButtonRTCopy );
	SDButtonRTSaveAs = new wxBitmapButton( SDToolbarRendertarget, wxID_ANY, wxBitmap( wxT("#105"), wxBITMAP_TYPE_RESOURCE ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	SDButtonRTSaveAs->Enable( false );
	
	SDButtonRTSaveAs->Enable( false );
	
	SDToolbarRendertarget->AddControl( SDButtonRTSaveAs );
	SDToolbarRendertarget->Realize();
	
	bSizer11->Add( SDToolbarRendertarget, 0, wxEXPAND|wxRIGHT|wxLEFT, 5 );
	
	SDSurfaceSwitch = new wxNotebook( SDPanelScenesTop, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	SDRendertarget = new wxPanel( SDSurfaceSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer( wxVERTICAL );
	
	SDRendertargetView = new wxPanel( SDRendertarget, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER );
	bSizer71->Add( SDRendertargetView, 1, wxEXPAND | wxALL, 5 );
	
	SDRendertarget->SetSizer( bSizer71 );
	SDRendertarget->Layout();
	bSizer71->Fit( SDRendertarget );
	SDSurfaceSwitch->AddPage( SDRendertarget, wxT("Primary RT"), true );
	SDDepthStencil = new wxPanel( SDSurfaceSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer81;
	bSizer81 = new wxBoxSizer( wxVERTICAL );
	
	SDDepthStencilView = new wxPanel( SDDepthStencil, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER );
	bSizer81->Add( SDDepthStencilView, 1, wxEXPAND | wxALL, 5 );
	
	SDDepthStencilDepth = new wxSlider( SDDepthStencil, wxID_ANY, 0, 0, 65536, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS|wxSL_TOP );
	bSizer81->Add( SDDepthStencilDepth, 0, wxALL|wxEXPAND, 5 );
	
	SDDepthStencil->SetSizer( bSizer81 );
	SDDepthStencil->Layout();
	bSizer81->Fit( SDDepthStencil );
	SDSurfaceSwitch->AddPage( SDDepthStencil, wxT("Depth / Stencil"), false );
	
	bSizer11->Add( SDSurfaceSwitch, 1, wxEXPAND | wxALL, 5 );
	
	SDStatusRT = new wxStaticText( SDPanelScenesTop, wxID_ANY, wxT("Format: R8G8B8, 100x100"), wxDefaultPosition, wxDefaultSize, 0 );
	SDStatusRT->Wrap( -1 );
	bSizer11->Add( SDStatusRT, 0, wxALL|wxEXPAND, 5 );
	
	SDPanelScenesTop->SetSizer( bSizer11 );
	SDPanelScenesTop->Layout();
	bSizer11->Fit( SDPanelScenesTop );
	SDPanelScenesBottom = new wxPanel( SDSplitterScenes, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxVERTICAL );
	
	SDSceneVariables = new wxNotebook( SDPanelScenesBottom, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	SDSceneStates = new wxPanel( SDSceneVariables, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER|wxTAB_TRAVERSAL );
	SDSceneStates->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	SDSceneStateGrid = new wxGrid( SDSceneStates, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	SDSceneStateGrid->CreateGrid( 2, 2 );
	SDSceneStateGrid->EnableEditing( false );
	SDSceneStateGrid->EnableGridLines( true );
	SDSceneStateGrid->EnableDragGridSize( true );
	SDSceneStateGrid->SetMargins( 0, 0 );
	
	// Columns
	SDSceneStateGrid->AutoSizeColumns();
	SDSceneStateGrid->EnableDragColMove( false );
	SDSceneStateGrid->EnableDragColSize( true );
	SDSceneStateGrid->SetColLabelSize( 20 );
	SDSceneStateGrid->SetColLabelValue( 0, wxT("State") );
	SDSceneStateGrid->SetColLabelValue( 1, wxT("Value") );
	SDSceneStateGrid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_BOTTOM );
	
	// Rows
	SDSceneStateGrid->AutoSizeRows();
	SDSceneStateGrid->EnableDragRowSize( false );
	SDSceneStateGrid->SetRowLabelSize( 80 );
	SDSceneStateGrid->SetRowLabelAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	
	// Label Appearance
	SDSceneStateGrid->SetLabelBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDSceneStateGrid->SetLabelFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 91, false, wxEmptyString ) );
	
	// Cell Defaults
	SDSceneStateGrid->SetDefaultCellBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDSceneStateGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	bSizer4->Add( SDSceneStateGrid, 1, wxALL|wxEXPAND, 0 );
	
	SDSceneStates->SetSizer( bSizer4 );
	SDSceneStates->Layout();
	bSizer4->Fit( SDSceneStates );
	SDSceneVariables->AddPage( SDSceneStates, wxT("States"), false );
	
	bSizer21->Add( SDSceneVariables, 1, wxEXPAND | wxALL, 5 );
	
	SDPanelScenesBottom->SetSizer( bSizer21 );
	SDPanelScenesBottom->Layout();
	bSizer21->Fit( SDPanelScenesBottom );
	SDSplitterScenes->SplitHorizontally( SDPanelScenesTop, SDPanelScenesBottom, 410 );
	bSizer18->Add( SDSplitterScenes, 1, wxEXPAND, 5 );
	
	SDPanelScenes->SetSizer( bSizer18 );
	SDPanelScenes->Layout();
	bSizer18->Fit( SDPanelScenes );
	SDViewSwitch->AddPage( SDPanelScenes, wxT("Scenes"), false );
	
	bSizer1->Add( SDViewSwitch, 1, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( bSizer1 );
	this->Layout();
	SDStatusBar = this->CreateStatusBar( 1, wxST_SIZEGRIP|wxNO_BORDER, wxID_ANY );
	SDStatusBar->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_MENU ) );
	
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_ACTIVATE, wxActivateEventHandler( wxShaderDeveloper::DoActivate ) );
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( wxShaderDeveloper::DoClose ) );
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( wxShaderDeveloper::DoResize ) );
	this->Connect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( wxShaderDeveloper::DoUpdate ) );
	this->Connect( SDCompileSources->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	this->Connect( SDSaveBinary->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	this->Connect( SDLegacy->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	this->Connect( SDUpgradeSM1X->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	this->Connect( SDMaximumSM->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	this->Connect( SDOptimize->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	this->Connect( SDRuntime->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	SDChoicePass->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoRenderpassSwitch ), NULL, this );
	SDViewSwitch->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( wxShaderDeveloper::DoViewSwitch ), NULL, this );
	SDComboShader->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderSwitch ), NULL, this );
	SDButtonShaderLoad->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderLoad ), NULL, this );
	SDButtonShaderSave->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderSave ), NULL, this );
	SDButtonShaderSaveAs->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderSaveAs ), NULL, this );
	SDShaderVersion->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderVersion ), NULL, this );
	SDShaderCompile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderCompile ), NULL, this );
	SDSourceEditor->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxShaderDeveloper::DoShaderUpdate ), NULL, this );
	SDShaderEnable->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderToggle ), NULL, this );
	SDChoiceScene->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoScenesSwitch ), NULL, this );
	SDSurfaceSwitch->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( wxShaderDeveloper::DoSurfaceSwitch ), NULL, this );
	SDRendertargetView->Connect( wxEVT_PAINT, wxPaintEventHandler( wxShaderDeveloper::SDPaintRT ), NULL, this );
	SDDepthStencilView->Connect( wxEVT_PAINT, wxPaintEventHandler( wxShaderDeveloper::SDPaintDS ), NULL, this );
}

wxShaderDeveloper::~wxShaderDeveloper()
{
	// Disconnect Events
	this->Disconnect( wxEVT_ACTIVATE, wxActivateEventHandler( wxShaderDeveloper::DoActivate ) );
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( wxShaderDeveloper::DoClose ) );
	this->Disconnect( wxEVT_SIZE, wxSizeEventHandler( wxShaderDeveloper::DoResize ) );
	this->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( wxShaderDeveloper::DoUpdate ) );
	this->Disconnect( wxID_COMPILE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	this->Disconnect( wxID_SAVEBIN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	this->Disconnect( wxID_LEGACY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	this->Disconnect( wxID_UPGRADE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	this->Disconnect( wxID_MAXIMUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	this->Disconnect( wxID_OPTIMIZE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	this->Disconnect( wxID_RUNTIME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoOptions ) );
	SDChoicePass->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoRenderpassSwitch ), NULL, this );
	SDViewSwitch->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( wxShaderDeveloper::DoViewSwitch ), NULL, this );
	SDComboShader->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderSwitch ), NULL, this );
	SDButtonShaderLoad->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderLoad ), NULL, this );
	SDButtonShaderSave->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderSave ), NULL, this );
	SDButtonShaderSaveAs->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderSaveAs ), NULL, this );
	SDShaderVersion->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderVersion ), NULL, this );
	SDShaderCompile->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderCompile ), NULL, this );
	SDSourceEditor->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxShaderDeveloper::DoShaderUpdate ), NULL, this );
	SDShaderEnable->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderToggle ), NULL, this );
	SDChoiceScene->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoScenesSwitch ), NULL, this );
	SDSurfaceSwitch->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( wxShaderDeveloper::DoSurfaceSwitch ), NULL, this );
	SDRendertargetView->Disconnect( wxEVT_PAINT, wxPaintEventHandler( wxShaderDeveloper::SDPaintRT ), NULL, this );
	SDDepthStencilView->Disconnect( wxEVT_PAINT, wxPaintEventHandler( wxShaderDeveloper::SDPaintDS ), NULL, this );
	
}

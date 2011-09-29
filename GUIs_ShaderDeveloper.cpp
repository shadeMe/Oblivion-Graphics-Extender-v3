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
	SDShaderOptions = new wxMenu();
	wxMenuItem* SDShaderCompileSources;
	SDShaderCompileSources = new wxMenuItem( SDShaderOptions, wxID_SCOMPILE, wxString( wxT("Compile Sources") ) , wxEmptyString, wxITEM_CHECK );
	SDShaderOptions->Append( SDShaderCompileSources );
	
	wxMenuItem* SDShaderSaveBinary;
	SDShaderSaveBinary = new wxMenuItem( SDShaderOptions, wxID_SSAVEBIN, wxString( wxT("Save Binary") ) , wxEmptyString, wxITEM_CHECK );
	SDShaderOptions->Append( SDShaderSaveBinary );
	
	wxMenuItem* m_separator1;
	m_separator1 = SDShaderOptions->AppendSeparator();
	
	wxMenuItem* SDShaderLegacy;
	SDShaderLegacy = new wxMenuItem( SDShaderOptions, wxID_SLEGACY, wxString( wxT("Legacy Compiler") ) , wxEmptyString, wxITEM_CHECK );
	SDShaderOptions->Append( SDShaderLegacy );
	
	wxMenuItem* SDShaderUpgradeSM1X;
	SDShaderUpgradeSM1X = new wxMenuItem( SDShaderOptions, wxID_SUPGRADE, wxString( wxT("Upgrade Legacy SM") ) , wxEmptyString, wxITEM_CHECK );
	SDShaderOptions->Append( SDShaderUpgradeSM1X );
	
	wxMenuItem* SDShaderMaximumSM;
	SDShaderMaximumSM = new wxMenuItem( SDShaderOptions, wxID_SMAXIMUM, wxString( wxT("Maximize SM") ) , wxEmptyString, wxITEM_CHECK );
	SDShaderOptions->Append( SDShaderMaximumSM );
	
	wxMenuItem* SDShaderOptimize;
	SDShaderOptimize = new wxMenuItem( SDShaderOptions, wxID_SOPTIMIZE, wxString( wxT("Optimize") ) , wxEmptyString, wxITEM_CHECK );
	SDShaderOptions->Append( SDShaderOptimize );
	
	wxMenuItem* m_separator2;
	m_separator2 = SDShaderOptions->AppendSeparator();
	
	wxMenuItem* SDShaderRuntime;
	SDShaderRuntime = new wxMenuItem( SDShaderOptions, wxID_SRUNTIME, wxString( wxT("Runtime Replacement") ) , wxEmptyString, wxITEM_CHECK );
	SDShaderOptions->Append( SDShaderRuntime );
	
	SDMenubar->Append( SDShaderOptions, wxT("Shaders") ); 
	
	SDEffectOptions = new wxMenu();
	wxMenuItem* SDEffectCompileSources;
	SDEffectCompileSources = new wxMenuItem( SDEffectOptions, wxID_ECOMPILE, wxString( wxT("Compile Sources") ) , wxEmptyString, wxITEM_CHECK );
	SDEffectOptions->Append( SDEffectCompileSources );
	SDEffectCompileSources->Enable( false );
	
	wxMenuItem* SDEffectSaveBinary;
	SDEffectSaveBinary = new wxMenuItem( SDEffectOptions, wxID_ESAVEBIN, wxString( wxT("Save Binary") ) , wxEmptyString, wxITEM_CHECK );
	SDEffectOptions->Append( SDEffectSaveBinary );
	SDEffectSaveBinary->Enable( false );
	
	wxMenuItem* m_separator11;
	m_separator11 = SDEffectOptions->AppendSeparator();
	
	wxMenuItem* SDEffectLegacy;
	SDEffectLegacy = new wxMenuItem( SDEffectOptions, wxID_ELEGACY, wxString( wxT("Legacy Compiler") ) , wxEmptyString, wxITEM_CHECK );
	SDEffectOptions->Append( SDEffectLegacy );
	
	wxMenuItem* SDEffectOptimize;
	SDEffectOptimize = new wxMenuItem( SDEffectOptions, wxID_EOPTIMIZE, wxString( wxT("Optimize") ) , wxEmptyString, wxITEM_CHECK );
	SDEffectOptions->Append( SDEffectOptimize );
	
	SDMenubar->Append( SDEffectOptions, wxT("Effects") ); 
	
	SDProfileOptions = new wxMenu();
	wxMenuItem* SDProfileGPU;
	SDProfileGPU = new wxMenuItem( SDProfileOptions, wxID_PROFILE, wxString( wxT("Profile GPU") ) , wxEmptyString, wxITEM_CHECK );
	SDProfileOptions->Append( SDProfileGPU );
	SDProfileGPU->Enable( false );
	
	wxMenuItem* SDProfileTex;
	SDProfileTex = new wxMenuItem( SDProfileOptions, wxID_KILLTEX, wxString( wxT("Remove Texture-accesses") ) , wxEmptyString, wxITEM_CHECK );
	SDProfileOptions->Append( SDProfileTex );
	SDProfileTex->Enable( false );
	
	wxMenuItem* m_separator111;
	m_separator111 = SDProfileOptions->AppendSeparator();
	
	wxMenuItem* SDModeWireframe;
	SDModeWireframe = new wxMenuItem( SDProfileOptions, wxID_WIREFRAME, wxString( wxT("Render wireframe") ) , wxEmptyString, wxITEM_CHECK );
	SDProfileOptions->Append( SDModeWireframe );
	SDModeWireframe->Enable( false );
	
	wxMenuItem* SDModeTesselation;
	SDModeTesselation = new wxMenuItem( SDProfileOptions, wxID_TESSELATION, wxString( wxT("Render tesselations") ) , wxEmptyString, wxITEM_CHECK );
	SDProfileOptions->Append( SDModeTesselation );
	SDModeTesselation->Enable( false );
	
	SDMenubar->Append( SDProfileOptions, wxT("Profiling") ); 
	
	SDTools = new wxMenu();
	SDToolsSettings = new wxMenu();
	wxMenuItem* SDToolsSettingsAmplify;
	SDToolsSettingsAmplify = new wxMenuItem( SDToolsSettings, wxID_AMPLIFY, wxString( wxT("Angle amplification over mip-maps for NM") ) , wxEmptyString, wxITEM_CHECK );
	SDToolsSettings->Append( SDToolsSettingsAmplify );
	
	wxMenuItem* SDToolsSettingsGamma;
	SDToolsSettingsGamma = new wxMenuItem( SDToolsSettings, wxID_MIPGAMMA, wxString( wxT("Gamma correction for MIPs") ) , wxEmptyString, wxITEM_CHECK );
	SDToolsSettings->Append( SDToolsSettingsGamma );
	
	wxMenuItem* SDToolsSettingsBatch;
	SDToolsSettingsBatch = new wxMenuItem( SDToolsSettings, wxID_BATCH, wxString( wxT("Do batch operations") ) , wxEmptyString, wxITEM_CHECK );
	SDToolsSettings->Append( SDToolsSettingsBatch );
	
	SDTools->Append( -1, wxT("Settings"), SDToolsSettings );
	
	wxMenuItem* SDConvertQDMy;
	SDConvertQDMy = new wxMenuItem( SDTools, wxID_ANY, wxString( wxT("Convert PM to QDM (destroy specular)") ) , wxEmptyString, wxITEM_NORMAL );
	SDTools->Append( SDConvertQDMy );
	
	wxMenuItem* SDConvertQDMn;
	SDConvertQDMn = new wxMenuItem( SDTools, wxID_ANY, wxString( wxT("Convert PM to QDM (preserve specular)") ) , wxEmptyString, wxITEM_NORMAL );
	SDTools->Append( SDConvertQDMn );
	
	wxMenuItem* SDConvertRGBH;
	SDConvertRGBH = new wxMenuItem( SDTools, wxID_ANY, wxString( wxT("Convert / Re-mip color+height-map [rgbh]") ) , wxEmptyString, wxITEM_NORMAL );
	SDTools->Append( SDConvertRGBH );
	
	wxMenuItem* SDConvertRGBA;
	SDConvertRGBA = new wxMenuItem( SDTools, wxID_ANY, wxString( wxT("Convert / Re-mip color+alpha [rgba]") ) , wxEmptyString, wxITEM_NORMAL );
	SDTools->Append( SDConvertRGBA );
	
	wxMenuItem* SDConvertRGB;
	SDConvertRGB = new wxMenuItem( SDTools, wxID_ANY, wxString( wxT("Convert / Re-mip color-map [rgb-]") ) , wxEmptyString, wxITEM_NORMAL );
	SDTools->Append( SDConvertRGB );
	
	wxMenuItem* SDConvertLA;
	SDConvertLA = new wxMenuItem( SDTools, wxID_ANY, wxString( wxT("Convert / Re-mip luminance+alpha-map [--la]") ) , wxEmptyString, wxITEM_NORMAL );
	SDTools->Append( SDConvertLA );
	
	wxMenuItem* SDConvertA;
	SDConvertA = new wxMenuItem( SDTools, wxID_ANY, wxString( wxT("Convert / Re-mip alpha-map [---a]") ) , wxEmptyString, wxITEM_NORMAL );
	SDTools->Append( SDConvertA );
	
	wxMenuItem* SDConvertXYZD;
	SDConvertXYZD = new wxMenuItem( SDTools, wxID_ANY, wxString( wxT("Convert / Re-mip normal+specular-map [xyzs]") ) , wxEmptyString, wxITEM_NORMAL );
	SDTools->Append( SDConvertXYZD );
	
	wxMenuItem* SDConvertXYZ;
	SDConvertXYZ = new wxMenuItem( SDTools, wxID_ANY, wxString( wxT("Convert / Re-mip normal-map [xyz-]") ) , wxEmptyString, wxITEM_NORMAL );
	SDTools->Append( SDConvertXYZ );
	
	wxMenuItem* SDConvertXY_Z;
	SDConvertXY_Z = new wxMenuItem( SDTools, wxID_ANY, wxString( wxT("Convert / Re-mip normal-map [xy+z-]") ) , wxEmptyString, wxITEM_NORMAL );
	SDTools->Append( SDConvertXY_Z );
	
	wxMenuItem* SDConvertXY;
	SDConvertXY = new wxMenuItem( SDTools, wxID_ANY, wxString( wxT("Convert / Re-mip normal-map [xy--]") ) , wxEmptyString, wxITEM_NORMAL );
	SDTools->Append( SDConvertXY );
	
	SDMenubar->Append( SDTools, wxT("Tools") ); 
	
	SDTweaks = new wxMenu();
	SDTweaksAF = new wxMenu();
	wxMenuItem* SDTweakAF1;
	SDTweakAF1 = new wxMenuItem( SDTweaksAF, wxID_AF1, wxString( wxT("1") ) , wxEmptyString, wxITEM_RADIO );
	SDTweaksAF->Append( SDTweakAF1 );
	
	wxMenuItem* SDTweakAF2;
	SDTweakAF2 = new wxMenuItem( SDTweaksAF, wxID_AF2, wxString( wxT("2") ) , wxEmptyString, wxITEM_RADIO );
	SDTweaksAF->Append( SDTweakAF2 );
	
	wxMenuItem* SDTweakAF4;
	SDTweakAF4 = new wxMenuItem( SDTweaksAF, wxID_AF4, wxString( wxT("4") ) , wxEmptyString, wxITEM_RADIO );
	SDTweaksAF->Append( SDTweakAF4 );
	
	wxMenuItem* SDTweakAF8;
	SDTweakAF8 = new wxMenuItem( SDTweaksAF, wxID_AF8, wxString( wxT("8") ) , wxEmptyString, wxITEM_RADIO );
	SDTweaksAF->Append( SDTweakAF8 );
	
	wxMenuItem* SDTweakAF16;
	SDTweakAF16 = new wxMenuItem( SDTweaksAF, wxID_AF16, wxString( wxT("16") ) , wxEmptyString, wxITEM_RADIO );
	SDTweaksAF->Append( SDTweakAF16 );
	
	wxMenuItem* SDTweakAF32;
	SDTweakAF32 = new wxMenuItem( SDTweaksAF, wxID_AF32, wxString( wxT("32") ) , wxEmptyString, wxITEM_RADIO );
	SDTweaksAF->Append( SDTweakAF32 );
	
	SDTweaks->Append( -1, wxT("Anisotropy"), SDTweaksAF );
	
	wxMenuItem* SDTweaksGamma;
	SDTweaksGamma = new wxMenuItem( SDTweaks, wxID_LINEAR, wxString( wxT("Linear colorspace") ) , wxEmptyString, wxITEM_CHECK );
	SDTweaks->Append( SDTweaksGamma );
	
	SDMenubar->Append( SDTweaks, wxT("Tweaks") ); 
	
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
	SDShaderCompileAll = new wxButton( SDToolbarShader, wxID_ANY, wxT("Compile all failed"), wxDefaultPosition, wxDefaultSize, 0 );
	SDToolbarShader->AddControl( SDShaderCompileAll );
	SDShaderFlush = new wxButton( SDToolbarShader, wxID_ANY, wxT("Flush from disk"), wxDefaultPosition, wxDefaultSize, 0 );
	SDToolbarShader->AddControl( SDShaderFlush );
	SDToolbarShader->Realize();
	
	bSizer111->Add( SDToolbarShader, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	SDShaderCodeSwitch = new wxNotebook( SDPanelShadersTop, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	SDShaderSource = new wxPanel( SDShaderCodeSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );
	
	SDShaderSourceEditor = new wxTextCtrl( SDShaderSource, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP|wxTE_MULTILINE|wxTE_NOHIDESEL|wxTE_PROCESS_TAB|wxTE_RICH|wxTE_RICH2 );
	SDShaderSourceEditor->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer7->Add( SDShaderSourceEditor, 1, wxALL|wxEXPAND, 5 );
	
	SDShaderSource->SetSizer( bSizer7 );
	SDShaderSource->Layout();
	bSizer7->Fit( SDShaderSource );
	SDShaderCodeSwitch->AddPage( SDShaderSource, wxT("HLSL"), true );
	SDShaderAssembly = new wxPanel( SDShaderCodeSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );
	
	SDShaderAssemblyEditor = new wxTextCtrl( SDShaderAssembly, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP|wxTE_MULTILINE|wxTE_NOHIDESEL|wxTE_PROCESS_TAB|wxTE_RICH|wxTE_RICH2 );
	SDShaderAssemblyEditor->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer8->Add( SDShaderAssemblyEditor, 1, wxALL|wxEXPAND, 5 );
	
	SDShaderAssembly->SetSizer( bSizer8 );
	SDShaderAssembly->Layout();
	bSizer8->Fit( SDShaderAssembly );
	SDShaderCodeSwitch->AddPage( SDShaderAssembly, wxT("Assembly"), false );
	SDShaderErrors = new wxPanel( SDShaderCodeSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );
	
	SDShaderErrorView = new wxTextCtrl( SDShaderErrors, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP|wxTE_MULTILINE|wxTE_NOHIDESEL|wxTE_PROCESS_TAB|wxTE_READONLY|wxTE_RICH|wxTE_RICH2 );
	SDShaderErrorView->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer9->Add( SDShaderErrorView, 1, wxALL|wxEXPAND, 5 );
	
	SDShaderErrors->SetSizer( bSizer9 );
	SDShaderErrors->Layout();
	bSizer9->Fit( SDShaderErrors );
	SDShaderCodeSwitch->AddPage( SDShaderErrors, wxT("Errors and Warnings"), false );
	SDShaderDisassembly = new wxPanel( SDShaderCodeSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxVERTICAL );
	
	SDShaderDisassemblyView = new wxTextCtrl( SDShaderDisassembly, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP|wxTE_MULTILINE|wxTE_NOHIDESEL|wxTE_PROCESS_TAB|wxTE_READONLY|wxTE_RICH|wxTE_RICH2 );
	SDShaderDisassemblyView->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer10->Add( SDShaderDisassemblyView, 1, wxALL|wxEXPAND, 5 );
	
	SDShaderDisassembly->SetSizer( bSizer10 );
	SDShaderDisassembly->Layout();
	bSizer10->Fit( SDShaderDisassembly );
	SDShaderCodeSwitch->AddPage( SDShaderDisassembly, wxT("Disassembly"), false );
	
	bSizer111->Add( SDShaderCodeSwitch, 1, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );
	
	SDStatusShader = new wxStaticText( SDPanelShadersTop, wxID_ANY, wxT("Status: compiled, enabled, in use"), wxDefaultPosition, wxDefaultSize, 0 );
	SDStatusShader->Wrap( -1 );
	bSizer3->Add( SDStatusShader, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	SDShaderEnable = new wxCheckBox( SDPanelShadersTop, wxID_ANY, wxT("Enabled"), wxPoint( -1,-1 ), wxDefaultSize, wxALIGN_RIGHT|wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER );
	SDShaderEnable->SetValue(true); 
	bSizer3->Add( SDShaderEnable, 0, wxALL, 5 );
	
	bSizer111->Add( bSizer3, 0, wxEXPAND|wxLEFT|wxRIGHT, 2 );
	
	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxHORIZONTAL );
	
	SDShaderPairing = new wxStaticText( SDPanelShadersTop, wxID_ANY, wxT("Pair: ..."), wxDefaultPosition, wxDefaultSize, 0 );
	SDShaderPairing->Wrap( -1 );
	bSizer34->Add( SDShaderPairing, 1, wxALL, 5 );
	
	SDShaderMark = new wxCheckBox( SDPanelShadersTop, wxID_ANY, wxT("Mark"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	bSizer34->Add( SDShaderMark, 0, wxALL, 5 );
	
	bSizer111->Add( bSizer34, 0, wxRIGHT|wxLEFT|wxEXPAND, 2 );
	
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
	SDShaderConstSetGrid->EnableEditing( true );
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
	SDShaderSamplerGrid->EnableEditing( true );
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
	SDPanelEffects = new wxPanel( SDViewSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxVERTICAL );
	
	SDSpitterEffects = new wxSplitterWindow( SDPanelEffects, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE );
	SDSpitterEffects->SetSashGravity( 1 );
	SDSpitterEffects->Connect( wxEVT_IDLE, wxIdleEventHandler( wxShaderDeveloper::SDSpitterEffectsOnIdle ), NULL, this );
	
	SDPanelEffectsTop = new wxPanel( SDSpitterEffects, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer1111;
	bSizer1111 = new wxBoxSizer( wxVERTICAL );
	
	SDComboEffect = new wxBitmapComboBox( SDPanelEffectsTop, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY|wxCB_SORT ); 
	bSizer1111->Add( SDComboEffect, 0, wxALL|wxEXPAND, 5 );
	
	SDToolbarEffect = new wxToolBar( SDPanelEffectsTop, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL ); 
	SDButtonEffectNew = new wxBitmapButton( SDToolbarEffect, wxID_ANY, wxBitmap( wxT("#112"), wxBITMAP_TYPE_RESOURCE ), wxDefaultPosition, wxSize( -1,-1 ), wxBU_AUTODRAW );
	SDButtonEffectNew->Enable( false );
	
	SDButtonEffectNew->Enable( false );
	
	SDToolbarEffect->AddControl( SDButtonEffectNew );
	SDButtonEffectLoad = new wxBitmapButton( SDToolbarEffect, wxID_ANY, wxBitmap( wxT("#103"), wxBITMAP_TYPE_RESOURCE ), wxDefaultPosition, wxSize( -1,-1 ), wxBU_AUTODRAW );
	SDButtonEffectLoad->Enable( false );
	
	SDButtonEffectLoad->Enable( false );
	
	SDToolbarEffect->AddControl( SDButtonEffectLoad );
	SDButtonEffectSave = new wxBitmapButton( SDToolbarEffect, wxID_ANY, wxBitmap( wxT("#104"), wxBITMAP_TYPE_RESOURCE ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	SDToolbarEffect->AddControl( SDButtonEffectSave );
	SDButtonEffectSaveAs = new wxBitmapButton( SDToolbarEffect, wxID_ANY, wxBitmap( wxT("#105"), wxBITMAP_TYPE_RESOURCE ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	SDButtonEffectSaveAs->Enable( false );
	
	SDButtonEffectSaveAs->Enable( false );
	
	SDToolbarEffect->AddControl( SDButtonEffectSaveAs );
	SDEffectCompile = new wxButton( SDToolbarEffect, wxID_ANY, wxT("Compile"), wxDefaultPosition, wxDefaultSize, 0 );
	SDToolbarEffect->AddControl( SDEffectCompile );
	SDToolbarEffect->Realize();
	
	bSizer1111->Add( SDToolbarEffect, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	SDEffectCodeSwitch = new wxNotebook( SDPanelEffectsTop, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	SDEffectSource = new wxPanel( SDEffectCodeSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer72;
	bSizer72 = new wxBoxSizer( wxVERTICAL );
	
	SDEffectSourceEditor = new wxTextCtrl( SDEffectSource, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP|wxTE_MULTILINE|wxTE_NOHIDESEL|wxTE_PROCESS_TAB|wxTE_RICH|wxTE_RICH2 );
	SDEffectSourceEditor->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer72->Add( SDEffectSourceEditor, 1, wxALL|wxEXPAND, 5 );
	
	SDEffectSource->SetSizer( bSizer72 );
	SDEffectSource->Layout();
	bSizer72->Fit( SDEffectSource );
	SDEffectCodeSwitch->AddPage( SDEffectSource, wxT("FX"), true );
	SDEffectErrors = new wxPanel( SDEffectCodeSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer91;
	bSizer91 = new wxBoxSizer( wxVERTICAL );
	
	SDEffectErrorView = new wxTextCtrl( SDEffectErrors, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP|wxTE_MULTILINE|wxTE_NOHIDESEL|wxTE_PROCESS_TAB|wxTE_READONLY|wxTE_RICH|wxTE_RICH2 );
	SDEffectErrorView->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer91->Add( SDEffectErrorView, 1, wxALL|wxEXPAND, 5 );
	
	SDEffectErrors->SetSizer( bSizer91 );
	SDEffectErrors->Layout();
	bSizer91->Fit( SDEffectErrors );
	SDEffectCodeSwitch->AddPage( SDEffectErrors, wxT("Errors and Warnings"), false );
	SDEffectDisassembly = new wxPanel( SDEffectCodeSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer101;
	bSizer101 = new wxBoxSizer( wxVERTICAL );
	
	SDEffectDisassemblyView = new wxTextCtrl( SDEffectDisassembly, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_DONTWRAP|wxTE_MULTILINE|wxTE_NOHIDESEL|wxTE_PROCESS_TAB|wxTE_READONLY|wxTE_RICH|wxTE_RICH2 );
	SDEffectDisassemblyView->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer101->Add( SDEffectDisassemblyView, 1, wxALL|wxEXPAND, 5 );
	
	SDEffectDisassembly->SetSizer( bSizer101 );
	SDEffectDisassembly->Layout();
	bSizer101->Fit( SDEffectDisassembly );
	SDEffectCodeSwitch->AddPage( SDEffectDisassembly, wxT("Disassembly"), false );
	
	bSizer1111->Add( SDEffectCodeSwitch, 1, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxHORIZONTAL );
	
	SDStatusEffect = new wxStaticText( SDPanelEffectsTop, wxID_ANY, wxT("Status: compiled, enabled, in use"), wxDefaultPosition, wxDefaultSize, 0 );
	SDStatusEffect->Wrap( -1 );
	bSizer31->Add( SDStatusEffect, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	SDEffectEnable = new wxCheckBox( SDPanelEffectsTop, wxID_ANY, wxT("Enabled"), wxPoint( -1,-1 ), wxDefaultSize, wxALIGN_RIGHT );
	SDEffectEnable->SetValue(true); 
	bSizer31->Add( SDEffectEnable, 0, wxALL, 5 );
	
	bSizer1111->Add( bSizer31, 0, wxEXPAND|wxLEFT|wxRIGHT, 2 );
	
	SDPanelEffectsTop->SetSizer( bSizer1111 );
	SDPanelEffectsTop->Layout();
	bSizer1111->Fit( SDPanelEffectsTop );
	SDPanelEffectsBottom = new wxPanel( SDSpitterEffects, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer( wxVERTICAL );
	
	SDEffectVariables = new wxNotebook( SDPanelEffectsBottom, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	SDEffectConstants = new wxPanel( SDEffectVariables, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER|wxTAB_TRAVERSAL );
	SDEffectConstants->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	
	wxBoxSizer* bSizer141;
	bSizer141 = new wxBoxSizer( wxVERTICAL );
	
	SDEffectConstSetGrid = new wxGrid( SDEffectConstants, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	SDEffectConstSetGrid->CreateGrid( 3, 1 );
	SDEffectConstSetGrid->EnableEditing( true );
	SDEffectConstSetGrid->EnableGridLines( true );
	SDEffectConstSetGrid->EnableDragGridSize( true );
	SDEffectConstSetGrid->SetMargins( 0, 0 );
	
	// Columns
	SDEffectConstSetGrid->AutoSizeColumns();
	SDEffectConstSetGrid->EnableDragColMove( false );
	SDEffectConstSetGrid->EnableDragColSize( true );
	SDEffectConstSetGrid->SetColLabelSize( 20 );
	SDEffectConstSetGrid->SetColLabelValue( 0, wxT("Value") );
	SDEffectConstSetGrid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_BOTTOM );
	
	// Rows
	SDEffectConstSetGrid->AutoSizeRows();
	SDEffectConstSetGrid->EnableDragRowSize( false );
	SDEffectConstSetGrid->SetRowLabelSize( 80 );
	SDEffectConstSetGrid->SetRowLabelAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	
	// Label Appearance
	SDEffectConstSetGrid->SetLabelBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDEffectConstSetGrid->SetLabelFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 91, false, wxEmptyString ) );
	
	// Cell Defaults
	SDEffectConstSetGrid->SetDefaultCellBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDEffectConstSetGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	bSizer141->Add( SDEffectConstSetGrid, 1, wxALL|wxEXPAND, 0 );
	
	SDEffectConstants->SetSizer( bSizer141 );
	SDEffectConstants->Layout();
	bSizer141->Fit( SDEffectConstants );
	SDEffectVariables->AddPage( SDEffectConstants, wxT("Variables"), true );
	SDEffectTextures = new wxPanel( SDEffectVariables, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER|wxTAB_TRAVERSAL );
	SDEffectTextures->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	
	wxBoxSizer* bSizer1311;
	bSizer1311 = new wxBoxSizer( wxVERTICAL );
	
	SDEffectTexturesGrid = new wxGrid( SDEffectTextures, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	SDEffectTexturesGrid->CreateGrid( 2, 1 );
	SDEffectTexturesGrid->EnableEditing( true );
	SDEffectTexturesGrid->EnableGridLines( true );
	SDEffectTexturesGrid->EnableDragGridSize( true );
	SDEffectTexturesGrid->SetMargins( 0, 0 );
	
	// Columns
	SDEffectTexturesGrid->AutoSizeColumns();
	SDEffectTexturesGrid->EnableDragColMove( false );
	SDEffectTexturesGrid->EnableDragColSize( true );
	SDEffectTexturesGrid->SetColLabelSize( 20 );
	SDEffectTexturesGrid->SetColLabelValue( 0, wxT("Value") );
	SDEffectTexturesGrid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_BOTTOM );
	
	// Rows
	SDEffectTexturesGrid->AutoSizeRows();
	SDEffectTexturesGrid->EnableDragRowSize( false );
	SDEffectTexturesGrid->SetRowLabelSize( 80 );
	SDEffectTexturesGrid->SetRowLabelAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	
	// Label Appearance
	SDEffectTexturesGrid->SetLabelBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDEffectTexturesGrid->SetLabelFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 91, false, wxEmptyString ) );
	
	// Cell Defaults
	SDEffectTexturesGrid->SetDefaultCellBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDEffectTexturesGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	bSizer1311->Add( SDEffectTexturesGrid, 1, wxALL|wxEXPAND, 0 );
	
	SDEffectTextures->SetSizer( bSizer1311 );
	SDEffectTextures->Layout();
	bSizer1311->Fit( SDEffectTextures );
	SDEffectVariables->AddPage( SDEffectTextures, wxT("Textures"), false );
	
	bSizer121->Add( SDEffectVariables, 1, wxEXPAND | wxALL, 5 );
	
	SDPanelEffectsBottom->SetSizer( bSizer121 );
	SDPanelEffectsBottom->Layout();
	bSizer121->Fit( SDPanelEffectsBottom );
	SDSpitterEffects->SplitHorizontally( SDPanelEffectsTop, SDPanelEffectsBottom, 0 );
	bSizer22->Add( SDSpitterEffects, 1, wxEXPAND, 5 );
	
	SDPanelEffects->SetSizer( bSizer22 );
	SDPanelEffects->Layout();
	bSizer22->Fit( SDPanelEffects );
	SDViewSwitch->AddPage( SDPanelEffects, wxT("Effects"), false );
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
	SDRendertargetGrabbed = new wxPanel( SDSurfaceSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer711;
	bSizer711 = new wxBoxSizer( wxVERTICAL );
	
	SDRendertargetGrabbedView = new wxPanel( SDRendertargetGrabbed, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER );
	bSizer711->Add( SDRendertargetGrabbedView, 1, wxEXPAND | wxALL, 5 );
	
	SDRendertargetGrabbed->SetSizer( bSizer711 );
	SDRendertargetGrabbed->Layout();
	bSizer711->Fit( SDRendertargetGrabbed );
	SDSurfaceSwitch->AddPage( SDRendertargetGrabbed, wxT("Grabbed RT"), false );
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
	SDPanelStats = new wxPanel( SDViewSwitch, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer181;
	bSizer181 = new wxBoxSizer( wxVERTICAL );
	
	SDSplitterStats = new wxSplitterWindow( SDPanelStats, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE );
	SDSplitterStats->SetSashGravity( 1 );
	SDSplitterStats->Connect( wxEVT_IDLE, wxIdleEventHandler( wxShaderDeveloper::SDSplitterStatsOnIdle ), NULL, this );
	
	SDPanelStatsTop = new wxPanel( SDSplitterStats, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer112;
	bSizer112 = new wxBoxSizer( wxVERTICAL );
	
	SDStatsView = new wxPanel( SDPanelStatsTop, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER );
	bSizer112->Add( SDStatsView, 1, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxHORIZONTAL );
	
	SDStatusStats = new wxStaticText( SDPanelStatsTop, wxID_ANY, wxT("Global: 12.4 FPS, 35% frame-time"), wxDefaultPosition, wxDefaultSize, 0 );
	SDStatusStats->Wrap( -1 );
	bSizer32->Add( SDStatusStats, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	SDStatsNormalize = new wxCheckBox( SDPanelStatsTop, wxID_ANY, wxT("Normalize"), wxPoint( -1,-1 ), wxDefaultSize, wxALIGN_RIGHT );
	bSizer32->Add( SDStatsNormalize, 0, wxALL, 5 );
	
	bSizer112->Add( bSizer32, 0, wxEXPAND|wxLEFT|wxRIGHT, 2 );
	
	SDPanelStatsTop->SetSizer( bSizer112 );
	SDPanelStatsTop->Layout();
	bSizer112->Fit( SDPanelStatsTop );
	SDPanelStatsBottom = new wxPanel( SDSplitterStats, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer211;
	bSizer211 = new wxBoxSizer( wxVERTICAL );
	
	wxArrayString SDChoiceStatsChoices;
	SDChoiceStats = new wxChoice( SDPanelStatsBottom, wxID_ANY, wxDefaultPosition, wxDefaultSize, SDChoiceStatsChoices, 0 );
	SDChoiceStats->SetSelection( 0 );
	SDChoiceStats->SetToolTip( wxT("Surface Pass") );
	
	bSizer211->Add( SDChoiceStats, 0, wxALL|wxEXPAND, 5 );
	
	SDStatsVariables = new wxNotebook( SDPanelStatsBottom, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	SDSceneStats = new wxPanel( SDStatsVariables, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER|wxTAB_TRAVERSAL );
	SDSceneStats->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	
	wxBoxSizer* bSizer41;
	bSizer41 = new wxBoxSizer( wxVERTICAL );
	
	SDSceneStatsGrid = new wxGrid( SDSceneStats, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	
	// Grid
	SDSceneStatsGrid->CreateGrid( 2, 1 );
	SDSceneStatsGrid->EnableEditing( false );
	SDSceneStatsGrid->EnableGridLines( true );
	SDSceneStatsGrid->EnableDragGridSize( true );
	SDSceneStatsGrid->SetMargins( 0, 0 );
	
	// Columns
	SDSceneStatsGrid->AutoSizeColumns();
	SDSceneStatsGrid->EnableDragColMove( false );
	SDSceneStatsGrid->EnableDragColSize( true );
	SDSceneStatsGrid->SetColLabelSize( 20 );
	SDSceneStatsGrid->SetColLabelValue( 0, wxT("Value") );
	SDSceneStatsGrid->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_BOTTOM );
	
	// Rows
	SDSceneStatsGrid->AutoSizeRows();
	SDSceneStatsGrid->EnableDragRowSize( false );
	SDSceneStatsGrid->SetRowLabelSize( 80 );
	SDSceneStatsGrid->SetRowLabelAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	
	// Label Appearance
	SDSceneStatsGrid->SetLabelBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDSceneStatsGrid->SetLabelFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 91, false, wxEmptyString ) );
	
	// Cell Defaults
	SDSceneStatsGrid->SetDefaultCellBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );
	SDSceneStatsGrid->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	bSizer41->Add( SDSceneStatsGrid, 1, wxALL|wxEXPAND, 0 );
	
	SDSceneStats->SetSizer( bSizer41 );
	SDSceneStats->Layout();
	bSizer41->Fit( SDSceneStats );
	SDStatsVariables->AddPage( SDSceneStats, wxT("Statistics"), false );
	
	bSizer211->Add( SDStatsVariables, 1, wxEXPAND | wxALL, 5 );
	
	SDPanelStatsBottom->SetSizer( bSizer211 );
	SDPanelStatsBottom->Layout();
	bSizer211->Fit( SDPanelStatsBottom );
	SDSplitterStats->SplitHorizontally( SDPanelStatsTop, SDPanelStatsBottom, 410 );
	bSizer181->Add( SDSplitterStats, 1, wxEXPAND, 5 );
	
	SDPanelStats->SetSizer( bSizer181 );
	SDPanelStats->Layout();
	bSizer181->Fit( SDPanelStats );
	SDViewSwitch->AddPage( SDPanelStats, wxT("Stats"), false );
	
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
	this->Connect( SDShaderCompileSources->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Connect( SDShaderSaveBinary->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Connect( SDShaderLegacy->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Connect( SDShaderUpgradeSM1X->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Connect( SDShaderMaximumSM->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Connect( SDShaderOptimize->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Connect( SDShaderRuntime->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Connect( SDEffectCompileSources->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoEffectOptions ) );
	this->Connect( SDEffectSaveBinary->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoEffectOptions ) );
	this->Connect( SDEffectLegacy->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoEffectOptions ) );
	this->Connect( SDEffectOptimize->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoEffectOptions ) );
	this->Connect( SDProfileGPU->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoProfileOptions ) );
	this->Connect( SDProfileTex->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoProfileOptions ) );
	this->Connect( SDModeWireframe->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoProfileOptions ) );
	this->Connect( SDModeTesselation->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoModeTesselation ) );
	this->Connect( SDConvertQDMy->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolPMtoQDMy ) );
	this->Connect( SDConvertQDMn->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolPMtoQDMn ) );
	this->Connect( SDConvertRGBH->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipRGBH ) );
	this->Connect( SDConvertRGBA->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipRGBA ) );
	this->Connect( SDConvertRGB->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipRGB ) );
	this->Connect( SDConvertLA->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipLA ) );
	this->Connect( SDConvertA->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipA ) );
	this->Connect( SDConvertXYZD->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipXYZD ) );
	this->Connect( SDConvertXYZ->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipXYZ ) );
	this->Connect( SDConvertXY_Z->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipXY_Z ) );
	this->Connect( SDConvertXY->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipXY ) );
	this->Connect( SDTweakAF1->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoAF ) );
	this->Connect( SDTweakAF2->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoAF ) );
	this->Connect( SDTweakAF4->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoAF ) );
	this->Connect( SDTweakAF8->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoAF ) );
	this->Connect( SDTweakAF16->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoAF ) );
	this->Connect( SDTweakAF32->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoAF ) );
	this->Connect( SDTweaksGamma->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoLinear ) );
	SDChoicePass->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoRenderpassSwitch ), NULL, this );
	SDViewSwitch->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( wxShaderDeveloper::DoViewSwitch ), NULL, this );
	SDComboShader->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderSwitch ), NULL, this );
	SDButtonShaderLoad->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderLoad ), NULL, this );
	SDButtonShaderSave->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderSave ), NULL, this );
	SDButtonShaderSaveAs->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderSaveAs ), NULL, this );
	SDShaderVersion->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderVersion ), NULL, this );
	SDShaderCompile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderCompile ), NULL, this );
	SDShaderCompileAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderCompileAll ), NULL, this );
	SDShaderFlush->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderFlush ), NULL, this );
	SDShaderSourceEditor->Connect( wxEVT_KEY_UP, wxKeyEventHandler( wxShaderDeveloper::DoShaderHighlight ), NULL, this );
	SDShaderSourceEditor->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxShaderDeveloper::DoShaderUpdate ), NULL, this );
	SDShaderAssemblyEditor->Connect( wxEVT_KEY_UP, wxKeyEventHandler( wxShaderDeveloper::DoAssemblerHighlight ), NULL, this );
	SDShaderAssemblyEditor->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxShaderDeveloper::DoShaderUpdate ), NULL, this );
	SDShaderEnable->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderToggle ), NULL, this );
	SDShaderMark->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoMarkToggle ), NULL, this );
	SDShaderConstSetGrid->Connect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( wxShaderDeveloper::DoShaderConstantChange ), NULL, this );
	SDShaderConstSetGrid->Connect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( wxShaderDeveloper::DoShaderConstantSelect ), NULL, this );
	SDShaderSamplerGrid->Connect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( wxShaderDeveloper::DoShaderSamplerChange ), NULL, this );
	SDShaderSamplerGrid->Connect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( wxShaderDeveloper::DoShaderSamplerSelect ), NULL, this );
	SDComboEffect->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoEffectSwitch ), NULL, this );
	SDButtonEffectNew->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoEffectAdd ), NULL, this );
	SDButtonEffectLoad->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoEffectLoad ), NULL, this );
	SDButtonEffectSave->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoEffectSave ), NULL, this );
	SDButtonEffectSaveAs->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoEffectSaveAs ), NULL, this );
	SDEffectCompile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoEffectCompile ), NULL, this );
	SDEffectSourceEditor->Connect( wxEVT_KEY_UP, wxKeyEventHandler( wxShaderDeveloper::DoEffectHighlight ), NULL, this );
	SDEffectSourceEditor->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxShaderDeveloper::DoEffectUpdate ), NULL, this );
	SDEffectEnable->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoEffectToggle ), NULL, this );
	SDEffectConstSetGrid->Connect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( wxShaderDeveloper::DoEffectConstantChange ), NULL, this );
	SDEffectConstSetGrid->Connect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( wxShaderDeveloper::DoEffectConstantSelect ), NULL, this );
	SDEffectTexturesGrid->Connect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( wxShaderDeveloper::DoEffectTextureChange ), NULL, this );
	SDEffectTexturesGrid->Connect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( wxShaderDeveloper::DoEffectTextureSelect ), NULL, this );
	SDChoiceScene->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoScenesSwitch ), NULL, this );
	SDSurfaceSwitch->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( wxShaderDeveloper::DoSurfaceSwitch ), NULL, this );
	SDRendertargetView->Connect( wxEVT_PAINT, wxPaintEventHandler( wxShaderDeveloper::SDPaintRT ), NULL, this );
	SDRendertargetGrabbedView->Connect( wxEVT_PAINT, wxPaintEventHandler( wxShaderDeveloper::SDPaintGrabbedRT ), NULL, this );
	SDDepthStencilView->Connect( wxEVT_PAINT, wxPaintEventHandler( wxShaderDeveloper::SDPaintDS ), NULL, this );
	SDStatsView->Connect( wxEVT_PAINT, wxPaintEventHandler( wxShaderDeveloper::SDPaintProfile ), NULL, this );
	SDStatsNormalize->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoStatsToggle ), NULL, this );
	SDChoiceStats->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoStatsSwitch ), NULL, this );
}

wxShaderDeveloper::~wxShaderDeveloper()
{
	// Disconnect Events
	this->Disconnect( wxEVT_ACTIVATE, wxActivateEventHandler( wxShaderDeveloper::DoActivate ) );
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( wxShaderDeveloper::DoClose ) );
	this->Disconnect( wxEVT_SIZE, wxSizeEventHandler( wxShaderDeveloper::DoResize ) );
	this->Disconnect( wxEVT_UPDATE_UI, wxUpdateUIEventHandler( wxShaderDeveloper::DoUpdate ) );
	this->Disconnect( wxID_SCOMPILE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Disconnect( wxID_SSAVEBIN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Disconnect( wxID_SLEGACY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Disconnect( wxID_SUPGRADE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Disconnect( wxID_SMAXIMUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Disconnect( wxID_SOPTIMIZE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Disconnect( wxID_SRUNTIME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderOptions ) );
	this->Disconnect( wxID_ECOMPILE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoEffectOptions ) );
	this->Disconnect( wxID_ESAVEBIN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoEffectOptions ) );
	this->Disconnect( wxID_ELEGACY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoEffectOptions ) );
	this->Disconnect( wxID_EOPTIMIZE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoEffectOptions ) );
	this->Disconnect( wxID_PROFILE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoProfileOptions ) );
	this->Disconnect( wxID_KILLTEX, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoProfileOptions ) );
	this->Disconnect( wxID_WIREFRAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoProfileOptions ) );
	this->Disconnect( wxID_TESSELATION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoModeTesselation ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolPMtoQDMy ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolPMtoQDMn ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipRGBH ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipRGBA ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipRGB ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipLA ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipA ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipXYZD ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipXYZ ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipXY_Z ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoToolRemipXY ) );
	this->Disconnect( wxID_AF1, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoAF ) );
	this->Disconnect( wxID_AF2, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoAF ) );
	this->Disconnect( wxID_AF4, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoAF ) );
	this->Disconnect( wxID_AF8, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoAF ) );
	this->Disconnect( wxID_AF16, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoAF ) );
	this->Disconnect( wxID_AF32, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoAF ) );
	this->Disconnect( wxID_LINEAR, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoLinear ) );
	SDChoicePass->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoRenderpassSwitch ), NULL, this );
	SDViewSwitch->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( wxShaderDeveloper::DoViewSwitch ), NULL, this );
	SDComboShader->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderSwitch ), NULL, this );
	SDButtonShaderLoad->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderLoad ), NULL, this );
	SDButtonShaderSave->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderSave ), NULL, this );
	SDButtonShaderSaveAs->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderSaveAs ), NULL, this );
	SDShaderVersion->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoShaderVersion ), NULL, this );
	SDShaderCompile->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderCompile ), NULL, this );
	SDShaderCompileAll->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderCompileAll ), NULL, this );
	SDShaderFlush->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderFlush ), NULL, this );
	SDShaderSourceEditor->Disconnect( wxEVT_KEY_UP, wxKeyEventHandler( wxShaderDeveloper::DoShaderHighlight ), NULL, this );
	SDShaderSourceEditor->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxShaderDeveloper::DoShaderUpdate ), NULL, this );
	SDShaderAssemblyEditor->Disconnect( wxEVT_KEY_UP, wxKeyEventHandler( wxShaderDeveloper::DoAssemblerHighlight ), NULL, this );
	SDShaderAssemblyEditor->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxShaderDeveloper::DoShaderUpdate ), NULL, this );
	SDShaderEnable->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoShaderToggle ), NULL, this );
	SDShaderMark->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoMarkToggle ), NULL, this );
	SDShaderConstSetGrid->Disconnect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( wxShaderDeveloper::DoShaderConstantChange ), NULL, this );
	SDShaderConstSetGrid->Disconnect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( wxShaderDeveloper::DoShaderConstantSelect ), NULL, this );
	SDShaderSamplerGrid->Disconnect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( wxShaderDeveloper::DoShaderSamplerChange ), NULL, this );
	SDShaderSamplerGrid->Disconnect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( wxShaderDeveloper::DoShaderSamplerSelect ), NULL, this );
	SDComboEffect->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoEffectSwitch ), NULL, this );
	SDButtonEffectNew->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoEffectAdd ), NULL, this );
	SDButtonEffectLoad->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoEffectLoad ), NULL, this );
	SDButtonEffectSave->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoEffectSave ), NULL, this );
	SDButtonEffectSaveAs->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoEffectSaveAs ), NULL, this );
	SDEffectCompile->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoEffectCompile ), NULL, this );
	SDEffectSourceEditor->Disconnect( wxEVT_KEY_UP, wxKeyEventHandler( wxShaderDeveloper::DoEffectHighlight ), NULL, this );
	SDEffectSourceEditor->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( wxShaderDeveloper::DoEffectUpdate ), NULL, this );
	SDEffectEnable->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoEffectToggle ), NULL, this );
	SDEffectConstSetGrid->Disconnect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( wxShaderDeveloper::DoEffectConstantChange ), NULL, this );
	SDEffectConstSetGrid->Disconnect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( wxShaderDeveloper::DoEffectConstantSelect ), NULL, this );
	SDEffectTexturesGrid->Disconnect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler( wxShaderDeveloper::DoEffectTextureChange ), NULL, this );
	SDEffectTexturesGrid->Disconnect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( wxShaderDeveloper::DoEffectTextureSelect ), NULL, this );
	SDChoiceScene->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoScenesSwitch ), NULL, this );
	SDSurfaceSwitch->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( wxShaderDeveloper::DoSurfaceSwitch ), NULL, this );
	SDRendertargetView->Disconnect( wxEVT_PAINT, wxPaintEventHandler( wxShaderDeveloper::SDPaintRT ), NULL, this );
	SDRendertargetGrabbedView->Disconnect( wxEVT_PAINT, wxPaintEventHandler( wxShaderDeveloper::SDPaintGrabbedRT ), NULL, this );
	SDDepthStencilView->Disconnect( wxEVT_PAINT, wxPaintEventHandler( wxShaderDeveloper::SDPaintDS ), NULL, this );
	SDStatsView->Disconnect( wxEVT_PAINT, wxPaintEventHandler( wxShaderDeveloper::SDPaintProfile ), NULL, this );
	SDStatsNormalize->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( wxShaderDeveloper::DoStatsToggle ), NULL, this );
	SDChoiceStats->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( wxShaderDeveloper::DoStatsSwitch ), NULL, this );
	
}

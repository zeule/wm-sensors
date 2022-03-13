#include "./main_window.hxx"

LPCTSTR lpcstrWSensorsRegKey = _T("Software\\Microsoft\\WTL Samples\\MTPad");

#include "./about_dialog.hxx"
#include "ui/ribbon.h"

#define FILE_MENU_POSITION 0

LRESULT wsensors::ui::MainWindow::onCreate(LPCREATESTRUCT /*lpcs*/)
{
	// SetWindowTheme(_T("DarkMode_Explorer"), NULL);

	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// atach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	// SetMenu(NULL);

	bool bRibbonUI = RunTimeHelper::IsRibbonUIAvailable();
	if (bRibbonUI) {
		// UI Setup and adjustments
		UIAddMenu(m_CmdBar.GetMenu(), true);
		UIRemoveUpdateElement(ID_FILE_MRU_FIRST);
		UIPersistElement(ID_GROUP_CLIPBOARD);

		// Ribbon Controls initialization
		// m_groupFont.SetImage(UI_PKEY_SmallImage, GetCommandBarBitmap(ID_FORMAT_FONT));
		// m_spinner.SetValue(UI_PKEY_MinValue, 1);

		// Ribbon UI state and settings restoration
		CRibbonPersist(lpcstrWSensorsRegKey).Restore(bRibbonUI, m_hgRibbonSettings);
	} else
		CMenuHandle(m_CmdBar.GetMenu()).DeleteMenu(ID_VIEW_RIBBON, MF_BYCOMMAND);

	/*HWND hWndToolBar =*/ CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	// AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateSimpleStatusBar();
#if 1
	statusBar_.SubclassWindow(m_hWndStatusBar);
	int arrParts[] = {ID_DEFAULT_PANE, IDS_STRING_STATUS_PANEL_STATS, IDS_MAIN_STATUS_PANEL_STATUS};
	statusBar_.SetPanes(arrParts, sizeof(arrParts) / sizeof(int), false);
	sensorsTreeContainer_.Create(
	    m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE);
	m_hWndClient = sensorsTreeContainer_.m_hWnd;
	ATLASSERT(::IsWindow(m_hWndClient));

	/* m_hWndClient = sensorsTree_.Create(
	    m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | TVS_HASLINES | TVS_HASBUTTONS,
	    WS_EX_CLIENTEDGE);
	*/
	sensorsTree_.createAndSetup(m_hWndClient);
#endif

	//		UIAddToolBar(hWndToolBar);
	//		UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

#if 0
		m_hWndToolBarPP = CreateSimpleToolBarCtrl(
		    m_hWnd, IDR_PRINTPREVIEWBAR, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE, ATL_IDW_TOOLBAR + 1);
		AddSimpleReBarBand(m_hWndToolBarPP, NULL, TRUE);
		
		UIAddToolBar(m_hWndToolBarPP);
#endif
	//HMENU hMenu = m_CmdBar.GetMenu();
	//HMENU hFileMenu = ::GetSubMenu(hMenu, FILE_MENU_POSITION);
#ifdef _DEBUG
	// absolute position, can change if menu changes
	//TCHAR szMenuString[100];
//		::GetMenuString(hFileMenu, RECENT_MENU_POSITION, szMenuString, sizeof(szMenuString), MF_BYPOSITION);
//		ATLASSERT(lstrcmp(szMenuString, _T("Recent &Files")) == 0);
#endif //_DEBUG
	//		HMENU hMruMenu = ::GetSubMenu(hFileMenu, RECENT_MENU_POSITION);
	//		m_mru.SetMenuHandle(hMruMenu);

	//		m_mru.ReadFromRegistry(lpcstrMTPadRegKey);

	ShowRibbonUI(bRibbonUI);
	UISetCheck(ID_VIEW_RIBBON, bRibbonUI);

	// Tray icon
	HICON hIconSmall = (HICON)::LoadImage(
	    _Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON),
	    ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

	InstallIcon(_T("wsensors"), hIconSmall, IDR_TRAY_POPUP);

	return 0;
}

void wsensors::ui::MainWindow::onClose()
{
	ShowWindow(SW_HIDE);
}

LRESULT wsensors::ui::MainWindow::onAppExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DefWindowProc(WM_CLOSE, 0, 0);
	return 0;
}

LRESULT wsensors::ui::MainWindow::onAppAbout(WORD, WORD, HWND, BOOL&)
{
	AboutDialog dlg;
	dlg.DoModal();
	return 0;
}

LRESULT wsensors::ui::MainWindow::onTrayShowHide(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShowWindow(IsWindowVisible() ? SW_HIDE : SW_SHOW);
	return 0;
}

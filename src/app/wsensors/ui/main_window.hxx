#pragma once

#include "./sensors_tree.hxx"
#include "./trayiconimpl.h"

#include "../stdatl.hxx"

#include "resource.h"

#include <atlcrack.h>
#include <atlribbon.h>
#include <atltheme.h>

namespace wsensors::ui {
	class MainWindow: public CRibbonFrameWindowImpl<MainWindow>, public CTrayIconImpl<MainWindow> {
		using ThisType = MainWindow;

	public:
		DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME);

		BEGIN_MSG_MAP(ThisType)
			MSG_WM_CREATE(onCreate)
			MSG_WM_CLOSE(onClose)
			CHAIN_MSG_MAP(CRibbonFrameWindowImpl<ThisType>)
			CHAIN_MSG_MAP(CTrayIconImpl<ThisType>)
			COMMAND_ID_HANDLER(ID_APP_EXIT, onAppExit)
			COMMAND_ID_HANDLER(ID_APP_ABOUT, onAppAbout)
			COMMAND_ID_HANDLER(ID_TRAYPOPUP_SHOW_HIDE, onTrayShowHide)
			//	CHAIN_COMMANDS_MEMBER((sensorsTree_))
		END_MSG_MAP()

#if 0
		BEGIN_RIBBON_CONTROL_MAP(MainWindow)
		END_RIBBON_CONTROL_MAP()
#endif

		CCommandBarCtrl m_CmdBar;

		SensorsTree& sensorsTree()
		{
			return sensorsTree_;
		}

	private:
		LRESULT onCreate(LPCREATESTRUCT lpcs);
		void onClose();
		LRESULT onAppExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT onAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT onTrayShowHide(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

		class DummyWindow: public CWindowImpl<DummyWindow> {
			BEGIN_MSG_MAP(DummyWindow)
			END_MSG_MAP()
		};

		DummyWindow sensorsTreeContainer_;
		SensorsTree sensorsTree_;
		CMultiPaneStatusBarCtrl statusBar_;
	};
} // namespace wsensors::ui

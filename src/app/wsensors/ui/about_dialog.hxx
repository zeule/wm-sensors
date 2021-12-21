#pragma once

#include "./resource.h"

namespace wsensors::ui {

	class AboutDialog: public CDialogImpl<AboutDialog> {
	public:
		enum
		{
			IDD = IDD_ABOUTBOX
		};

		BEGIN_MSG_MAP(AboutDialog)
			MESSAGE_HANDLER(WM_INITDIALOG, onInitDialog)
			COMMAND_ID_HANDLER(IDOK, onCloseCmd)
		END_MSG_MAP()

		// Handler prototypes (uncomment arguments if needed):
		//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
		//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
		//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT onInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT onCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	};
} // namespace wsensors::ui

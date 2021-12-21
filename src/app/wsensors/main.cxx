#include "stdafx.hxx"

#include "./controller.hxx"
#include "./settings.hxx"
#include "./utility.hxx"
#include "ui/main_window.hxx"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <vector>

CAppModule _Module;

int Run(LPTSTR /*lpCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	wsensors::GdiPlusInit gdiplus;

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	wsensors::Settings settings;
	wsensors::ui::MainWindow mainWindow;

	if (mainWindow.CreateEx(NULL) == NULL) {
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wsensors::Controller controller{wm_sensors::SensorsTree(), mainWindow.sensorsTree(), settings};
	theLoop.AddMessageFilter(&controller);

	mainWindow.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(
    _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	std::vector<spdlog::sink_ptr> sinks;
	if (true) {
		sinks.push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
	}
#if 0
	if (!logFile.empty()) {
		sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile.string()));
	}
#endif
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
#endif

	auto logger = std::make_shared<spdlog::logger>("wsensors", std::begin(sinks), std::end(sinks));
	spdlog::register_logger(logger);
	spdlog::set_default_logger(logger);

	::CoInitialize(NULL);

	INITCOMMONCONTROLSEX iccx;
	iccx.dwSize = sizeof(iccx);
	iccx.dwICC = ICC_BAR_CLASSES | ICC_TREEVIEW_CLASSES; // change to support other controls
	::InitCommonControlsEx(&iccx);

	_Module.Init(NULL, hInstance);

	int nRet = Run(lpCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();
	return nRet;
}

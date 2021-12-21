add_executable(wsensors WIN32)

target_sources(wsensors PRIVATE
main.cxx
custom_messages.h
ui/about_dialog.cxx
ui/about_dialog.hxx
ui/main_window.cxx
ui/main_window.hxx
ui/sensors_tree.cxx
ui/sensors_tree.hxx
ui/TreeListView.h
ui/trayiconimpl.h
ui/wsensors.rc
controller.cxx
controller.hxx
sensors_data_model.cxx
sensors_data_model.hxx
settings.cxx
settings.hxx
utility.cxx
utility.hxx
)

target_precompile_headers(wsensors PRIVATE
	"stdafx.hxx"
)

auto_source_group(wsensors)

# when generating for VS, resource compiler is simply "rc", but when generating for Ninja (Open CMake folder in VS),
# resource compiler is specified with a full path to a binary in Windows SDK

message(CHECK_START "Finding out how to call uicc")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
if (CMAKE_RC_COMPILER STREQUAL "rc")
	message(STATUS "Assuming uicc.exe in %PATH%")
	set(_uicc "uicc.exe")
else()
	message(STATUS "Deducing WindowsSDK path from resorce compiler executable (${CMAKE_RC_COMPILER})")
	# use path to a tool from WindowsSDK to get path to its bin firectory
	# for example: CMAKE_RC_COMPILER: C:/Program Files (x86)/Windows Kits/10/bin/10.0.22000.0/x64/rc.exe
	set(_uicc_path "../../x86/uicc.exe")
	cmake_path(ABSOLUTE_PATH _uicc_path BASE_DIRECTORY "${CMAKE_RC_COMPILER}" NORMALIZE OUTPUT_VARIABLE _uicc)
endif()
list(POP_BACK CMAKE_MESSAGE_INDENT)
message(CHECK_PASS "Using uicc executable ${_uicc}")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ui")
add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ui/ribbon.bml" "${CMAKE_CURRENT_BINARY_DIR}/ui/ribbon.rc" "${CMAKE_CURRENT_BINARY_DIR}/ui/ribbon.h"
	COMMAND "${_uicc}" "${CMAKE_CURRENT_SOURCE_DIR}/ui/ribbon.xml" ribbon.bml /header:ribbon.h /res:ribbon.rc
	MAIN_DEPENDENCY ui/ribbon.xml
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ui"
)

target_sources(wsensors PRIVATE 
	"${CMAKE_CURRENT_BINARY_DIR}/ui/ribbon.h"
	"${CMAKE_CURRENT_BINARY_DIR}/ui/ribbon.rc"
)

target_include_directories(wsensors PRIVATE
"${CMAKE_CURRENT_BINARY_DIR}"
../..
)

target_link_libraries(wsensors 
	PRIVATE
		wm-sensors
		wtl::wtl
		Gdiplus
)

target_compile_definitions(wsensors PRIVATE UNICODE _UNICODE STRICT)

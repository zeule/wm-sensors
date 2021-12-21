target_sources(wm-sensors PRIVATE
	kernel_driver.cxx
	kernel_driver.hxx
	inpout.cxx
	inpout.hxx
	ioctl.hxx
	winring0.cxx
	winring0.hxx
	
)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_winRingDriverFile "WinRing0x64.sys")
	set(_inpoutDriverFile "inpoutx64.sys")
else()
	set(_winRingDriverFile "WinRing0.sys")
	set(_inpoutDriverFile "inpout32.sys")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ring0-drv-config.h.cmakein"
	"${CMAKE_CURRENT_BINARY_DIR}/ring0-drv-config.h" @ONLY
)

function (copy_file_to_binary_dir _customTargetName _target _srcFile)
	get_filename_component(_fileName "${_srcFile}" NAME)
	if (WIN32)
		set(action copy_if_different)
	else()
		set(action create_symlink)
	endif()
	add_custom_target(${_customTargetName}
		COMMAND ${CMAKE_COMMAND} -E ${action} "${_srcFile}" "$<TARGET_FILE_DIR:${_target}>/${_fileName}")
	add_dependencies(${_target} ${_customTargetName})
endfunction()

copy_file_to_binary_dir(copy-winring0-driver wm-sensors "${CMAKE_CURRENT_SOURCE_DIR}/${_winRingDriverFile}")
copy_file_to_binary_dir(copy-inpout-driver wm-sensors "${CMAKE_CURRENT_SOURCE_DIR}/${_inpoutDriverFile}")

target_include_directories(wm-sensors PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

install(
	FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/${_winRingDriverFile}"
		"${CMAKE_CURRENT_SOURCE_DIR}/${_inpoutDriverFile}"
	TYPE BIN COMPONENT "runtime"
)

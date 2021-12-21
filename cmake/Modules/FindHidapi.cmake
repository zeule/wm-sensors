
if (NOT TARGET hidapi::hidapi)
	find_package(${CMAKE_FIND_PACKAGE_NAME} CONFIG)
	if (NOT ${${CMAKE_FIND_PACKAGE_NAME}_FOUND})
		include(FindPackageHandleStandardArgs)
		find_package(PkgConfig)
		if (PkgConfig_FOUND)
			pkg_check_modules(HIDAPI hidapi-libusb>=0.10.0  IMPORTED_TARGET GLOBAL)
			if (HIDAPI_FOUND)
				add_library(hidapi::hidapi ALIAS PkgConfig::HIDAPI)
			endif()
		endif()
		find_package_handle_standard_args("${CMAKE_FIND_PACKAGE_NAME}"
			REQUIRED_VARS HIDAPI_LIBRARIES
		)
	endif()
endif()

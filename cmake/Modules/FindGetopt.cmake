if (NOT TARGET getopt::getopt)
	find_library(GETOPT_LIBRARY NAMES getopt)
	find_path(GETOPT_INCLUDE_DIR NAMES getopt.h)
	# find_file(_dll NAMES getopt.dll)
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args("${CMAKE_FIND_PACKAGE_NAME}"
			REQUIRED_VARS GETOPT_LIBRARY GETOPT_INCLUDE_DIR
	)
	add_library(getopt::getopt SHARED IMPORTED)
	set_target_properties(getopt::getopt PROPERTIES
		IMPORTED_LOCATION "${_dll}"
		IMPORTED_IMPLIB "${GETOPT_LIBRARY}"
		INTERFACE_INCLUDE_DIRECTORIES "${GETOPT_INCLUDE_DIR}"
	)
endif()

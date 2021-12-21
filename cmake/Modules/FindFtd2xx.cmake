if (NOT TARGET Ftd2xx::Ftd2xx)
	find_library(ftd2xx_lib NAMES ftd2xx)
	find_path(ftdx_inc_dir NAMES ftd2xx.h)
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args("${CMAKE_FIND_PACKAGE_NAME}"
			REQUIRED_VARS ftd2xx_lib ftdx_inc_dir
	)
	add_library(Ftd2xx::Ftd2xx SHARED IMPORTED)
	set_target_properties(Ftd2xx::Ftd2xx PROPERTIES
		IMPORTED_LOCATION "${ftd2xx_lib}"
		INTERFACE_INCLUDE_DIRECTORIES "${ftdx_inc_dir}"
	)
endif()

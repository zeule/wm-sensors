if (NOT TARGET wtl::wtl)
	find_path(WTL_INCLUDE_DIR "atlapp.h" HINTS ENV WTL_DIR PATH_SUFFIXES "wtl")
	find_package_handle_standard_args("${CMAKE_FIND_PACKAGE_NAME}"
			REQUIRED_VARS WTL_INCLUDE_DIR
	)
	add_library(wtlwtl INTERFACE)
	set_target_properties(wtlwtl PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${WTL_INCLUDE_DIR}"
	)
	add_library(wtl::wtl ALIAS wtlwtl)
endif()

cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/.git")
	include(VersionFromGit)
	version_from_git(
		LOG       ON
		TIMESTAMP "%Y%m%d"
		VAR_PREFIX WMS_
	)
endif()

project(wm-sensors
	VERSION ${WMS_VERSION}
)

include(CPack)
include(FeatureSummary)
include(Utility)

add_subdirectory(src)

feature_summary(WHAT ALL)

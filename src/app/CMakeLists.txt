add_subdirectory(sensors)

find_package(WTL)
if (WTL_FOUND)
	add_subdirectory(wsensors)
endif()

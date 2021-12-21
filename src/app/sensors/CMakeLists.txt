add_executable(sensors)
target_sources(sensors PRIVATE chips.c main.c)
target_link_libraries(sensors PRIVATE wm-sensors)
target_include_directories(sensors PRIVATE ../..)

if(MSVC)
	target_compile_options(sensors PRIVATE /W0) # essentially this is a 3rd-party code
endif()

if (WIN32)
	find_package(Getopt REQUIRED)
	target_link_libraries(sensors PRIVATE getopt::getopt)
endif()



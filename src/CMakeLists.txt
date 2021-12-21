set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN True)

option(BUILD_SHARED_LIBS "Build libraries as shared" ON)

find_package(Boost 1.75 REQUIRED)
find_package(spdlog 1.9 REQUIRED)

include(GNUInstallDirs)
include(CompilerSettings)

wms_set_compiler_flags()

include(ExternalProject)

if (WIN32)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmakein" "${CMAKE_CURRENT_BINARY_DIR}/version.h" @ONLY)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(lib)
add_subdirectory(app)

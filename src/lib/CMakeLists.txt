project(libwm-sensors)
add_library(wm-sensors)

include(GenerateExportHeader)
include(CheckFunctionExists)

# checking for strndup
set(CMAKE_REQUIRED_INCLUDES "<string.h>")
check_function_exists(strndup HAVE_STRNDUP)

configure_file(config.h.cmakein wm-sensors_config.h)

generate_export_header(wm-sensors
          #[BASE_NAME <base_name>]
          #[EXPORT_MACRO_NAME <export_macro_name>]
          #[EXPORT_FILE_NAME <export_file_name>]
          #[DEPRECATED_MACRO_NAME <deprecated_macro_name>]
          #[NO_EXPORT_MACRO_NAME <no_export_macro_name>]
          #[INCLUDE_GUARD_NAME <include_guard_name>]
          #[STATIC_DEFINE <static_define>]
          #[NO_DEPRECATED_MACRO_NAME <no_deprecated_macro_name>]
          #[DEFINE_NO_DEPRECATED]
          #[PREFIX_NAME <prefix_name>]
          #[CUSTOM_CONTENT_FROM_VARIABLE <variable>]
)

target_include_directories(wm-sensors
	PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

find_package(PalSigslot CONFIG REQUIRED)
find_package(Hidapi)

target_sources(wm-sensors PRIVATE
sensor.cxx
sensor.hxx
sensors.cxx
sensors.h
error.h
source_class.cxx
source_class.hxx
sensor_path.cxx
sensor_path.hxx
sensor_tree.cxx
sensor_tree.hxx
stdint.hxx
wm_sensor_types.hxx
visitor/chip_visitor.cxx
visitor/chip_visitor.hxx
impl/chip_registrator.cxx
impl/chip_registrator.hxx
impl/group_affinity.cxx
impl/group_affinity.hxx
impl/sensor_collection.hxx
impl/libsensors/chip_data.cxx
impl/libsensors/chip_data.hxx
impl/libsensors/error.cxx
config.h.cmakein
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") 
	target_link_options(wm-sensors PUBLIC /MANIFESTUAC:level='highestAvailable')
endif()

if (WIN32)
	target_compile_definitions(wm-sensors PRIVATE UNICODE _UNICODE STRICT)
endif()

target_link_libraries(wm-sensors
	PRIVATE
		Boost::boost
	PUBLIC
		spdlog::spdlog
		Pal::Sigslot
)

if (Hidapi_FOUND)
	target_link_libraries(wm-sensors PRIVATE hidapi::hidapi)
endif()

set_property(TARGET wm-sensors PROPERTY PUBLIC_HEADER
	sensor.hxx wm_sensor_types.hxx sensor_tree.hxx
	sensors.h error.h
	${CMAKE_CURRENT_BINARY_DIR}/wm-sensors_export.h
)

add_subdirectory(hardware)
add_subdirectory(utility)

auto_source_group(wm-sensors)

install(TARGETS wm-sensors
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
		PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/wm-sensors COMPONENT development
)

install(DIRECTORY visitor DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/wm-sensors/
	COMPONENT development FILES_MATCHING PATTERN "*.hxx")

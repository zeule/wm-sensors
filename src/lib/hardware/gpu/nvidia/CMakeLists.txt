set(_nvapi_dir "${libwm-sensors_SOURCE_DIR}/3rd-party/nvapi")

target_include_directories(wm-sensors SYSTEM PRIVATE "${_nvapi_dir}")
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	target_link_directories(wm-sensors PRIVATE "${_nvapi_dir}/amd64")
	target_link_libraries(wm-sensors PRIVATE nvapi64)
else()
	target_link_directories(wm-sensors PRIVATE "${_nvapi_dir}/x86")
	target_link_libraries(wm-sensors PRIVATE nvapi)
endif()

target_sources(wm-sensors PRIVATE
nvidia_gpu.cxx
nvidia_gpu.hxx
probe.cxx
)

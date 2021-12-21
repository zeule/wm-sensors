target_sources(wm-sensors PRIVATE
smbios.cxx
smbios.hxx
smbios_enums_io.cxx
)

add_subdirectory(impl)
add_subdirectory(controller)
add_subdirectory(cpu)
add_subdirectory(gpu)
add_subdirectory(memory)
add_subdirectory(motherboard)
add_subdirectory(psu)

target_sources(wm-sensors PRIVATE
identification.cxx
identification.hxx
lpc_io.cxx
lpc_io.hxx
port.cxx
port.hxx
)

add_subdirectory(ec)
add_subdirectory(superio)

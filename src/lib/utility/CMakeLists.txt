target_sources(wm-sensors PRIVATE
bit.hxx
macro.hxx
string.cxx
string.hxx
unaligned.hxx
utility.hxx
)
if (Hidapi_FOUND)
	add_subdirectory(hidapi++)
endif()

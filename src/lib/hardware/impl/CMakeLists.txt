target_sources(wm-sensors PRIVATE
	ring0.cxx
	ring0.hxx
)

if (Hidapi_FOUND)
	target_sources(wm-sensors PRIVATE 
		usbhid_chip.cxx
		usbhid_chip.hxx
	)
endif()

add_subdirectory(ring0)

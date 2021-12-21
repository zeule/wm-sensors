macro(wms_set_compiler_flags)
	set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
	set(CMAKE_C_VISIBILITY_PRESET "hidden")
	set(CMAKE_VISIBILITY_INLINES_HIDDEN True)

	if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
		message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION} [${CMAKE_CXX_COMPILER}]")

		include(CheckCXXCompilerFlag)

		#-Wshadow -Wconversion ?
		set(_GCC_COMMON_C_AND_CXX_FLAGS "-pedantic -pedantic-errors"
			"-Wall -Wextra"
			"-Wfloat-equal -Wcast-qual -Wcast-align"
			"-Wsign-conversion -Winvalid-pch -Werror=return-type -Wno-long-long"
			"-Wshadow=local"
	#	-fstack-protector-all
			"-Werror -Wno-error=deprecated-declarations -Wno-error=cpp"
	# 		"-fsanitize=address -fsanitize=undefined"
			)

		set (_GCC_COMMON_CXX_FLAGS "-fexceptions -frtti -Woverloaded-virtual -Wold-style-cast"
			"-Werror=overloaded-virtual"
			"-Wnon-virtual-dtor -Wstrict-null-sentinel"
	# 		"-Weffc++"
			"-Wsuggest-override"
			)

		list(JOIN _GCC_COMMON_CXX_FLAGS " " _GCC_COMMON_CXX_FLAGS_LIST)

		list(JOIN _GCC_COMMON_C_AND_CXX_FLAGS " " _GCC_COMMON_C_AND_CXX_FLAGS_STRING)
		list(JOIN _GCC_COMMON_CXX_FLAGS " " _GCC_COMMON_CXX_FLAGS_STRING)

		message(STATUS "CXX options: ${_GCC_COMMON_CXX_FLAGS_LIST}")

		string(APPEND CMAKE_C_FLAGS " ${_GCC_COMMON_C_AND_CXX_FLAGS_STRING}")
		string(APPEND CMAKE_CXX_FLAGS " ${_GCC_COMMON_CXX_FLAGS_STRING} ${_GCC_COMMON_C_AND_CXX_FLAGS_STRING}")


		if (CMAKE_SYSTEM_NAME MATCHES Linux)
			set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined")
			set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined")
			# AB: _GNU_SOURCE turns on some other definitions (see
			# /usr/include/features.h). in particular we need _ISOC99_SOURCE
			# (atm at least for isnan(), possibly for more, too)
			#add_definitions(-D_GNU_SOURCE)
			#set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
			set (FIX_KDE_FLAGS "-fexceptions -UQT_NO_EXCEPTIONS") #KDE disables exceptions,

	# 		add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
	# 		set ( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address" )
	# 		set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address" )

			# check whether we can enable -Og optimization for debug build
			# also let's enable -march=native for debug builds
			check_cxx_compiler_flag(-Og _DEBUG_OPTIMIZATION_LEVEL_IS_SUPPORTED)
			set(_DEBUG_OPTIMIZATION_LEVEL_IS_SUPPORTED FALSE) # GDB has problems with -0g

			if (_DEBUG_OPTIMIZATION_LEVEL_IS_SUPPORTED)
				add_compile_options("$<$<CONFIG:DEBUG>:-Og;-g;-g3>")
			else(_DEBUG_OPTIMIZATION_LEVEL_IS_SUPPORTED)
				add_compile_options("$<$<CONFIG:DEBUG>:-O0;-g;-g3>")
			endif (_DEBUG_OPTIMIZATION_LEVEL_IS_SUPPORTED)
		endif (CMAKE_SYSTEM_NAME MATCHES Linux)

	endif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")


	if (WIN32)
		set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH};$ENV{LIB}")

		if (NOT CMAKE_SYSTEM MATCHES "CYGWIN*")
			if (MSVC)
				find_package(WindowsSDK REQUIRED)
				# message(STATUS "WINDOWSSDK_PREFERRED_DIR: ${WINDOWSSDK_PREFERRED_DIR}")
				set(DISABLED_WARNINGS
					4061 4290 4464
					4623 # default constructor was implicitly defined as deleted
					4820 5045 26912
					4706 # assignment within conditional expression
					4710 # function not inlined
					4711 # function 'xxx' selected for automatic inline expansion
				)
				# these ones might actually be useful
				list(APPEND DISABLED_WARNINGS 4514 4868)
				list(TRANSFORM DISABLED_WARNINGS PREPEND -wd)
				list(JOIN DISABLED_WARNINGS " " DISABLE_WARNINGS_CMD)

				set(_MSVC_INCLUDE_DIR "${CMAKE_CXX_COMPILER}/../../../../include")
				cmake_path(NORMAL_PATH _MSVC_INCLUDE_DIR)
				set(_C_CXX_FLAGS "${DISABLE_WARNINGS_CMD} /Wall /WX /external:W0 -external:I\"${WINDOWSSDK_PREFERRED_DIR}\" -external:I\"${_MSVC_INCLUDE_DIR}\"")


				set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_C_CXX_FLAGS}" )
				set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_C_CXX_FLAGS}" )
				
				set ( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Ox /Ot /Ob2" )
				set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Ot /Ob2" )
			endif (MSVC)

			add_definitions(-D_USE_MATH_DEFINES -DNOMINMAX -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS)
			add_compile_options("/utf-8")

		endif (NOT CMAKE_SYSTEM MATCHES "CYGWIN*")
	endif (WIN32)
endmacro(wms_set_compiler_flags)


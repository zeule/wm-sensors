# The MIT License (MIT)
#
# Copyright (c) 2016-2017 Theo Willows
#           (c) 2019 A. Kunz - NTB Interstaatliche Hochschule für Technik Buchs Switzerland
#           (c) 2020 A. Kunz - OST Ostschweizer Fachhochschule, Switzerland
#           (c) 2022 Eugene Shalygin
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.20.0)
include(CMakeParseArguments)

function (version_from_git)
	set(options OPTIONAL FAST )
	set(oneValueKeywords
		GIT_EXECUTABLE
		INCLUDE_HASH
		LOG
		TIMESTAMP
		VAR_PREFIX
	)
	set(multiValueKeywords)
	cmake_parse_arguments(ARG "${options}" "${oneValueKeywords}" "${multiValueKeywords}" ${ARGN})

	if(NOT DEFINED ARG_INCLUDE_HASH)
		set(ARG_INCLUDE_HASH ON)
	endif()

	message(CHECK_START "Looking for version in git repository")
	list(APPEND CMAKE_MESSAGE_INDENT "  ")

	if(DEFINED ARG_GIT_EXECUTABLE)
		set(GIT_EXECUTABLE "${ARG_GIT_EXECUTABLE}")
	else ()
		# Find Git or bail out
		find_package(Git)
		if (NOT Git_FOUND AND NOT DEFINED ARG_DEFAULT)
				message(FATAL_ERROR
					"the Git executable was neither provided nor found and no default version value was provided")
		endif()
	endif()

	execute_process(
		COMMAND				"${GIT_EXECUTABLE}" describe --match "v[0-9]*.[0-9]*.[0-9]*" --tags --always
		WORKING_DIRECTORY	"${CMAKE_CURRENT_SOURCE_DIR}"
		RESULT_VARIABLE		_git_result
		OUTPUT_VARIABLE		_git_describe
		ERROR_VARIABLE		_git_error
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_STRIP_TRAILING_WHITESPACE
    )
	if(_git_result)
		message(FATAL_ERROR "Failed to execute Git: ${git_error}")
	endif()

	message(STATUS "Git describe: ${_git_describe}")

	if(_git_describe MATCHES "^v(0|[1-9][0-9]*)[.](0|[1-9][0-9]*)[.](0|[1-9][0-9]*)-?([0-9]+)?(-g)?(g[.0-9A-Za-z-]+)?$" )
		set(version_major	"${CMAKE_MATCH_1}")
		set(version_minor	"${CMAKE_MATCH_2}")
		set(version_patch	"${CMAKE_MATCH_3}")
		set(version_tweak	"${CMAKE_MATCH_4}") # default if current commit is tagged
		set(hash			"${CMAKE_MATCH_6}")
		if (NOT version_tweak)
			set(version_tweak 0)
		endif()
	else()
		if (_git_describe MATCHES "^[0-9a-f]+$")
			set(version_major	0)
			set(version_minor	0)
			set(version_patch	0)
			set(version_tweak	0)
			set(hash		"${_git_describe}" )
		else()
			message( FATAL_ERROR "Git describe returned unexpected value: [${_git_describe}]")
		endif()
	endif()

	# Construct the version variables
	set(version ${version_major}.${version_minor}.${version_patch})
	set(semver  ${version})
	if (version_tweak)
		string(APPEND semver ".${version_tweak}")
	endif()
	set(version_string "${semver}")
	if (hash)
		string(APPEND version_string "-${hash}")
	endif()

	list(POP_BACK CMAKE_MESSAGE_INDENT)
	message(CHECK_PASS "Detected")

	# Log the results
	if(ARG_LOG)
		message(STATUS
		"version_from_git: Version: ${version}
		Git hash:    [${hash}]
		SemVer:      [${semver}]"
		)
	endif(ARG_LOG)

	# Set parent scope variables
	set(${ARG_VAR_PREFIX}GIT_HASH			${hash}				PARENT_SCOPE)
	set(${ARG_VAR_PREFIX}SEMVER				${semver}			PARENT_SCOPE)
	set(${ARG_VAR_PREFIX}VERSION			${version}			PARENT_SCOPE)
	set(${ARG_VAR_PREFIX}VERSION_MAJOR		${version_major}	PARENT_SCOPE)
	set(${ARG_VAR_PREFIX}VERSION_MINOR		${version_minor}	PARENT_SCOPE)
	set(${ARG_VAR_PREFIX}VERSION_PATCH		${version_patch}	PARENT_SCOPE)
	set(${ARG_VAR_PREFIX}VERSION_TWEAK		${version_tweak}	PARENT_SCOPE)
	set(${ARG_VAR_PREFIX}FULL_VERSION_STR	${version_string}	PARENT_SCOPE)
endfunction()

ExternalProject_Add(amd-display-library
	GIT_REPOSITORY		https://github.com/GPUOpen-LibrariesAndSDKs/display-library
	GIT_TAG				79cdb8e22b8650c861b390d5a9895032492ce8c5 # 16.0
	PREFIX				"${CMAKE_CURRENT_BINARY_DIR}/3rdparty"
	CONFIGURE_COMMAND	""
	BUILD_COMMAND		""
	INSTALL_COMMAND		""
	TEST_COMMAND		""
)

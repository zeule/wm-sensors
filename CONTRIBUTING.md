# Contributing to wm-sensors

## Building
The project requires a C++20-capable compiler and a number of 3rd-party packages. Currently the only
tested compiler is MSVC 14.30 (`_MSC_VER` 1930, Visual Studio 2022) and vcpkg as package manager.

### Environment and required packages

The following packages are required (vcpkg names):

 - Boost
 - spdlog
 - PalSigslot

The following packages are optional:

 - Hidapi (for USB HID based sensors)

To compile the `sensors` application (terminal application from the lm-sensors project), the
following additional packages are required:

 - getopt

For the GUI application `wsensors` additionally required packages are:

 - WTL

### Compiling

The project utilises the CMake build system and the standard configure and build steps apply.


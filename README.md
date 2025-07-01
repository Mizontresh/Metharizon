# Metharizon

A game in the making.

## Prerequisites

- **CMake** 3.10 or higher
- **C++17** compatible compiler
- **vcpkg** package manager
- SDL2, GLAD and GLM installed through vcpkg

Clone the repository with submodules so the `vcpkg` directory is available:

```bash
git clone --recurse-submodules <repo-url>
```

Bootstrap vcpkg and install the dependencies:

```bash
# Windows
./vcpkg/bootstrap-vcpkg.bat
# Linux/macOS
./vcpkg/bootstrap-vcpkg.sh

./vcpkg/vcpkg install sdl2 glad glm
```

## Configuring CMake

`CMAKE_TOOLCHAIN_FILE` must point to the vcpkg toolchain file. When generating the
build files specify:

```bash
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release
```

## Building and Running

After configuring, build the project and run the resulting executable:

```bash
cmake --build build --config Release

# On Windows the executable is in build/Release
# On other platforms the path may be build/Metharizon
./build/Release/Metharizon
```

## Recommended tools and tested platforms

The project uses **CMake** together with **vcpkg** for dependency management. It
has been developed and tested on Windows 10 using Visual Studio 2022, but any
platform with a modern C++17 compiler should be able to build and run the
application.

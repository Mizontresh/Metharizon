# Metharizon
A game in the making

## Building

Dependencies are managed with [vcpkg](https://github.com/microsoft/vcpkg).
If the repository contains a `vcpkg` directory at its root, the CMake
configuration will automatically use the toolchain file from this directory.
Otherwise provide the path manually via the `CMAKE_TOOLCHAIN_FILE` cache
variable, for example:

```sh
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

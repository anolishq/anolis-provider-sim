set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# Release libraries only. Every preset that uses this triplet builds Release or
# RelWithDebInfo, so the debug half of each package was compiled and never linked.
set(VCPKG_BUILD_TYPE release)

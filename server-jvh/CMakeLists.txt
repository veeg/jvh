cmake_minimum_required (VERSION 3.3)

project (server-jvh VERSION 0.0.1)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_PREFIX_PATH "/usr/local/")
find_package (PkgConfig REQUIRED)

pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(PROTOBUF REQUIRED protobuf)
pkg_check_modules(NOPOLL REQUIRED nopoll)
pkg_check_modules(FFMPEG REQUIRED libavcodec)
set (CMAKE_PREFIX_PATH)
add_subdirectory (src)

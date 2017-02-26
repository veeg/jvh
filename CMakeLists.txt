cmake_minimum_required (VERSION 3.3)

project (videostream VERSION 0.0.1)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Require packages
set (CMAKE_PREFIX_PATH "/usr/local/")
find_package (PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(PROTOBUF REQUIRED protobuf)
pkg_check_modules(NOPOLL REQUIRED nopoll)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavutil libavformat libavdevice)
pkg_check_modules(V4L2 REQUIRED libv4l2)

set (CMAKE_PREFIX_PATH)

add_subdirectory (server-jvh)
set_source_files_properties(
        ${CMAKE_BINARY_DIR}/protobuf/videostream.pb.cc
        PROPERTIES GENERATED TRUE
)
add_subdirectory (capture)


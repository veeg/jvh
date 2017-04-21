set(SHARED_LIBS ${CMAKE_SOURCE_DIR}/shared)

add_executable (capture
    "v4l2_capture.cc"
    ${SHARED_LIBS}/prototcp.cc
)

# XXX: please find another way to
# include videostream.pb.cc, if possible.
target_include_directories (capture PRIVATE
    ${SHARED_LIBS}/include/
    ${CMAKE_BINARY_DIR}/protobuf/
    ${PROTOBUF_INCLUDE_DIRS}
    ${V4L2_INCLUDE_DIRS}
)

target_link_libraries (capture
    ${PROTOBUF_LIBRARIES}
    videostream_proto
    ${V4L2_LIBRARIES}
)


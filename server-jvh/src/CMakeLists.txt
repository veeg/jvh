
file (GLOB proto_source protobuf/*.proto)
set (pcode)
set (pheaders)
foreach (protofile ${proto_source})
    get_filename_component(proto ${protofile} NAME_WE)
    set (pcode ${pcode} ${CMAKE_BINARY_DIR}/protobuf/${proto}.pb.cc)
    set (pheaders ${pheaders} ${CMAKE_BINARY_DIR}/protobuf/${proto}.pb.h)
endforeach(protofile)

set (PROTOC_BIN protoc)

add_custom_command(
    OUTPUT ${pcode} ${pheaders}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/protobuf
    COMMAND ${PROTOC_BIN}
    --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/protobuf
    --cpp_out=${CMAKE_BINARY_DIR}/protobuf
    --js_out=binary:${CMAKE_BINARY_DIR}/protobuf
    ${proto_source}
    DEPENDS ${proto_source}
    COMMENT "Generating code from proto files"
)
add_custom_target(protoc ALL DEPENDS ${pcode} ${pheaders})

add_executable (videoservice
    server.cc
    video_source_comm.cc
    client.cc
    main.cc
    encode.cc
    ${CMAKE_BINARY_DIR}/protobuf/videofeed.pb.cc
    ${CMAKE_BINARY_DIR}/protobuf/videoplay.pb.cc
)

target_include_directories (videoservice PRIVATE
    ${CMAKE_BINARY_DIR}/protobuf/
    ${CMAKE_CURRENT_SOURCE_DIR}/private_include/
    ${GTKMM_INCLUDE_DIRS}
    ${NOPOLL_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${SIGC++_INCLUDE_DIRS}
)

target_link_libraries (videoservice
    ${GTKMM_LIBRARIES}
    ${NOPOLL_LIBRARIES}
    ${FFMPEG_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${SIGC++_LIBRARIES}
)
add_dependencies (videoservice protoc)

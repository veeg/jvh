#  Create all protobuf files
file (GLOB proto_source ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/*.proto)
set (pcode)
set (pheaders)
foreach (protofile ${proto_source})
    get_filename_component(proto ${protofile} NAME_WE)
    set (pcode ${pcode} ${CMAKE_BINARY_DIR}/protobuf/${proto}.pb.cc)
    set (pheaders ${pheaders} ${CMAKE_BINARY_DIR}/protobuf/${proto}.pb.h)
endforeach(protofile)

# The protocol buffer compiler binary
set (PROTOC_BIN protoc)

add_custom_command (
  OUTPUT ${pcode} ${pheaders} ${CMAKE_BINARY_DIR}/protobuf/videostreampb.js
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/protobuf
    COMMAND ${PROTOC_BIN}
    --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/protobuf
    --cpp_out=${CMAKE_BINARY_DIR}/protobuf
    --js_out=library=videostreampb,binary:${CMAKE_BINARY_DIR}/protobuf
    ${proto_source}
    DEPENDS ${proto_source}
    COMMENT "Generating code from proto files"
)

add_custom_target(protoc ALL DEPENDS ${pcode} ${pheaders})

add_executable (videostream
    server.cc
    client.cc
    client_tcp.cc
    main.cc
    encoder.cc
    stream_encoded.cc
    queue.cc
    client_websocket.cc
    ${CMAKE_BINARY_DIR}/protobuf/videostream.pb.cc
)

target_include_directories (videostream PRIVATE
    ${CMAKE_BINARY_DIR}/protobuf/
    ${CMAKE_CURRENT_SOURCE_DIR}/private_include/
    ${GTKMM_INCLUDE_DIRS}
    ${NOPOLL_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${SIGC++_INCLUDE_DIRS}
)

MESSAGE (${PROTOBUF_INCLUDE_DIRS})

target_link_libraries (videostream
    ${GTKMM_LIBRARIES}
    ${NOPOLL_LIBRARIES}
    ${FFMPEG_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${SIGC++_LIBRARIES}
)

MESSAGE (${PROTOBUF_LIBRARIES})

add_dependencies (videostream protoc)

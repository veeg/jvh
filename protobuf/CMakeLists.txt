set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

file (GLOB proto_source ${CMAKE_CURRENT_SOURCE_DIR}/*.proto)
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
    --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
    --cpp_out=${CMAKE_BINARY_DIR}/protobuf
    --js_out=library=videostreampb,binary:${CMAKE_BINARY_DIR}/protobuf
    ${proto_source}
    DEPENDS ${proto_source}
    COMMENT "Generating code from proto files"
)
set(VIDEOSTREAM_PROTO videostream_proto)

set(_VIDEOSTREAM_PROTOCPP ${CMAKE_BINARY_DIR}/protobuf/*.cpp)

add_custom_target(protoc ALL DEPENDS ${pcode} ${pheaders})

message (${pcode})

add_library(${VIDEOSTREAM_PROTO} SHARED ${pcode})

target_include_directories (${VIDEOSTREAM_PROTO} PUBLIC ${CMAKE_BINARY_DIR}/protobuf/)

target_link_libraries (${VIDEOSTREAM_PROTO} ${PROTOBUF_LIBRARIES})

enable_testing ()

find_package (GTest REQUIRED)

set (SERVER_TEST test-server-jvh)

file (GLOB SERVER_TEST_SOURCES "*.cc")

add_executable (${SERVER_TEST} ${SERVER_TEST_SOURCES})

set_property (TARGET ${SERVER_TEST} PROPERTY CXX_STANDARD 11)
set_property (TARGET ${SERVER_TEST} PROPERTY CXX_STANDARD_REQUIRED ON)

add_definitions ("-Wall")
add_definitions ("-Werror")

target_include_directories (${SERVER_TEST} PRIVATE
    ${CMAKE_BINARY_DIR}/protobuf/
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries (${SERVER_TEST} ${GTEST_BOTH_LIBRARIES})
target_link_libraries (${SERVER_TEST} videostream)
target_link_libraries (${SERVER_TEST} pthread)

add_test (VideostreamTest ${SERVER_TEST})


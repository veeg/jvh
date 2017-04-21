
add_executable(run
        "main.cc"
)

target_include_directories(run PRIVATE
    ${VIDEOSTREAM_INCLUDE_DIRS}
)

target_link_libraries(run videostream)

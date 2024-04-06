# src files. For inlined headers, include header as a source
set(lib-srcs
        app/main.cpp
        airnow/record.hpp
)

# library targets
find_package(Boost REQUIRED COMPONENTS serialization mpi)

add_library(basic_socket ${lib-srcs})
TARGET_LINK_LIBRARIES(basic_socket ${Boost_LIBRARIES} ${SOCKET_LIBRARY})

# dependencies
set(BUILD_LIBRARIES basic_socket ${Boost_LIBRARIES} ${SOCKET_LIBRARY} pthread)


# app targets
add_executable(main app/main.cpp)
target_link_libraries(main ${BUILD_LIBRARIES})

# installs: cmake --target install --config Release
install(TARGETS main CONFIGURATIONS Release
        DESTINATION release)


# for clang
set_target_properties(main PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
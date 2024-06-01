cmake_minimum_required(VERSION 3.15)
project(bb CXX)

find_package(httplib REQUIRED CONFIG)
find_package(Doxygen)

if (DOXYGEN_FOUND)
    doxygen_add_docs(
        bb_docs
        ALL
        ${PROJECT_SOURCE_DIR}
        COMMENT "Generate HTML documentation with Doxygen"
    )
    install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
        DESTINATION doc
    )
else ()
    message(WARNING "Doxygen not found. Documentation will not be generated.")
endif()

add_library(bb_service src/service.cpp src/handlers.cpp)
target_include_directories(bb_service PUBLIC include)
target_link_libraries(bb_service httplib::httplib)
set_target_properties(bb_service PROPERTIES PUBLIC_HEADER "include/service.h")

add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(bb PRIVATE include)
target_link_libraries(${PROJECT_NAME} bb_service)

if (NOT BUILD_TESTING STREQUAL OFF)
    add_subdirectory(tests)
endif()

install(TARGETS bb_service)
install(TARGETS bb)

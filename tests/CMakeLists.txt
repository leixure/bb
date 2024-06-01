cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(GTest REQUIRED CONFIG)

add_executable(test_bb html.cpp service.cpp)
target_include_directories(test_bb PRIVATE ../include)
target_link_libraries(test_bb GTest::gmock GTest::gtest GTest::gtest_main)

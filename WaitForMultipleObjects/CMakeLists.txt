cmake_minimum_required(VERSION 3.10)
project(WaitForMultipleObjectsBugTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(WaitBugTest WaitBugTest.cpp)

# Link against Windows libraries
target_link_libraries(WaitBugTest)

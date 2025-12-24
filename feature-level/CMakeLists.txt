cmake_minimum_required(VERSION 3.16)
project(FeatureLevelChecker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-only project
if(NOT WIN32)
    message(FATAL_ERROR "This project only supports Windows")
endif()

add_executable(${PROJECT_NAME} main.cpp)

# Link DirectX libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11
    d3d12
    dxgi
)

# Set subsystem to console for printf output
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

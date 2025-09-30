cmake_minimum_required(VERSION 3.10)

project(PixelModel2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)

# --- Source Files ---
add_executable(PixelModel2 
    src/main.cpp
    src/i960.cpp
    src/memory.cpp
    src/tgp.cpp
)

add_executable(PixelModel2Minimal
    src/main_minimal.cpp
    src/i960.cpp
    src/memory.cpp
    src/tgp.cpp
)

add_executable(PixelModel2Test
    src/test_memory.cpp
    src/memory.cpp
    src/tgp.cpp
)

add_executable(SimpleTest
    src/test.cpp
)

add_executable(TestInit0
    src/test_init_0.cpp
)

add_executable(TestInit1
    src/test_init_1.cpp
)

add_executable(TestInit2
    src/test_init_2.cpp
)

add_executable(TestInit3
    src/test_init_3.cpp
)

add_executable(PixelModel2LogicalTest
    src/main_test_logical.cpp
    src/i960.cpp
    src/memory.cpp
    src/tgp.cpp
)

add_executable(PixelModel2InterruptTest
    src/main_test_interrupt.cpp
    src/i960.cpp
    src/memory.cpp
    src/tgp.cpp
)

add_executable(PixelModel2TGPTest
    src/main_test_tgp.cpp
    src/i960.cpp
    src/memory.cpp
    src/tgp.cpp
)

add_executable(PixelModel2TGP3DTest
    src/main_test_tgp_3d.cpp
    src/i960.cpp
    src/memory.cpp
    src/tgp.cpp
)

# --- Linking ---
target_link_libraries(PixelModel2 PRIVATE 
    SDL3::SDL3
    opengl32
)

target_link_libraries(PixelModel2Test PRIVATE 
    opengl32
)

target_link_libraries(PixelModel2LogicalTest PRIVATE 
    opengl32
)

target_link_libraries(PixelModel2InterruptTest PRIVATE 
    opengl32
)

target_link_libraries(PixelModel2TGPTest PRIVATE 
    opengl32
)

target_link_libraries(PixelModel2TGP3DTest PRIVATE 
    opengl32
)

target_link_libraries(PixelModel2Minimal PRIVATE 
    opengl32
)

# --- Include Directories ---
target_include_directories(PixelModel2 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SDL3_INCLUDE_DIRS}
)

target_include_directories(PixelModel2Test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(PixelModel2LogicalTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(PixelModel2InterruptTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(PixelModel2TGPTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(TestInit1 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(TestInit2 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(TestInit3 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(PixelModel2Minimal PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(SimpleTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(PixelModel2TGP3DTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(TestInit0 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

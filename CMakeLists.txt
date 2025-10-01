cmake_minimum_required(VERSION 3.10)

project(PixelModel2 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prefer GLVND over legacy OpenGL
set(OpenGL_GL_PREFERENCE GLVND)

# Find OpenGL (optional - most platforms have it)
find_package(OpenGL QUIET)

if(OPENGL_FOUND)
    message(STATUS "OpenGL found - building with graphics support")
else()
    message(STATUS "OpenGL not found - attempting to build anyway (may fail on some targets)")
endif()

# SDL3 is optional - only required for the main PixelModel2 executable
find_package(SDL3 QUIET)

# Ensure upstream miniz headers are found before local include/ copies
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniz-3.1.0)

# --- Third-party libs ---
## Use miniz from the bundled third_party/miniz-3.1.0 directory (clean upstream tree)
add_library(third_party_miniz STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniz-3.1.0/miniz.c
)
target_include_directories(third_party_miniz PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniz-3.1.0
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
## using vendored upstream miniz; no external zlib required

# --- Core Library ---
# Create a library for core emulation components to avoid recompilation
add_library(pixel_model2_core STATIC
    src/core/i960.cpp
    src/core/memory.cpp
    src/core/tgp.cpp
)
target_include_directories(pixel_model2_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(pixel_model2_core PUBLIC third_party_miniz)

# --- Main Executables ---
# Main executable (requires SDL3)
if(SDL3_FOUND)
    add_executable(PixelModel2 
        src/main/main.cpp
    )
    target_link_libraries(PixelModel2 PRIVATE pixel_model2_core)
    message(STATUS "SDL3 found - building PixelModel2 with graphics support")
else()
    message(STATUS "SDL3 not found - skipping PixelModel2 main executable (building tests only)")
endif()

add_executable(PixelModel2Minimal
    src/main/main_minimal.cpp
)
target_link_libraries(PixelModel2Minimal PRIVATE pixel_model2_core)

# --- Test Executables ---
add_executable(PixelModel2Test
    tests/test_memory.cpp
)
target_link_libraries(PixelModel2Test PRIVATE pixel_model2_core)

add_executable(SimpleTest
    tests/test.cpp
)

add_executable(ZipExtractTest
    tests/test_zip_extract.cpp
    src/core/memory.cpp
    src/core/tgp_stub.cpp
)
target_link_libraries(ZipExtractTest PRIVATE third_party_miniz)
target_include_directories(ZipExtractTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(LoadMemoryTest
    tests/test_load_memory.cpp
    src/core/memory.cpp
    src/core/tgp_stub.cpp
)
target_link_libraries(LoadMemoryTest PRIVATE third_party_miniz)
target_include_directories(LoadMemoryTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(TestInit0
    tests/test_init_0.cpp
)

add_executable(TestInit1
    tests/test_init_1.cpp
)

add_executable(TestInit2
    tests/test_init_2.cpp
)

add_executable(TestInit3
    tests/test_init_3.cpp
)

add_executable(PixelModel2LogicalTest
    tests/main_test_logical.cpp
)
target_link_libraries(PixelModel2LogicalTest PRIVATE pixel_model2_core)

add_executable(PixelModel2InterruptTest
    tests/main_test_interrupt.cpp
)
target_link_libraries(PixelModel2InterruptTest PRIVATE pixel_model2_core)

add_executable(PixelModel2TGPTest
    tests/main_test_tgp.cpp
)
target_link_libraries(PixelModel2TGPTest PRIVATE pixel_model2_core)

add_executable(PixelModel2TGP3DTest
    tests/main_test_tgp_3d.cpp
)
target_link_libraries(PixelModel2TGP3DTest PRIVATE pixel_model2_core)

# --- Linking OpenGL to core library ---
if(OPENGL_FOUND)
    target_link_libraries(pixel_model2_core PUBLIC OpenGL::GL)
elseif(WIN32)
    target_link_libraries(pixel_model2_core PUBLIC opengl32)
endif()

# --- Linking SDL3 to main executable ---
if(SDL3_FOUND)
    if(TARGET SDL3::SDL3)
        target_link_libraries(PixelModel2 PRIVATE SDL3::SDL3)
    else()
        # Fallbacks for SDKs that provide SDL3 headers/libs but not CMake imported targets
        set(_linked FALSE)
        if(DEFINED SDL3_LIBRARIES)
            # SDL3_LIBRARIES may contain imported-target names (e.g. "SDL3::SDL3") or file paths.
            foreach(_sdl_lib IN LISTS SDL3_LIBRARIES)
                if(TARGET ${_sdl_lib})
                    target_link_libraries(PixelModel2 PRIVATE ${_sdl_lib})
                    set(_linked TRUE)
                elseif(EXISTS "${_sdl_lib}")
                    target_link_libraries(PixelModel2 PRIVATE "${_sdl_lib}")
                    set(_linked TRUE)
                endif()
            endforeach()
        endif()
        if(NOT _linked AND DEFINED SDL3_LIBRARY)
            if(TARGET ${SDL3_LIBRARY})
                target_link_libraries(PixelModel2 PRIVATE ${SDL3_LIBRARY})
                set(_linked TRUE)
            elseif(EXISTS "${SDL3_LIBRARY}")
                target_link_libraries(PixelModel2 PRIVATE "${SDL3_LIBRARY}")
                set(_linked TRUE)
            endif()
        endif()
        if(NOT _linked)
            # Try linking the MSVC import library from common vendor locations as a last resort
            if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3/lib/x64/SDL3.lib")
                target_link_libraries(PixelModel2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3/lib/x64/SDL3.lib")
                set(_linked TRUE)
            elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL3/lib/x64/SDL3.lib")
                target_link_libraries(PixelModel2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL3/lib/x64/SDL3.lib")
                set(_linked TRUE)
            endif()
        endif()
        if(NOT _linked)
            message(FATAL_ERROR "SDL3 was detected but no usable target, imported target, or library file was found. Please install SDL3 dev files or adjust CMAKE_PREFIX_PATH.")
        endif()
    endif()
endif()

# --- Include Directories ---
if(SDL3_FOUND)
    target_include_directories(PixelModel2 PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${SDL3_INCLUDE_DIRS}
    )
    
    # If SDL3::Headers target was created by the SDL3 package, also add it as an interface include.
    if(TARGET SDL3::Headers)
        target_include_directories(PixelModel2 PRIVATE $<TARGET_PROPERTY:SDL3::Headers,INTERFACE_INCLUDE_DIRECTORIES>)
    else()
        # Fallback include paths - common locations inside this repo or vendor folder
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL3/include/SDL3")
            target_include_directories(PixelModel2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL3/include")
        elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3/include/SDL3")
            target_include_directories(PixelModel2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL3/include")
        endif()
    endif()
endif()

target_include_directories(PixelModel2Minimal PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(SimpleTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(TestInit0 PRIVATE 
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

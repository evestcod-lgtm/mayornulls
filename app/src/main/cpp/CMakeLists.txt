cmake_minimum_required(VERSION 3.22.1)
project(nullsmod)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dobby hooking library (inline, no submodule for simplicity)
# In production: add_subdirectory(Dobby)
# Here we use a lightweight hand-rolled trampoline hook

add_library(
    nullsmod
    SHARED
    main.cpp
    hooks.cpp
    il2cpp_resolver.cpp
    features/aimbot.cpp
    features/autododge.cpp
    features/xray.cpp
    features/graphics.cpp
    features/esp.cpp
    features/fps_unlocker.cpp
    features/esports.cpp
)

target_include_directories(nullsmod PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(nullsmod
    ${log-lib}
    ${android-lib}
)

target_compile_options(nullsmod PRIVATE
    -O2
    -fvisibility=hidden
    -ffunction-sections
    -fdata-sections
    -Wno-error
)

target_link_options(nullsmod PRIVATE
    -Wl,--gc-sections
    -Wl,--strip-all
)

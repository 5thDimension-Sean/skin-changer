cmake_minimum_required(VERSION 3.20)
project(ext-cs2-skin-changer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sources
set(SRC_FILES
    "src/main.cpp"
    "ext/ImGui 1.90/imgui.cpp"
    "ext/ImGui 1.90/imgui_demo.cpp"
    "ext/ImGui 1.90/imgui_draw.cpp"
    "ext/ImGui 1.90/imgui_impl_dx11.cpp"
    "ext/ImGui 1.90/imgui_impl_win32.cpp"
    "ext/ImGui 1.90/imgui_tables.cpp"
    "ext/ImGui 1.90/imgui_widgets.cpp"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/ext"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/ImGui 1.90"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext/json/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CONSOLE)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /permissive- /Zc:__cplusplus)
    target_link_libraries(${PROJECT_NAME} PRIVATE d3d11 dxgi)
endif()

# Try to find libcurl (install via vcpkg or system package manager)
find_package(CURL QUIET)
if(CURL_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})
else()
    message(STATUS "libcurl not found — install it (vcpkg recommended). See README notes.")
endif()

message(STATUS "Configured ${PROJECT_NAME} — sources: ${SRC_FILES}")

cmake_minimum_required(VERSION 3.20)
project(ext-cs2-skin-changer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source tree lives in the ext-cs2-skin-changer subdirectory
set(APP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/ext-cs2-skin-changer")

# Sources
set(SRC_FILES
    "${APP_ROOT}/src/main.cpp"
    "${APP_ROOT}/ext/ImGui 1.90/imgui.cpp"
    "${APP_ROOT}/ext/ImGui 1.90/imgui_demo.cpp"
    "${APP_ROOT}/ext/ImGui 1.90/imgui_draw.cpp"
    "${APP_ROOT}/ext/ImGui 1.90/imgui_impl_dx11.cpp"
    "${APP_ROOT}/ext/ImGui 1.90/imgui_impl_win32.cpp"
    "${APP_ROOT}/ext/ImGui 1.90/imgui_tables.cpp"
    "${APP_ROOT}/ext/ImGui 1.90/imgui_widgets.cpp"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${APP_ROOT}/ext"
    "${APP_ROOT}/ext/ImGui 1.90"
    "${APP_ROOT}/ext/json/include"
    "${APP_ROOT}/src"
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

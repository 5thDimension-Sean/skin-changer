cmake_minimum_required(VERSION 3.20)
project(ext-cs2-skin-changer CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")

# Main project directory
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/ext-cs2-skin-changer")

# ImGui sources
set(IMGUI_DIR "${PROJECT_ROOT}/ext/ImGui 1.90")
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_impl_dx11.cpp"
    "${IMGUI_DIR}/imgui_impl_win32.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
)

# Main project sources
set(PROJECT_SOURCES
    "${PROJECT_ROOT}/src/main.cpp"
    ${IMGUI_SOURCES}
)

# Create executable
add_executable(ext-cs2-skin-changer ${PROJECT_SOURCES})

# Include directories
target_include_directories(ext-cs2-skin-changer PRIVATE
    "${PROJECT_ROOT}"
    "${PROJECT_ROOT}/ext"
    "${PROJECT_ROOT}/ext/ImGui 1.90"
    "${PROJECT_ROOT}/ext/json/include"
    "${PROJECT_ROOT}/src"
)

# Link libraries
target_link_libraries(ext-cs2-skin-changer PRIVATE
    d3d11.lib
    dxgi.lib
)

# Preprocessor definitions
target_compile_definitions(ext-cs2-skin-changer PRIVATE
    $<$<CONFIG:Debug>:_DEBUG;_CONSOLE>
    $<$<CONFIG:Release>:NDEBUG;_CONSOLE>
    WIN32
    _WINDOWS
)

# Compiler options
if(MSVC)
    target_compile_options(ext-cs2-skin-changer PRIVATE /W3 /WX-)
    # Enable whole program optimization for Release builds
    target_compile_options(ext-cs2-skin-changer PRIVATE $<$<CONFIG:Release>:/GL>)
    target_link_options(ext-cs2-skin-changer PRIVATE $<$<CONFIG:Release>:/LTCG>)
endif()

# Windows specific settings
set_target_properties(ext-cs2-skin-changer PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    WIN32_EXECUTABLE FALSE
)

include(FetchContent)

FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG release-3.2.18
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
set(SDL3_MAIN OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)

find_package(OpenGL REQUIRED)

FetchContent_Declare(ImGui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG v1.92.1-docking
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(ImGui)
FetchContent_GetProperties(ImGui SOURCE_DIR IMGUI_DIR)
set(IMGUI_BACKENDS_DIR "${IMGUI_DIR}/backends")

add_library(imgui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_BACKENDS_DIR}/imgui_impl_sdl3.cpp
        ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_BACKENDS_DIR}
        ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(imgui PUBLIC SDL3::SDL3 ${OPENGL_LIBRARIES})

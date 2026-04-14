set(PROJECT_SOURCES
        main.cpp
        benchmark/Benchmark.cpp
        renderers/CpuRenderer.cpp
        renderers/GpuRenderer.cpp
        renderers/RenderCommon.cpp
)

set(PROJECT_HEADERS
        benchmark/Benchmark.h
        renderers/IRenderer.h
        renderers/CpuRenderer.h
        renderers/GpuRenderer.h
        renderers/RenderCommon.h
)

include_directories("${GLFW_SOURCE_DIR}/deps")
set(GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h")

add_executable(${PROJECT_NAME} WIN32
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        ${GLAD_GL}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        imgui
        glfw
        ${OPENGL_LIBRARIES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        PDS_PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

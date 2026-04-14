set(PROJECT_SOURCES
        main.cpp
        
)

set(PROJECT_HEADERS
        
)

add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        imgui
        SDL3::SDL3
        ${OPENGL_LIBRARIES}

)
SET(SOURCES

    Framebuffer.cpp
    Shader.cpp
    Texture.cpp
    VertexArray.cpp
    TextureArray.cpp
    Cubemap.cpp
)


add_library(GL ${SOURCES})
target_link_libraries(GL 
    GLAD 
    Util 
    ${X11_LIBRARIES} 
    ${OPENGL_LIBRARIES} 
    ${CMAKE_DL_LIBS} 
    ${SDL2_LIBRARIES} 
    Threads::Threads
)
target_include_directories(GL PUBLIC 
    include 
    ${OPENGL_INCLUDE_DIR} 
    ${GLAD_INCLUDE_DIR} 
    ${DEPS_FOLDER} 
    ${CMAKE_CURRENT_SOURCE_DIR}
)
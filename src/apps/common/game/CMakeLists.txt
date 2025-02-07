SET(SOURCES

    CameraController.cpp
    Map.cpp
    ServiceLocator.cpp
    TexturePalette.cpp
    ColorPalette.cpp
    ChunkMeshBuilder.cpp
    ChunkMeshMgr.cpp
    Input.cpp
    MapMgr.cpp

    models/Player.cpp
    models/Explosion.cpp
    models/FPS.cpp
    models/Respawn.cpp
    models/ModelMgr.cpp

    sound/Gallery.cpp
)


add_library(Game ${SOURCES})
target_link_libraries(Game 
    GLAD 
    ${X11_LIBRARIES} 
    ${OPENGL_LIBRARIES} 
    ${CMAKE_DL_LIBS} 
    ${SDL2_LIBRARIES} 
    Threads::Threads 
    GL 
    Rendering
    Collisions 
    Animation
    Math
    Entity
    Audio

    Log
)
target_include_directories(Game PUBLIC include 
    ${OPENGL_INCLUDE_DIR}  
    ${GLAD_INCLUDE_DIR} 
    ${DEPS_FOLDER} 
    ${DEPS_FOLDER}/openal-soft/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL2_INCLUDE_DIRS}
)
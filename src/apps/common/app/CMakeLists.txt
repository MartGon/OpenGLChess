SET(SOURCES

    App.cpp
    Configuration.cpp
    VideoSettingsPopUp.cpp
)


add_library(App ${SOURCES})
target_link_libraries(App
    imgui
    GLAD 
    ${X11_LIBRARIES} 
    ${OPENGL_LIBRARIES} 
    ${CMAKE_DL_LIBS} 
    ${SDL2_LIBRARIES} 
    Threads::Threads 
    GL

    Log
)
target_include_directories(App PUBLIC include 
    ${OPENGL_INCLUDE_DIR}  
    ${GLAD_INCLUDE_DIR} 
    ${DEPS_FOLDER} 
    ${DEPS_FOLDER}/imgui 
    ${APPS_COMMON_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL2_INCLUDE_DIR}
)
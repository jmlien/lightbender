set(LIGHTBENDER_SRC
    ${LIGHTBENDER_SRC}
	${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/draw_basics.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/model.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shader.cpp
    PARENT_SCOPE)

#add_subdirectory("${CMAKE_SOURCE_DIR}/src/mascgl-3/soil")

include_directories(
    "${CMAKE_SOURCE_DIR}/src/mascgl-3"
    "${CMAKE_SOURCE_DIR}/src/mascgl-3/soil"
)

#####################################################
#                COMMON Libraries                   #
#####################################################

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLM REQUIRED)
find_package(GLEW REQUIRED STATIC)

if (APPLE)
include_directories(/usr/local/include)
endif (APPLE)

include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS})

########## add subdirectories ############
add_subdirectory(mathtool)
add_subdirectory(mascgl-3)
add_subdirectory(mascgl-3/soil)

#############################################
# light bender
#############################################

set(LIGHTBENDER_SRC
    ${LIGHTBENDER_SRC}
    lightbender.cpp
    main.cpp)

set(APP_NAME lightbender)

include_directories(
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/mathtool"
    "${CMAKE_SOURCE_DIR}/src/modelgraph"
    "${CMAKE_SOURCE_DIR}/src/mascgl-3")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
add_executable(${APP_NAME} ${LIGHTBENDER_SRC})
target_link_libraries(${APP_NAME}
                      ${OPENGL_LIBRARIES}
                      "-framework CoreFoundation"
                      glfw GLEW::glew_s)

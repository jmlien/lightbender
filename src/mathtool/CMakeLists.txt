set(LIGHTBENDER_SRC
    ${LIGHTBENDER_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/Basic.cpp
    PARENT_SCOPE)

include_directories(
    "${CMAKE_SOURCE_DIR}/src/mathtool"
)

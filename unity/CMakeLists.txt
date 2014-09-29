project(Unity C)

set(${PROJECT_NAME}_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include dirs" FORCE
)

set(${PROJECT_NAME}_LIBRARIES
    unity
    CACHE INTERNAL "${PROJECT_NAME}: Libraries" FORCE
)

set(SRC_FILES
    unity.c
    unity_fixture.c
)

include_directories(
    ${${PROJECT_NAME}_INCLUDE_DIRS}
)

add_library(unity STATIC ${SRC_FILES})

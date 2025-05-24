cmake_minimum_required(VERSION 3.10)
project(CheckersGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

include_directories(Checkers)

set(SOURCE_FILES
    main.cpp
    Checkers/checkers.cpp
    Checkers/checkers.h
)

add_executable(CheckersGame ${SOURCE_FILES})

target_link_libraries(CheckersGame
    sfml-graphics
    sfml-window
    sfml-system
)

if(WIN32)
    add_custom_command(TARGET CheckersGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SFML_DIR}/../bin/sfml-graphics-2.dll"
            "${SFML_DIR}/../bin/sfml-window-2.dll"
            "${SFML_DIR}/../bin/sfml-system-2.dll"
            $<TARGET_FILE_DIR:CheckersGame>
    )
endif()

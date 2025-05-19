cmake_minimum_required(VERSION 3.10)
project(VisualIllusion)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Создание исполняемого файла
add_executable(VisualIllusion main.cpp)

# Подключение SFML
target_link_libraries(VisualIllusion
    sfml-graphics
    sfml-window
    sfml-system
)

# Для Windows: копирование DLL (опционально)
if(WIN32)
    add_custom_command(TARGET VisualIllusion POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SFML_DIR}/../bin/sfml-graphics-2.dll"
            "${SFML_DIR}/../bin/sfml-window-2.dll"
            "${SFML_DIR}/../bin/sfml-system-2.dll"
            $<TARGET_FILE_DIR:VisualIllusion>
    )
endif()

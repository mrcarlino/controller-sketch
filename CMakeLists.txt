cmake_minimum_required(VERSION 3.10)
project(ControllerSketch)

set(CMAKE_CXX_STANDARD 17)

if (MINGW)
    # This ensures the executable opens a console window
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(SDL2 CONFIG REQUIRED)

qt_standard_project_setup()

add_executable(ControllerSketch
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/RenderCanvas.cpp
    src/RenderCanvas.h
    src/ControllerInput.cpp
    src/ControllerInput.h
    src/Brush.cpp
    src/Brush.h
)

target_link_libraries(ControllerSketch
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    SDL2::SDL2
)
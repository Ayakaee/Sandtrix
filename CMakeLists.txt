cmake_minimum_required(VERSION 3.10) # CMake install : https://cmake.org/download/
set(CMAKE_CXX_COMPILER "D:/SoftwareInstall/Qt/Tools/mingw1310_64/bin/g++.exe")
project(sandtrix LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "D:/SoftwareInstall/Qt/6.8.1/mingw_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 COMPONENTS Widgets REQUIRED) # Qt COMPONENTS
aux_source_directory(./src srcs)
qt_add_resources(RESOURCES resource.qrc)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_SOURCE_DIR}/app_icon.rc")

add_executable(${PROJECT_NAME}
    WIN32 # If you need a terminal for debug, please comment this statement 
    ${srcs} 
    D:/Code/C++/Qt/sandtrix/src/modewidget.h
) 
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets) # Qt5 Shared Library

target_sources(${PROJECT_NAME} PRIVATE ${RESOURCES} ${APP_ICON_RESOURCE_WINDOWS})
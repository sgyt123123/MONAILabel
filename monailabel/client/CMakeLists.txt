cmake_minimum_required(VERSION 3.5)

project(monailabel LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("monailabelclient/" "monailabelclient/utils")
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5Widgets_PRIVATE_INCLUDE_DIRS})
add_subdirectory(monailabelclient)
aux_source_directory(. DIR_SRCS)
add_executable(monailabel_ui ${DIR_SRCS})
target_link_libraries(monailabel_ui Qt5::Core Qt5::Widgets monaiclient) 



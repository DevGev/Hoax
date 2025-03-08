cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS "-O3")

project(hoax)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(FLTK REQUIRED)



add_executable(hoax source/tools/proclist.cpp source/main.cpp source/hoax/ui.cpp source/hoax/subwindows/scanner.cpp source/hoax/subwindows/input_line.cpp source/hoax/subwindows/process_picker.cpp source/hoax/subwindows/address_modifier.cpp source/hoax/subwindows/memory_viewer.cpp source/hoax/subwindows/address_actions.cpp source/hoax/subwindows/notepad.cpp)
target_link_libraries(hoax PRIVATE Threads::Threads scanmem fltk fltk_images pthread ${CMAKE_DL_LIBS})

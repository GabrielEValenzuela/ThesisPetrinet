cmake_minimum_required(VERSION 3.20)

#Set up project
project(PetrinetSE
VERSION 0.1
DESCRIPTION "A simulator and emulator for petri network based on modern C++"
LANGUAGES CXX)
message("Generating the project...")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/date")
file(GLOB SOURCE_FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/date/*.cpp"
    )
add_executable(Petrinet ${SOURCE_FILES} )
set_property(TARGET Petrinet PROPERTY CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(Petrinet PRIVATE Threads::Threads)
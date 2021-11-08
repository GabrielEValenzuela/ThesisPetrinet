cmake_minimum_required(VERSION 3.20)

#Set up project
project(PetrinetSE
VERSION 2.1
DESCRIPTION "A simulator and emulator for petri network based on modern C++"
LANGUAGES CXX)
message("Generating the project...")
set(ENABLE_PROF FALSE)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/date")
file(GLOB SOURCE_FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/date/*.cpp"
    )
add_executable(${PROJECT_NAME} ${SOURCE_FILES} )
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
if(ENABLE_PROF)
message("Generating profiling output...")
set(TRACY_PATH "./lib/tracy")
add_library(tracy STATIC ${TRACY_PATH}/TracyClient.cpp)
target_compile_features(tracy PRIVATE cxx_std_17)
target_include_directories(tracy PUBLIC ${TRACY_PATH})
add_definitions(-DTRACY_ENABLE=0)
target_link_libraries(${PROJECT_NAME} PUBLIC tracy)
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_DL_LIBS})
target_link_libraries(${PROJECT_NAME} PUBLIC tbb)
message("Profiling done. You should start tracy sever now")
endif()


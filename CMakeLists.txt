cmake_minimum_required(VERSION 3.21)
project(bench1 LANGUAGES CXX)

add_executable(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/bench1.cpp)

add_subdirectory(benchmark)
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
						CMAKE_CXX_STANDARD_REQUIRED ON
						CXX_STANDARD 20
						CMAKE_CXX_EXTENSIONS OFF
						LINKER_LANGUAGE CXX)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(AnyAny)

target_link_libraries(${CMAKE_PROJECT_NAME} anyanylib benchmark::benchmark)
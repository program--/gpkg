cmake_minimum_required(VERSION 3.14)
project(gpkg VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(EXTERN_DIR ${CMAKE_CURRENT_LIST_DIR}/extern)
set(TEST_DIR ${CMAKE_CURRENT_LIST_DIR}/tests)
set(MODULE_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)

list(APPEND CMAKE_MODULE_PATH ${MODULE_DIR})

add_library(gpkg INTERFACE)
target_include_directories(gpkg INTERFACE ${INCLUDE_DIR})

# Dependencies
find_package(SQLite3 REQUIRED)
if(SQLITE3_LIBRARY)
    target_include_directories(sqlite3 INTERFACE ${SQLITE3_INCLUDE})
    target_link_libraries(gpkg INTERFACE sqlite3)
    message(INFO " dependency found: SQLite3 -> ${SQLITE3_LIBRARY}")
else()
    # maybe fetch it?
    message(WARN " dependency not found: SQLite3")
endif()

# Development Dependencies
option(ENABLE_TESTS "include tests" ON)
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND ENABLE_TESTS)
    add_subdirectory(tests)
endif()

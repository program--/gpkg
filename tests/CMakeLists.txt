project(tests)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB TESTFILES "*_test.cpp")
add_library(test_sources OBJECT utils.hpp ${TESTFILES})
target_link_libraries(test_sources gpkg)
target_link_libraries(test_sources doctest::doctest)

add_executable(tests main.cpp)
target_compile_features(tests PRIVATE cxx_std_14)
target_link_libraries(tests test_sources)
target_link_libraries(tests doctest::doctest)

include(CTest)
include(Doctest)

enable_testing()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/data)
include_directories(${INCLUDE_DIR})
include_directories(${DOCTEST_INCLUDE_DIR})
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(tests)

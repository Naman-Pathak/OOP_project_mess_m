cmake_minimum_required(VERSION 3.10)
project(MessFoodWasteTracker CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra)
endif()

include_directories(include)

# Everything except main() lives in a library so the tests can reuse it.
add_library(messlib
  src/Utils.cpp
  src/FileHandler.cpp
  src/Tracker.cpp
  src/Analyzer.cpp
  src/Forecaster.cpp
  src/Optimizer.cpp
  src/Report.cpp)

add_executable(mess_tracker src/main.cpp)
target_link_libraries(mess_tracker messlib)

add_executable(generate_sample_data tools/generate_sample_data.cpp)
target_link_libraries(generate_sample_data messlib)

enable_testing()
foreach(name utils filehandler analyzer forecaster optimizer menu)
  add_executable(test_${name} tests/test_${name}.cpp)
  target_link_libraries(test_${name} messlib)
  add_test(NAME ${name} COMMAND test_${name})
endforeach()

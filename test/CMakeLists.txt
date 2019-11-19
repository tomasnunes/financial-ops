add_library(Catch INTERFACE)

target_include_directories(Catch INTERFACE ../lib/catch2)

set(TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests.cpp ${CMAKE_CURRENT_SOURCE_DIR}/integration_tests.cpp)

add_executable(process_operations_tests ${TEST_SOURCES})
target_compile_features(process_operations_tests PRIVATE cxx_std_17)
target_link_libraries(process_operations_tests Catch process_operations)
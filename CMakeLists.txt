# This requires a recent nightly build.
# This will be part of CMake 3.30.0.
cmake_minimum_required(VERSION 3.29.0 FATAL_ERROR)

# Enables the Standard module support. This needs to be done
# before selecting the languages.
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
set(CMAKE_CXX_MODULE_STD ON)

project("VulkanApp"
  LANGUAGES CXX
)

# Set language version used
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS ON)

# Create the executable target
add_executable(VulkanApp main.cpp)

file(GLOB_RECURSE MODULE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/Modules/*.ixx"
)

function(add_common_settings TARGET)
  target_sources(${TARGET}
    PUBLIC
    FILE_SET CXX_MODULES TYPE CXX_MODULES
    BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES ${MODULE_FILES}
  )
  
  target_include_directories(${TARGET}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Modules
  )
endfunction()

add_common_settings(VulkanApp)

# Add compiler options to each compilation unit.
get_property(TARGETS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
foreach(TARGET ${TARGETS})
    target_compile_options(${TARGET} PRIVATE -fno-exceptions)
endforeach()

## TESTING

enable_testing()
function(add_tests TEST_DIR)
  file(GLOB TEST_SOURCES "${TEST_DIR}/*.cpp")
  foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    add_common_settings(${TEST_NAME})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
  endforeach()
endfunction()
add_tests(Tests/ECS)
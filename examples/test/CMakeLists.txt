
# This is the project CMakeLists.txt file for the test subproject
cmake_minimum_required(VERSION 3.5)

# Include the components directory of the main application:
#
set(EXTRA_COMPONENT_DIRS "../../")

# Set the components to include the tests for.

set(TEST_COMPONENTS "meshino_framework")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(meshino_example_test)

cmake_minimum_required(VERSION 2.8)
project(ArRaspi)

set(CMAKE_BUILD_TYPE "Debug")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(MAIN_FILE main.cpp)
# clang does not work --> https://bugs.launchpad.net/ubuntu/+source/llvm-toolchain-3.6/+bug/1488254
SET(CMAKE_CXX_COMPILER "/usr/bin/g++")
#SET(CMAKE_EXE_LINKER_FLAGS "--coverage" ) # for clang only
set(Boost_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/boost_1_60_0)
set(Boost_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/boost_1_60_0/stage/lib)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/build/cmake-modules/")


find_package(Boost COMPONENTS system REQUIRED)

FILE(GLOB_RECURSE SOURCES_LIST sources/*.cpp)
FILE(GLOB_RECURSE TESTS_LIST tests/*.cpp)

include_directories(includes)
include_directories(${Boost_INCLUDE_DIR})

#add_definitions("-Wno-long-long -Wno-variadic-macros -Wall -Wextra -Wno-unknown-pragmas -pedantic -Os -W -pthread -std=c++14")
add_definitions("-g -O0 -Wall -W -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -pthread -std=c++14")
#add_definitions("-g -O0 -Wall -W -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -std=c++14")
add_library(arraspi ${SOURCES_LIST})

INCLUDE(CodeCoverage)
SETUP_TARGET_FOR_COVERAGE(
				lcov
				ArRaspi
				coverage
				)

SETUP_TARGET_FOR_COVERAGE_COBERTURA(
				gcovr
				ArRaspi
				coverage
				)
ADD_CUSTOM_TARGET(
	run
	${EXECUTABLE_OUTPUT_PATH}/./ArRaspi
)

ADD_CUSTOM_TARGET(
	runTests
	${EXECUTABLE_OUTPUT_PATH}/./ArRaspi_tests --gtest_output="xml:gtest.xml"
)

ADD_CUSTOM_TARGET(
    generateProtobuf
    COMMAND
    "protoc" -I../includes/Ar/Raspi/Messages/ --cpp_out=. ../includes/Ar/Raspi/Messages/RaspiMessages.proto
    COMMAND
    "mv" RaspiMessages.pb.h ../includes/Ar/Raspi/Messages/
    COMMAND
    "sed" -i "\"1i/// @date `date`\"" RaspiMessages.pb.cc
    COMMAND
    "sed" -i "'s|include \"RaspiMessages.pb.h\"|include <Ar/Raspi/Messages/RaspiMessages.pb.h>|g'" RaspiMessages.pb.cc
    COMMAND
    "mv" RaspiMessages.pb.cc ../sources/Ar/Raspi/Messages/RaspiMessages.pb.cpp

)

# Adding clang-tidy target if executable is found
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
  add_custom_target(
    clang-tidy
    COMMAND /usr/bin/clang-tidy
    ${SOURCES_LIST}
    -checks=*,-clang-analyzer-alpha.*,-llvm-include-order -header-filter=.*
    --
    -std=c++14 -I../includes -I${Boost_INCLUDE_DIR}
    )
endif()

ADD_DEPENDENCIES(arraspi generateProtobuf)

link_directories(${Boost_LIBRARY_DIR})
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/googletest/googletest/)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/googletest/googlemock/)

add_executable(ArRaspi ${MAIN_FILE})
add_executable(ArRaspi_tests ${TESTS_LIST})

target_include_directories(ArRaspi_tests PUBLIC ext/googletest/googletest/include)
target_include_directories(ArRaspi_tests PUBLIC ext/googletest/googlemock/include)

target_link_libraries(ArRaspi arraspi ${Boost_LIBRARIES} gcov pthread protobuf)
target_link_libraries(ArRaspi_tests arraspi ${Boost_LIBRARIES} gcov protobuf gmock gmock_main pthread)
# ================================================================================ #
# project macros
# ================================================================================ #
cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(MIMO-Tools CXX C)

# set default build type
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message("setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# ================================================================================ #
# language standard requirements
# ================================================================================ #
if (NOT DEFINED CMAKE_C_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED true)
endif ()

if (NOT DEFINED CMAKE_CUDA_STANDARD)
    set(CMAKE_CUDA_STANDARD 11)
    set(CMAKE_CUDA_STANDARD_REQUIRED true)
endif ()


# ================================================================================ #
# compiler flags
# ================================================================================ #
set(CMAKE_CXX_FLAGS "-w -lprotobuf -L/usr/local/lib -I/usr/local/include")

# ================================================================================ #
# 3rd party libraries
# ================================================================================ #
find_package(Armadillo REQUIRED)
include_directories(${ARMADILLO_INCLUDE_DIRS})
find_package(PCAP REQUIRED)
#find_package(Boost COMPONENTS system REQUIRED)
#include_directories(${Boost_INCLUDE_DIR})
add_subdirectory(proto)

# ================================================================================ #
# source code
# ================================================================================ #
set(UTIL_SRCS
        src/options.cc
        src/common.cc
        src/filter.cc)
set(MT_TOOL_SRCS
        src/mttool_impl.cc
        src/mttool_interface.cc)
set(dot11ndp_SRCS
        src/dot11vhtndp_utils.cc)
set(pbuf_SRCS
        src/pbuf.cc)

#set(ANALYZER_SRCS)


# ================================================================================ #
# TARGETS
# ================================================================================ #
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# build project utility functions
add_library(mt_utilities SHARED ${UTIL_SRCS} ${MT_TOOL_SRCS})
target_include_directories(mt_utilities
        PRIVATE ${PROJECT_SOURCE_DIR}/include/)

add_library(pbuf_interface ${pbuf_SRCS})
target_link_libraries(pbuf_interface
        PRIVATE ${PROTOBUF_LIBRARY}
        PRIVATE proto)
target_include_directories(pbuf_interface
        PRIVATE ${PROJECT_SOURCE_DIR}/include)

# build 802.11 ndp utilities
add_library(dot11_utilities ${dot11ndp_SRCS})
target_link_libraries(dot11_utilities
        PRIVATE ${PCAP_LIBRARY}
        PRIVATE proto
        PRIVATE ${PROTOBUF_LIBRARY})
target_include_directories(dot11_utilities
        PRIVATE ${PCAP_INCLUDE_DIR}
        PRIVATE ${PROJECT_SOURCE_DIR}/include/)

add_executable(mt_tool src/mttool_main.cc)
target_link_libraries(mt_tool
        PRIVATE mt_utilities
        PRIVATE dot11_utilities
        PRIVATE ${PCAP_LIBRARY}
        PRIVATE pbuf_interface)
target_include_directories(mt_tool
        PRIVATE ${PCAP_INCLUDE_DIR}
        PRIVATE ${PROJECT_SOURCE_DIR}/include)

# buidl MIMO analyzer functions
#add_library(mt_analyzer ANALYZER_SRCS)
#target_link_libraries(mt_analyzer
#        PRIVATE mt_utilities
#        PRIVATE ${ARMADILLO_LIBRARIES})

# ================================================================================ #
# Playground
# ================================================================================ #
add_executable(testpcap playground/testpcap.cc)
target_link_libraries(testpcap
        PRIVATE ${PCAP_LIBRARY}
        PRIVATE dot11_utilities
        PRIVATE mt_utilities
        PRIVATE pbuf_interface)
target_include_directories(testpcap
        PRIVATE ${PCAP_INCLUDE_DIR}
        PRIVATE ${PROJECT_SOURCE_DIR}/include/)

add_executable(testprotobuf playground/testprotobuf.cc)
target_link_libraries(testprotobuf
        proto
        ${PROTOBUF_LIBRARY})

add_executable(testregex playground/testregex.cc)

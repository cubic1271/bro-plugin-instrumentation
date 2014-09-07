
cmake_minimum_required(VERSION 2.6.3)

project(Plugin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include(BroPlugin)

bro_plugin_begin(Instrumentation Instrumentation)
link_libraries("-ldl")
include_directories("src/pcm")

# If on OS/X, we need PcmMsr for PCM to work ...
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   link_libraries("-lPcmMsr")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

bro_plugin_cc(src/Plugin.cc src/pcm/client_bw.cpp src/pcm/msr.cpp src/pcm/cpucounters.cpp src/pcm/pci.cpp)
bro_plugin_bif(src/consts.bif)
bro_plugin_bif(src/events.bif)
bro_plugin_bif(src/functions.bif)
bro_plugin_end()


##########
# HEADER #
##########

cmake_minimum_required(VERSION 3.10)
project(STASHCACHE VERSION 0.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
list(APPEND CMAKE_PREFIX_PATH "/home/aidanb/Builds/CacheLib/opt/cachelib/")

# Create options
option(DEV "Generate compiler commands and set logging to debug." OFF)

# Set options
set(PROJECT_BINARY_DIR "build")
set(PROJECT_SOURCE_DIR "src")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
if(DEV)
    set(CMAKE_EXPORT_COMPILE_COMMANDS True)
    set(CMAKE_BUILD_TYPE Debug)
endif()
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Configure injection header
configure_file(src/stashcacheconfig.hh.in stashcacheconfig.hh)

# Find dependencies
find_package(glog REQUIRED) # logging
find_package(cppiper REQUIRED) # logging
find_package(cachelib REQUIRED)

###############
# COMPILATION #
###############

add_library(
  stashcache
  SHARED
  src/cache.cc
  src/service.cc
  src/client.cc
  )
set_target_properties(stashcache PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(stashcache PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           include
                           "/home/aidanb/Builds/CacheLib/opt/cachelib/include"
                           )
target_link_libraries(stashcache PUBLIC cppiper::cppiper glog::glog cachelib)

################
# INSTALLATION #
################

install(TARGETS stashcache DESTINATION lib)
install(FILES "${PROJECT_BINARY_DIR}/stashcacheconfig.hh"
  DESTINATION include
)

##########
# SERVER #
##########

add_executable(stashcache-server src/server.cc)
target_include_directories(stashcache-server PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           include
                           )
target_link_libraries(stashcache-server PUBLIC stashcache)

#############
# BENCHMARK #
#############

add_executable(benchmark benchmark/benchmark.cc)
target_include_directories(benchmark PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           include
                           )
target_link_libraries(benchmark PUBLIC stashcache)

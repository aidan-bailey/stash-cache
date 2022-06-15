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
set(GCC_COVERAGE_COMPILE_FLAGS "-I/usr/lib/gcc/x86_64-pc-linux-gnu/11.2.0/include -I/usr/include/c++/11.2.0/include -I/home/aidanb/Repos/Personal/CacheLib/opt/cachelib/include -pthreads -fcoroutines")
if(DEV)
    set(CMAKE_EXPORT_COMPILE_COMMANDS True)
endif()

# Configure injection header
configure_file(src/stashcacheconfig.hh.in stashcacheconfig.hh)

# Find dependencies
find_package(spdlog REQUIRED) # logging
find_package(cppiper REQUIRED) # logging
find_package(cachelib REQUIRED)

###############
# COMPILATION #
###############

add_library(
  stashcache
  STATIC
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
target_link_libraries(stashcache PRIVATE cppiper spdlog::spdlog cachelib)

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

add_executable(server src/server.cc)
target_include_directories(server PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           include
                           )
target_link_libraries(server PUBLIC stashcache)

###########
# TESTING #
###########

add_executable(test test/test.cc)
target_include_directories(test PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           include
                           )
target_link_libraries(test PUBLIC stashcache)

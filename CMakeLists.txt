cmake_minimum_required(VERSION 2.8.7)
project(lsd_slam_core)
include(ExternalProject)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

cmake_minimum_required(VERSION 2.8.7)

if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(OpenCV 2.4 COMPONENTS core highgui imgproc calib3d REQUIRED)
find_package(Boost COMPONENTS thread filesystem system REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Pangolin 0.1 REQUIRED)
find_package(G2O REQUIRED)
find_package(SuiteParse REQUIRED) # Apparently needed by g2o
find_package(TCLAP REQUIRED )
find_package(Glew REQUIRED )
find_package(OpenGL REQUIRED )
find_package(G3Log REQUIRED)

# FabMap
# uncomment this part to enable fabmap
add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/openFabMap)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/openFabMap/include)
add_definitions("-DHAVE_FABMAP")
set(FABMAP_LIB openFABMAP )

## Architecture detection
EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message( STATUS "Architecture: ${ARCHITECTURE}" )

if( ${ARCHITECTURE} STREQUAL "aarch64" )
  set(CMAKE_CXX_FLAGS
      ## Unclear how many of these command line options are actually effective
      ## Args current for gcc 4.8.4
     "${CMAKE_CXX_FLAGS} -march=armv8-a -O3 -std=c++11"
  )
elseif( ${ARCHITECTURE} STREQUAL "x86_64" )
  # SSE flags
  add_definitions("-DENABLE_SSE")
  # set(CMAKE_CXX_FLAGS
  #    "${CMAKE_CXX_FLAGS} -msse4.1 -msse3 -msse2 -msse -march=native -O3 -std=c++11"
  # )
  set(CMAKE_CXX_FLAGS
     "${CMAKE_CXX_FLAGS} -ggdb -std=c++11"
  )
endif()

set( LSDSLAM_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/lib
  ${CMAKE_SOURCE_DIR}/lib/lsd_core
  ${EIGEN3_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIR}
  ${Pangolin_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/thirdparty/Sophus
  ${G2O_INCLUDE_DIR}
  ${CSPARSE_INCLUDE_DIR} #Has been set by SuiteParse
  ${CHOLMOD_INCLUDE_DIR} #Has been set by SuiteParse
  ${G3LOG_INCLUDE_DIR}
)
include_directories( ${LSDSLAM_INCLUDE_DIRS} )

add_subdirectory("lib/active_object")
add_subdirectory("lib/logger")
add_subdirectory("lib/lsd_core")

add_subdirectory("tools")


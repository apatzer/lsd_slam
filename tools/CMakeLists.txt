
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

# build image node
add_executable(LSD LSD.cpp
  LSDGuiThread.cpp
  LSDInputThread.cpp)
target_link_libraries(LSD lsdslam )

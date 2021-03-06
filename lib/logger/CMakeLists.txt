if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

# Set source files
set(logger_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/LogReader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/LogWriter.cpp
)

find_package( ZLIB REQUIRED )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/lib
  ${OpenCV_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIR}
  ${G3LOG_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIRS}
)

# build shared library.
add_library(logger SHARED ${logger_SOURCE_FILES})
target_link_libraries(logger
  ${Boost_LIBRARIES}
  ${OpenCV_LIBS}
  ${G3LOG_LIBRARIES}
  ${ZLIB_LIBRARIES}
  active_object
)

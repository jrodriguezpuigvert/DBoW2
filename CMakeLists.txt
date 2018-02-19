cmake_minimum_required(VERSION 3.0)
project(DBoW2)
include(ExternalProject)

add_compile_options(-std=c++11 -v)

option(BUILD_DBoW2   "Build DBoW2"            ON)
option(BUILD_Demo    "Build demo application" ON)

#Adding Lib
add_library(simple-websocket-server INTERFACE)

target_include_directories(simple-websocket-server INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(simple-websocket-server INTERFACE ${CMAKE_THREAD_LIBS_INIT})

if(USE_STANDALONE_ASIO)
  target_compile_definitions(simple-websocket-server INTERFACE USE_STANDALONE_ASIO)
  include(CheckIncludeFileCXX)
  CHECK_INCLUDE_FILE_CXX(asio.hpp HAVE_ASIO)
  if(NOT HAVE_ASIO)
    message(FATAL_ERROR "Standalone Asio not found")
  endif()
else()
  find_package(Boost 1.54.0 COMPONENTS system thread coroutine context REQUIRED)
  target_link_libraries(simple-websocket-server INTERFACE ${Boost_LIBRARIES})
  target_include_directories(simple-websocket-server INTERFACE ${Boost_INCLUDE_DIR})
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    target_compile_definitions(simple-websocket-server INTERFACE USE_BOOST_REGEX)
    find_package(Boost 1.54.0 COMPONENTS regex REQUIRED)
    target_link_libraries(simple-websocket-server INTERFACE ${Boost_LIBRARIES})
    target_include_directories(simple-websocket-server INTERFACE ${Boost_INCLUDE_DIR})
  endif()
endif()



if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif()

#OpenSSL for handshake websocket!
if(APPLE)
  set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
endif()
find_package(OpenSSL REQUIRED)

target_link_libraries(simple-websocket-server INTERFACE ${OPENSSL_LIBRARIES})
target_include_directories(simple-websocket-server INTERFACE ${OPENSSL_INCLUDE_DIR})


set(DEPENDENCY_DIR ${CMAKE_CURRENT_BINARY_DIR}/dependencies)
set(DEPENDENCY_INSTALL_DIR ${DEPENDENCY_DIR}/install)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(HDRS
        include/DBoW2/BowVector.h           include/DBoW2/FBrief.h
        include/DBoW2/QueryResults.h        include/DBoW2/TemplatedDatabase.h   include/DBoW2/FORB.h
        include/DBoW2/DBoW2.h               include/DBoW2/FClass.h              include/DBoW2/FeatureVector.h
        include/DBoW2/ScoringObject.h       include/DBoW2/TemplatedVocabulary.h include/DBoW2/MapFrame.h
        include/DBoW2/Base64.h)
set(SRCS
        src/BowVector.cpp     src/FBrief.cpp        src/FORB.cpp include/DBoW2/ws/server_ws.hpp
        include/DBoW2/ws/utility.hpp include/DBoW2/ws/status_code.hpp   include/DBoW2/ws/crypto.hpp
        src/FeatureVector.cpp src/QueryResults.cpp  src/ScoringObject.cpp src/MapFrame.cpp)

find_package(DLib QUIET 
  PATHS ${DEPENDENCY_INSTALL_DIR})
if(${DLib_FOUND})
  message("DLib library found, using it from the system")
  include_directories(${DLib_INCLUDE_DIRS})
  add_custom_target(Dependencies)
else(${DLib_FOUND})
  message("DLib library not found in the system, it will be downloaded on build")
  option(DOWNLOAD_DLib_dependency "Download DLib dependency" ON)
  if(${DOWNLOAD_DLib_dependency})
    ExternalProject_Add(DLib
      PREFIX ${DEPENDENCY_DIR}
      GIT_REPOSITORY http://github.com/dorian3d/DLib
      GIT_TAG master
      INSTALL_DIR ${DEPENDENCY_INSTALL_DIR}
      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>)
    add_custom_target(Dependencies ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} DEPENDS DLib)
  else()
    message(SEND_ERROR "Please, activate DOWNLOAD_DLib_dependency option or download manually")
  endif(${DOWNLOAD_DLib_dependency})
endif(${DLib_FOUND})

if(BUILD_DBoW2)
  add_library(${PROJECT_NAME} SHARED ${SRCS})
  include_directories(include/DBoW2/)
  add_dependencies(${PROJECT_NAME} Dependencies)
  target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${DLib_LIBS})
endif(BUILD_DBoW2)

if(BUILD_Demo)
  add_executable(demo demo/demo.cpp)
  target_link_libraries(demo ${PROJECT_NAME} ${OpenCV_LIBS} ${DLib_LIBS})
  #Adding library into exacutable.


  target_link_libraries(demo simple-websocket-server)
  find_package(Boost 1.54.0 COMPONENTS system thread filesystem REQUIRED)
  target_link_libraries(demo ${Boost_LIBRARIES})
  target_include_directories(demo PRIVATE ${Boost_INCLUDE_DIR})

  file(COPY demo/images DESTINATION ${CMAKE_BINARY_DIR}/)
endif(BUILD_Demo)

configure_file(src/DBoW2.cmake.in
  "${PROJECT_BINARY_DIR}/DBoW2Config.cmake" @ONLY)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

if(BUILD_DBoW2)
  install(DIRECTORY include/DBoW2 DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
endif()

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/DBoW2Config.cmake"
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})
install(FILES "${PROJECT_BINARY_DIR}/DBoW2Config.cmake"
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/DBoW2/)
install(DIRECTORY ${DEPENDENCY_INSTALL_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL)

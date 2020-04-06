cmake_minimum_required(VERSION 3.8)

if(DEFINED PROJECT_NAME)
  set(IRIS_SUBPROJECT ON)
endif()

project(iris VERSION 1.8.0 LANGUAGES CXX
  HOMEPAGE_URL "https://github.com/p-ranav/iris"
  DESCRIPTION "Lightweight Component Model and Messaging Framework based on ØMQ")

if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup()
endif()

option(IRIS_BUILD_TESTS "Build iris tests + enable CTest")
option(IRIS_SAMPLES "Build iris samples")
option(IRIS_DEMO "Build iris demo" OFF)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

find_package(Threads REQUIRED)

add_library(iris INTERFACE)
add_library(iris::iris ALIAS iris)

target_compile_features(iris INTERFACE cxx_std_17)
target_include_directories(iris INTERFACE
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)
target_link_libraries(iris INTERFACE Threads::Threads)

if(IRIS_SAMPLES)
  add_subdirectory(samples)
endif()

if(NOT IRIS_SUBPROJECT)
  configure_package_config_file(irisConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/irisConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iris)
  write_basic_package_version_file(irisConfigVersion.cmake
                                   COMPATIBILITY AnyNewerVersion)

  configure_file(iris.pc.in iris.pc @ONLY)

  install(TARGETS iris EXPORT irisTargets)
  install(EXPORT irisTargets
          FILE irisTargets.cmake
          NAMESPACE iris::
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iris)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/irisConfig.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/irisConfigVersion.cmake
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iris)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/iris.pc
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
  install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/iris
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
          USE_SOURCE_PERMISSIONS
          PATTERN "*.hpp")
  install(FILES LICENSE LICENSE.termcolor
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/iris)

  if(EXISTS "${PROJECT_SOURCE_DIR}/.gitignore")
    # Simple glob to regex conversion (.gitignore => CPACK_SOURCE_IGNORE_FILES)
    file(READ ".gitignore" DOT_GITIGNORE)
    string(REPLACE ";" "RANDOMSEQUENCE" DOT_GITIGNORE "${DOT_GITIGNORE}")
    string(REPLACE "\n" ";" DOT_GITIGNORE "${DOT_GITIGNORE}")
    string(REPLACE "RANDOMSEQUENCE" "\\;" DOT_GITIGNORE "${DOT_GITIGNORE}")
    foreach(IGNORE_LINE ${DOT_GITIGNORE})
      if(NOT IGNORE_LINE OR IGNORE_LINE MATCHES "^#")
        continue()
      endif()
      string(REPLACE "\\" "\\\\" IGNORE_LINE "${IGNORE_LINE}")
      string(REPLACE "." "\\\\." IGNORE_LINE "${IGNORE_LINE}")
      string(REPLACE "*" ".*" IGNORE_LINE "${IGNORE_LINE}")
      string(REPLACE "+" "\\\\+" IGNORE_LINE "${IGNORE_LINE}")
      list(APPEND CPACK_SOURCE_IGNORE_FILES "${IGNORE_LINE}")
    endforeach()
  endif()

  # extra ignored files
  list(APPEND CPACK_SOURCE_IGNORE_FILES
    .editorconfig
    .git
    .gitignore
    .travis.yml
    .appveyor.yml
  )
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
  set(CPACK_GENERATOR "TGZ;TXZ")
  set(CPACK_SOURCE_GENERATOR "TGZ;TXZ")
  include(CPack)
endif()

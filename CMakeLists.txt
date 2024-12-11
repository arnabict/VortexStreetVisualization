cmake_minimum_required(VERSION 3.18)

PROJECT(vispro CXX C)

# ------------------------------------
# ----- Compiler settings ------------
# ------------------------------------
# C++17
set(CMAKE_CXX_STANDARD 17)

if(UNIX) 
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  Set(CMAKE_BUILD_TYPE Release)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

IF(WIN32)
  ADD_DEFINITIONS(-DNOMINMAX)
  add_compile_options(-bigobj)
  add_compile_options(-MP)
ENDIF()

# ------------------------------------
# ----- Find dependent modules -------
# ------------------------------------

# ----- Eigen -----
include(FetchContent)
FetchContent_Declare(eigen3
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 9441d94dccccd5db8d64179516fdc5b53994a047
        )
FetchContent_Populate(eigen3)
add_library(eigen INTERFACE)
add_library (eigen3::eigen ALIAS eigen)
target_include_directories(eigen INTERFACE ${eigen3_SOURCE_DIR})

# ----- OpenMP -----
FIND_PACKAGE(OpenMP)
IF(OPENMP_FOUND)
  MESSAGE(STATUS "Using OpenMP parallelization")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
ELSE()
  MESSAGE(STATUS "Not using OpenMP parallelization")
ENDIF()

# ----- VTK -----
find_package(VTK REQUIRED)
include_directories(SYSTEM ${VTK_INCLUDE_DIRS})

# ------------------------------------
# ----- Define output paths ----------
# ------------------------------------
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(cmd)
ADD_SUBDIRECTORY(viewer)
cmake_minimum_required (VERSION 2.6)
project (RobotArm)
# The version number.
set (ROBOT_ARM_VERSION_MAJOR 1)
set (ROBOT_ARM_VERSION_MINOR 0)

FIND_PACKAGE( Boost 1.66.0 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )


# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )
 
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
include_directories("${PROJECT_SOURCE_DIR}/XYZRobot")

add_subdirectory(XYZRobot)
add_library(modbus STATIC XYZRobot/modbus.cxx)
add_library(xyz_robot STATIC XYZRobot/xyz_robot.cxx)
add_library(JobManager STATIC XYZRobot/JobManager.cpp)

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/XYZRobot")

# add the executable
add_executable(RobotArm main.cxx)
target_link_libraries(RobotArm ${Boost_LIBRARIES})
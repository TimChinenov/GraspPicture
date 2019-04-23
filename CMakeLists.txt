cmake_minimum_required(VERSION 3.1)

project(main)

set(EXECUTABLE_OUTPUT_PATH "..")
set(CMAKE_BUILD_TYPE Debug)

# force C++11, uncomment the following three lines if you're
# using OpenCV 4.0 or upper
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

# OpenCV stuff
find_package(OpenCV REQUIRED)
file(GLOB SRCS *.cpp)
add_executable(main ${SRCS})
target_link_libraries(main ${OpenCV_LIBS})

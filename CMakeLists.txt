#Enable c++14
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

#Project name
project(Rate-limiter-cpp)

#Add header files to project
FILE(GLOB_RECURSE INC_ALL "*.h")

#Create executable named TestLimiter.exe
add_executable(TestLimiter TestLimiter.cpp ${INC_ALL})

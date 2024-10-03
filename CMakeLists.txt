cmake_minimum_required(VERSION 3.10)

#project name and version
project("Networking Server Test Project" VERSION 0.1)

#C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#Configure header file to pass in CMake settings to source code
#configure_file(HEADERNAME.h.in HEADERNAME.h)

#add library/src/include subdirectories
#add_subdirectory(src)

#add executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/$<0:>")
add_executable(testServer ${PROJECT_SOURCE_DIR}/src/testServer.cpp)

#add additional source code dependent on OS
if(UNIX)
	target_compile_definitions(testServer PRIVATE "UNIX_BUILD")
	target_sources(testServer PRIVATE ${PROJECT_SOURCE_DIR}/src/linux/server_obj.cpp)
	target_include_directories(testServer PRIVATE "${PROJECT_SOURCE_DIR}/include/linux/")
endif(UNIX)

if(WIN32)
	target_compile_definitions(testServer PRIVATE "WIN32_BUILD")
	target_sources(testServer PRIVATE ${PROJECT_SOURCE_DIR}/src/win32/server_obj_win32.cpp)
	target_link_libraries(testServer PRIVATE ws2_32)
	target_include_directories(testServer PRIVATE "${PROJECT_SOURCE_DIR}/include/win32/")
endif(WIN32)


#link target libraries
#target_link_libraries(testServer PRIVATE CPPServerObject)

#add paths to the included header files
#target_include_directories(testServer PRIVATE "${PROJECT_SOURCE_DIR}/include")



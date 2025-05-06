cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)

# Enable Hot Reload for MSVC compilers if supported.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization

# Ensure SFML is built as static libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build SFML as static libraries" FORCE)

if(MSVC) 
add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
endif()

project(polymat)

# this is heuristically generated, and may not be correct
find_package(SFML COMPONENTS graphics window system audio network REQUIRED)


file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.h include/*.hpp)

# Add source to this project's executable.
add_executable ("${CMAKE_PROJECT_NAME}")

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 17)

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})

if(MSVC) # If using the VS compiler...

	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)


	#YOU CAN REMOVE THE CONSOLE WITH THIS LINE! YOU CAN EVEN DO AN IF PRODUCTION_BUILD TO REMOVE IT ONLY IN PRODUCTION BUILDS
	#set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console
	

endif()

add_custom_command(
    TARGET ${PROJECT_NAME}
    COMMENT "Copy Res directory"
    PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
    VERBATIM)

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "src" "include/engine" "include/")

target_link_libraries("${CMAKE_PROJECT_NAME}"
	sfml-graphics
	sfml-window
	sfml-audio
	sfml-system
	sfml-network
)
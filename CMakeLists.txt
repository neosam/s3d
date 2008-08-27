# Lisp3D Engine CMake file

# Project Name
project(lisp3d)

# Check and load dependencies
INCLUDE(${CMAKE_ROOT}/Modules/FindSDL.cmake)
#INCLUDE(${CMAKE_ROOT}/Modules/FindSDL_image.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
#INCLUDE(${CMAKE_ROOT}/Modules/FindGLU.cmake)
#INCLUDE(${CMAKE_ROOT}/Modules/FindOpenAL.cmake)

# Bind include options for the c++ compiler
include_directories(${CMAKE_BINARY_DIR}
		    ${SDL_INCLUDE_DIR}
#		    ${SDLIMAGE_INCLUDE_DIR}
		    ${OPENGL_INCLUDE_DIR})

# Bind libraries for the linker
link_libraries(${SDL_LIBRARY}
#	       ${SDLIMAGE_LIBRARY}
	       ${OPENGL_LIBRARIES})
#	       ${OPENAL_LIBRARY}
#	       ${GLU_LIBRARY}
#	       -lalut)

#ADD_DEFINITIONS(-DNDEBUG)
#ADD_DEFINITIONS(-ansi)
ADD_DEFINITIONS(-g)

#configure_file(src/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

# Create n3dViewer executable
add_executable(s3d
	src/s3d.c
	src/msgqueue.c)

add_executable(s3d_display
	src/display.c
	src/msgqueue.c)

install(TARGETS s3d s3d_display DESTINATION bin)
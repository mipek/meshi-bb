# Meshi-BlackBox project CMake file

cmake_minimum_required(VERSION 2.8)
project(blackbox)

# only required for windows but does no harm
add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)

include_directories(public)

file(GLOB blackbox_SRC
	"src/*.h"
	"src/*.cpp"
	"src/drivers/*.h"
	"src/drivers/*.cpp"
	"src/minmea/*.c"
	"public/*.h"
)

add_executable(blackbox ${blackbox_SRC})

if (UNIX)
	find_package(OpenCV REQUIRED)
	target_link_libraries(blackbox opencv_core opencv_video opencv_videoio)
endif (UNIX)

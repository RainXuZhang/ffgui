cmake_minimum_required(VERSION 3.20)
project(FFGui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt6 setup
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Multimedia
    MultimediaWidgets
    Quick
    QuickWidgets
    Svg
    SvgWidgets
    OpenGL
    OpenGLWidgets
)

# FFmpeg setup
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libavutil libavfilter libavdevice libswscale libswresample)

# Additional libraries
find_package(OpenGL REQUIRED)
find_package(OpenMP)

# CMake options
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${FFMPEG_INCLUDE_DIRS})

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h" "src/*.hpp")

# Main executable
add_executable(FFGui ${SOURCES})

# Enable Qt's automoc
set_target_properties(FFGui PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
    QT_AUTOMOC ON
)

# Link libraries
target_link_libraries(FFGui PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Quick
    Qt6::QuickWidgets
    Qt6::Svg
    Qt6::SvgWidgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    ${FFMPEG_LIBRARIES}
    ${OPENGL_LIBRARIES}
)

if(OpenMP_FOUND)
    target_link_libraries(FFGui PRIVATE OpenMP::OpenMP_CXX)
endif()

# Compile definitions
target_compile_definitions(FFGui PRIVATE
    QT_NO_DEBUG_OUTPUT
    QT_NO_WARNING_OUTPUT
)

# Include directories
target_include_directories(FFGui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${FFMPEG_INCLUDE_DIRS}
)

# Install
install(TARGETS FFGui DESTINATION bin)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
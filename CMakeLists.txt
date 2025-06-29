cmake_minimum_required(VERSION 3.10)
project(NoteOnTopApp VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GTKmm
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0)

# Include GTKmm headers
include_directories(
    ${GTKMM_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/markdowns
    ${WEBKIT2GTK_INCLUDE_DIRS}
)

link_directories(
    ${GTKMM_LIBRARY_DIRS}
    ${JSONCPP_LIBRARY_DIRS}
    ${CURL_LIBRARY_DIRS}
    ${WEBKIT2GTK_LIBRARY_DIRS}
)

add_definitions(
    ${GTKMM_CFLAGS_OTHER} 
    ${WEBKIT2GTK_CFLAGS_OTHER}
)

# Your source files
set(SOURCES
    main.cpp
    NoteEditor.cpp
    MainWindow.cpp
    markdown/markdown.cpp
    markdown/markdown-tokens.cpp
)

# Header files (optional, but good for IDEs)
set(HEADERS
    markdown/markdown.h
    markdown/markdown-tokens.h
    NoteEditor.h
    MainWindow.h
)

# Build executable
add_executable(noteontop ${SOURCES})

# Link GTKmm libraries
target_link_libraries(noteontop
    ${GTKMM_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    ${CURL_LIBRARIES}
    ${BOOST_INCLUDE_DIRS}
    ${WEBKIT2GTK_LIBRARIES}
)
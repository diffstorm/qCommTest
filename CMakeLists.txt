cmake_minimum_required(VERSION 3.5)

project(qCommTest LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt
find_package(Qt6 COMPONENTS Core Gui Network SerialPort Widgets REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Network SerialPort Widgets)

if(Qt6_FOUND)
    set(QT_VERSION 6)
    message(STATUS "Using Qt 6")
    set(QT_LIBS Qt6::Core Qt6::Gui Qt6::Network Qt6::SerialPort Qt6::Widgets)
else()
    set(QT_VERSION 5)
    message(STATUS "Using Qt 5")
    set(QT_LIBS Qt5::Core Qt5::Gui Qt5::Network Qt5::SerialPort Qt5::Widgets)
endif()

# Add source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    serial_port.cpp
    serial_port.h
    tcp_server.cpp
    tcp_server.h
    qdarkstyle/style.qrc
)

# Add executable
add_executable(qCommTest ${SOURCES})

# Link against Qt libraries
target_link_libraries(qCommTest PRIVATE ${QT_LIBS})

# Add icon for Windows
if(WIN32)
    set_target_properties(qCommTest PROPERTIES WIN32_EXECUTABLE TRUE)
    set(RC_ICONS qdarkstyle/rc/icon.ico)
    set(RC_FILE ${CMAKE_CURRENT_BINARY_DIR}/qCommTest.rc)
    file(WRITE ${RC_FILE} "IDI_ICON1 ICON DISCARDABLE \"${CMAKE_CURRENT_SOURCE_DIR}/${RC_ICONS}\"")
    target_sources(qCommTest PRIVATE ${RC_FILE})
endif()

# Install rules
install(TARGETS qCommTest
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

cmake_minimum_required(VERSION 3.16)

project(QCrossPlatformSerial VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Find Qt components
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core SerialPort Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core SerialPort Gui)

# Library sources
set(LIBRARY_SOURCES
    src/QCrossPlatformSerialPort.cpp
    src/QCrossPlatformSerialPortInfo.cpp
)

# Library headers
set(LIBRARY_HEADERS
    src/QCrossPlatformSerialPort.hpp
    src/QCrossPlatformSerialPortInfo.hpp
)

# Create static library
add_library(QCrossPlatformSerial STATIC
    ${LIBRARY_SOURCES}
    ${LIBRARY_HEADERS}
)

# Include directories
target_include_directories(QCrossPlatformSerial
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:src>
)

# Link Qt libraries
target_link_libraries(QCrossPlatformSerial PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Gui
)

# Android-specific configuration
if(ANDROID)
    # Add Java sources
    set(ANDROID_JAVA_SOURCES
        android/src/org/qtserial/UsbSerialManager.java
        android/src/org/qtserial/JniUsbSerial.java
        android/src/org/qtserial/SerialInputOutputManager.java
    )
    
    # Add Java sources to target
    target_sources(QCrossPlatformSerial PRIVATE ${ANDROID_JAVA_SOURCES})
    
    # Set Android-specific properties
    set_target_properties(QCrossPlatformSerial PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )
endif()

# Install targets
install(TARGETS QCrossPlatformSerial
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${LIBRARY_HEADERS}
    DESTINATION src
)

if(ANDROID)
    install(DIRECTORY android/
        DESTINATION android/QCrossPlatformSerial
        FILES_MATCHING PATTERN "*.java"
    )
endif()

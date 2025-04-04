cmake_minimum_required(VERSION 3.16)

project(QtAldegea VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt6 COMPONENTS Widgets Bluetooth REQUIRED)

# Stel Qt policies in
qt_policy(SET QTP0002 NEW)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        drivers/bluetoothscanner.cpp
        drivers/bluetoothscanner.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(QtAldegea  # <-- TARGET DEFINITIE
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    # Eventuele Android properties hier
    set_property(TARGET QtAldegea APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
else()
    # ... Qt 5 logica ...
    add_executable(QtAldegea ${PROJECT_SOURCES}) # <-- TARGET DEFINITIE
endif()

### Internationalization

file(GLOB TS_FILES i18n/*.ts)

# Source files to be translated (separate because some files may be masked)
file(GLOB_RECURSE TRANSLATION_SOURCE_FILES
    *.h
    *.ui
    *.cpp
)

if(${QT_VERSION} VERSION_GREATER_EQUAL "6.7.0")
	message(STATUS "Using new qt_add_translations")
	qt_add_translations(
                QtAldegea # <-- Verwijs naar het nu bestaande target
                TS_FILES ${TS_FILES}
		SOURCES ${TRANSLATION_SOURCE_FILES}
		LUPDATE_OPTIONS
		  #-no-obsolete
		  -locations none
		IMMEDIATE_CALL
	)
else()
	message(STATUS "Using old qt_add_translations")
	qt_add_translations(
                QtAldegea # <-- Verwijs naar het nu bestaande target
                TS_FILES ${TS_FILES}
		SOURCES ${TRANSLATION_SOURCE_FILES}
		LUPDATE_OPTIONS
		  #-no-obsolete
		  -locations none
    )
endif()

# For convenience, QtCreator does not show the umbrella target
add_custom_target(global_lupdate DEPENDS update_translations)

target_link_libraries(QtAldegea PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(QtAldegea PRIVATE Qt6::Widgets Qt6::Bluetooth) # Bluetooth toevoegen

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.QtAldegea)
endif()
set_target_properties(QtAldegea PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS QtAldegea
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtAldegea)
endif()

# Qt Project File for QtAldegea
# Generated based on provided CMakeLists.txt and main.cpp

# Specify required Qt modules
QT += widgets

# Set the target executable name
TARGET = QtAldegea

# Specify the project type (application)
TEMPLATE = app

# Enable C++17 features
CONFIG += c++17

# Define source code files
SOURCES += \
    main.cpp \
    mainwindow.cpp

# Define header files (important for MOC processing if they contain Q_OBJECT)
HEADERS += \
    mainwindow.h

# Define user interface files (will be processed by UIC)
FORMS += \
    mainwindow.ui

# Define translation source files
# Assumes the .ts file is located in an 'i18n' subdirectory
# lupdate will use this to update the .ts file
# lrelease will use this to create the .qm file
TRANSLATIONS += \
    i18n/QtAldegea_nl_NL.ts

# Define the Qt Resource file
# This file lists resources to be embedded into the executable,
# including the compiled .qm translation files.
RESOURCES += \
    $$PWD/translations.qrc

# --- Optional Settings (mirrored from CMakeLists.txt comments/defaults) ---

# Define version information (similar to PROJECT_VERSION)
# VERSION = 0.1.0

# macOS specific bundle information (qmake often sets reasonable defaults)
# QMAKE_MACOSX_BUNDLE_GUI_IDENTIFIER = com.example.QtAldegea
# QMAKE_bundle_DATA.CFBundleVersion = $$VERSION
# QMAKE_bundle_DATA.CFBundleShortVersionString = $$VERSION # Adjust as needed

# For Android deployment using .pro files, you often configure
# the AndroidManifest.xml and other settings within Qt Creator's
# Build & Run -> Run settings for the Android Kit, or use variables like:
# android {
#     DISTFILES += android/AndroidManifest.xml
#     # ANDROID_PACKAGE_SOURCE_DIR points to a directory containing AndroidManifest.xml,
#     # res/ directory, assets/, etc.
#     ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
# }

# If deploying QML files (not used here, but common):
# QML_IMPORT_PATH =
# Add QML files to RESOURCES or use specific deployment steps for mobile.
# PowerEditor/src/CMakeLists.txt — CMake Build System para Notepad++ Linux Nativo (Qt6)
# Copyright (C) Notepad++ contributors. GPL v3+

cmake_minimum_required(VERSION 3.16)
project(NotepadPlusPlus LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ── Configurar Qt6 ────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport DBus Network)


# Buscar QScintilla (Qt6)
find_library(QSCINTILLA_LIB NAMES qscintilla2_qt6 qscintilla2-qt6)
find_path(QSCINTILLA_INCLUDE_DIR NAMES Qsci/qsciscintilla.h PATH_SUFFIXES qt6)

if(NOT QSCINTILLA_LIB OR NOT QSCINTILLA_INCLUDE_DIR)
    message(FATAL_ERROR "Could NOT find QScintilla library or headers for Qt6.")
endif()

message(STATUS "Found QScintilla library: ${QSCINTILLA_LIB}")
message(STATUS "Found QScintilla include dir: ${QSCINTILLA_INCLUDE_DIR}")

add_compile_definitions(NPP_PLATFORM_LINUX)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform
    ${CMAKE_CURRENT_SOURCE_DIR}/WinControls/Qt
    ${QSCINTILLA_INCLUDE_DIR}
)

# ── Archivos fuente Linux ─────────────────────────────────────────────────────
set(NPP_LINUX_SOURCES
    linuxmain.cpp
    NotepadPlusApp_linux.cpp
    Platform/PlatformMsg.cpp
)

set(NPP_LINUX_HEADERS
    Platform/PlatformTypes.h
    Platform/PlatformString.h
    Platform/PlatformMsg.h
    Platform/PlatformWindow.h
    Platform/PlatformPlugin.h
    Platform/PlatformFileMonitor.h
    Platform/PlatformFileDialog.h
    Platform/PlatformTheme.h
    Platform/PlatformCrashHandler.h
    Platform/PlatformDesktopIntegration.h
    WinControls/Qt/NotepadPlusWindowQt.h
    WinControls/Qt/NppStatusBar.h
    WinControls/Qt/NppTabBar.h
    WinControls/Qt/NppToolBar.h
    WinControls/Qt/NppSplitter.h
    WinControls/Qt/NppTreeView.h
    WinControls/Qt/NppDialog.h
    WinControls/Qt/NppDockWidget.h
    WinControls/Qt/NppFindReplaceDlg.h
    WinControls/Qt/NppFileBrowser.h
    WinControls/Qt/NppPreferenceDlg.h
    WinControls/Qt/NppShortcutMapper.h
    WinControls/Qt/NppFunctionList.h
    WinControls/Qt/NppProjectPanel.h
    WinControls/Qt/NppPluginsAdmin.h
    WinControls/Qt/NppColourPicker.h
    WinControls/Qt/NppDocumentMap.h
    WinControls/Qt/NppClipboardHistory.h
    WinControls/Qt/NppVerticalFileSwitcher.h
    WinControls/Qt/NppAboutDlg.h
    WinControls/Qt/NppGoToLineDlg.h
    WinControls/Qt/NppRunDlg.h
    WinControls/Qt/NppMD5Dlg.h
    WinControls/Qt/NppAnsiCharPanel.h
    WinControls/Qt/NppWindowsDlg.h
    WinControls/Qt/NppColumnEditor.h
    WinControls/Qt/NppSummaryDlg.h
    WinControls/Qt/NppUserDefineDlg.h
    WinControls/Qt/NppLocalization.h
    WinControls/Qt/NppToolTip.h
    WinControls/Qt/NppTrayIcon.h
)




# Qt MOC automático
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Executable principal
add_executable(notepadplusplus
    ${NPP_LINUX_SOURCES}
    ${NPP_LINUX_HEADERS}
)

target_link_libraries(notepadplusplus PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::PrintSupport
    Qt6::DBus
    Qt6::Network
    ${QSCINTILLA_LIB}
    dl
)


message(STATUS "✅ Configuración CMake para Notepad++ Linux Nativo (Qt6) lista.")

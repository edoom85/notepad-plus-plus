# PowerEditor/src/CMakeLists.txt — Sistema de compilación CMake dual para Windows (MinGW/MSVC) y Linux Nativo (Qt6)
# Copyright (C) Notepad++ contributors. GPL v3+

cmake_minimum_required(VERSION 3.16)
project(NotepadPlusPlus LANGUAGES CXX C)

if(WIN32)
    # ── Configuración CMake para Windows (MinGW / MSVC) ─────────────────────
    SET(projIncludDir
        ./
        ./json/
        ./DarkMode/
        ./MISC/
        ./MISC/Common/
        ./MISC/Exception/
        ./MISC/PluginsManager/
        ./MISC/Process/
        ./MISC/RegExt/
        ./MISC/crc16/
        ./MISC/hmac/
        ./MISC/md5/
        ./MISC/sha1/
        ./MISC/sha2/
        ./MISC/sha512/
        ./Platform/
        ./ScintillaComponent/
        ./uchardet/
        ./WinControls/
        ./WinControls/AboutDlg/
        ./WinControls/AnsiCharPanel/
        ./WinControls/ClipboardHistory/
        ./WinControls/ColourPicker/
        ./WinControls/ContextMenu/
        ./WinControls/DockingWnd/
        ./WinControls/DocumentMap/
        ./WinControls/DoubleBuffer/
        ./WinControls/FileBrowser/
        ./WinControls/FindCharsInRange/
        ./WinControls/FunctionList/
        ./WinControls/Grid/
        ./WinControls/ImageListSet/
        ./WinControls/OpenSaveFileDialog/
        ./WinControls/PluginsAdmin/
        ./WinControls/Preference/
        ./WinControls/ProjectPanel/
        ./WinControls/ReadDirectoryChanges/
        ./WinControls/shortcut/
        ./WinControls/SplitterContainer/
        ./WinControls/StaticDialog/
        ./WinControls/StaticDialog/RunDlg/
        ./WinControls/StatusBar/
        ./WinControls/TabBar/
        ./WinControls/TaskList/
        ./WinControls/ToolBar/
        ./WinControls/ToolTip/
        ./WinControls/TrayIcon/
        ./WinControls/TreeView/
        ./WinControls/VerticalFileSwitcher/
        ./WinControls/WindowsDlg/
        ../../scintilla/include/
        ../../lexilla/include/
        ./pugixml/
    )

    SET(src_files
        ./WinControls/AnsiCharPanel/ansiCharPanel.cpp
        ./WinControls/AboutDlg/AboutDlg.cpp
        ./WinControls/Grid/BabyGrid.cpp
        ./WinControls/Grid/BabyGridWrapper.cpp
        ./ScintillaComponent/Buffer.cpp
        ./uchardet/CharDistribution.cpp
        ./WinControls/ClipboardHistory/clipboardHistoryPanel.cpp
        ./ScintillaComponent/columnEditor.cpp
        ./MISC/Common/Common.cpp
        ./MISC/Process/Processus.cpp
        ./WinControls/ContextMenu/ContextMenu.cpp

        ./WinControls/OpenSaveFileDialog/CustomFileDialog.cpp
        ./WinControls/DockingWnd/DockingCont.cpp

        ./WinControls/DockingWnd/DockingManager.cpp
        ./WinControls/DockingWnd/DockingSplitter.cpp
        ./ScintillaComponent/DocTabView.cpp
        ./WinControls/DocumentMap/documentMap.cpp
        ./WinControls/DocumentMap/documentSnapshot.cpp
        ./WinControls/DoubleBuffer/DoubleBuffer.cpp
        ./EncodingMapper.cpp
        ./WinControls/FindCharsInRange/FindCharsInRange.cpp
        ./ScintillaComponent/FindReplaceDlg.cpp
        ./ScintillaComponent/FunctionCallTip.cpp
        ./WinControls/FunctionList/functionListPanel.cpp
        ./WinControls/FunctionList/functionParser.cpp
        ./ScintillaComponent/GoToLineDlg.cpp
        ./WinControls/DockingWnd/Gripper.cpp
        ./WinControls/ImageListSet/ImageListSet.cpp
        ./uchardet/JpCntx.cpp
        ./lastRecentFileList.cpp
        ./WinControls/AnsiCharPanel/ListView.cpp
        ./localization.cpp
        ./MISC/Exception/MiniDumper.cpp
        ./Notepad_plus.cpp
        ./Notepad_plus_Window.cpp
        ./uchardet/nsBig5Prober.cpp
        ./uchardet/nsCharSetProber.cpp
        ./uchardet/nsEscCharsetProber.cpp
        ./uchardet/nsEUCJPProber.cpp
        ./uchardet/nsEUCKRProber.cpp
        ./uchardet/nsEUCTWProber.cpp
        ./uchardet/nsGB2312Prober.cpp
        ./uchardet/nsHebrewProber.cpp
        ./uchardet/nsLatin1Prober.cpp
        ./uchardet/nsMBCSGroupProber.cpp
        ./uchardet/nsSBCharSetProber.cpp
        ./uchardet/nsSBCSGroupProber.cpp
        ./uchardet/nsSJISProber.cpp
        ./uchardet/nsUniversalDetector.cpp
        ./uchardet/nsUTF8Prober.cpp
        ./Parameters.cpp
        ./MISC/PluginsManager/PluginsManager.cpp
        ./WinControls/Preference/preferenceDlg.cpp
        ./ScintillaComponent/Printer.cpp
        ./WinControls/ProjectPanel/ProjectPanel.cpp
        ./MISC/RegExt/regExtDlg.cpp
        ./WinControls/StaticDialog/RunDlg/RunDlg.cpp
        ./WinControls/shortcut/RunMacroDlg.cpp
        ./ScintillaComponent/ScintillaEditView.cpp
        ./WinControls/shortcut/shortcut.cpp
        ./WinControls/Grid/ShortcutMapper.cpp
        ./WinControls/TreeView/TreeView.cpp
        ./WinControls/WindowsDlg/SizeableDlg.cpp
        ./ScintillaComponent/SmartHighlighter.cpp
        ./WinControls/SplitterContainer/Splitter.cpp
        ./WinControls/SplitterContainer/SplitterContainer.cpp
        ./WinControls/StaticDialog/StaticDialog.cpp
        ./WinControls/StatusBar/StatusBar.cpp
        ./WinControls/TabBar/TabBar.cpp
        ./WinControls/TaskList/TaskList.cpp
        ./WinControls/TaskList/TaskListDlg.cpp
        ./pugixml/pugixml.cpp
        ./WinControls/ToolBar/ToolBar.cpp
        ./WinControls/ToolTip/ToolTip.cpp
        ./WinControls/TrayIcon/trayIconControler.cpp
        ./uchardet/uchardet.cpp
        ./WinControls/AboutDlg/URLCtrl.cpp
        ./ScintillaComponent/UserDefineDialog.cpp
        ./Utf8_16.cpp
        ./WinControls/VerticalFileSwitcher/VerticalFileSwitcher.cpp
        ./WinControls/VerticalFileSwitcher/VerticalFileSwitcherListView.cpp
        ./MISC/Exception/Win32Exception.cpp
        ./WinControls/WindowsDlg/WindowsDlg.cpp
        ./WinControls/WindowsDlg/WinMgr.cpp
        ./WinControls/ColourPicker/WordStyleDlg.cpp
        ./ScintillaComponent/xmlMatchedTagsHighlighter.cpp
        ./winmain.cpp
        ./WinControls/FileBrowser/fileBrowser.cpp
        ./WinControls/PluginsAdmin/pluginsAdmin.cpp
        ./WinControls/ColourPicker/ColourPicker.cpp
        ./WinControls/ColourPicker/ColourPopup.cpp
        ./dpiManagerV2.cpp
        ./NppDarkMode.cpp
        ./DarkMode/DarkMode.cpp
        ./WinControls/AnsiCharPanel/AsciiListView.cpp
        ./WinControls/ProjectPanel/ProjectPanel.cpp
        ./NppCommands.cpp
        ./NppIO.cpp
        ./NppNotification.cpp
        ./NppBigSwitch.cpp
        ./lesDlgs.cpp
        ./MISC/Common/FileInterface.cpp
        ./MISC/Common/SortLocale.cpp
        ./MISC/Common/verifySignedfile.cpp
        ./WinControls/TabBar/ControlsTab.cpp
        ./WinControls/WindowsDlg/WinRect.cpp
        ./ScintillaComponent/AutoCompletion.cpp
        ./ScintillaComponent/ScintillaCtrls.cpp
        ./WinControls/ReadDirectoryChanges/ReadDirectoryChanges.cpp
        ./WinControls/ReadDirectoryChanges/ReadDirectoryChangesPrivate.cpp
        ./WinControls/ReadDirectoryChanges/ReadFileChanges.cpp
        ./MISC/sha1/calc_sha1.cpp
        ./MISC/sha1/sha1.cpp
        ./MISC/sha2/sha-256.cpp
        ./MISC/sha512/sha512.cpp
        ./MISC/md5/md5Dlgs.cpp

        ./uchardet/LangBulgarianModel.cpp
        ./uchardet/LangCyrillicModel.cpp
        ./uchardet/LangGreekModel.cpp
        ./uchardet/LangHebrewModel.cpp
        ./uchardet/LangHungarianModel.cpp
        ./uchardet/LangThaiModel.cpp
        ./uchardet/nsEscSM.cpp
        ./uchardet/nsMBCSSM.cpp
    )




    SET(rcFiles
        ./MISC/md5/md5Dlgs.rc
        ./WinControls/AnsiCharPanel/ansiCharPanel.rc
        ./WinControls/ClipboardHistory/clipboardHistoryPanel.rc
        ./WinControls/ColourPicker/ColourPopup.rc
        ./ScintillaComponent/columnEditor.rc
        ./WinControls/DockingWnd/DockingGUIWidget.rc
        ./WinControls/DocumentMap/documentMap.rc
        ./WinControls/DocumentMap/documentSnapshot.rc
        ./WinControls/FileBrowser/fileBrowser.rc
        ./WinControls/FindCharsInRange/findCharsInRange.rc
        ./ScintillaComponent/FindReplaceDlg.rc
        ./WinControls/FunctionList/functionListPanel.rc
        ./Notepad_plus.rc
        ./WinControls/PluginsAdmin/pluginsAdmin.rc
        ./WinControls/Preference/preference.rc
        ./WinControls/ProjectPanel/ProjectPanel.rc
        ./MISC/RegExt/regExtDlg.rc
        ./WinControls/StaticDialog/RunDlg/RunDlg.rc
        ./WinControls/shortcut/RunMacroDlg.rc
        ./WinControls/shortcut/shortcut.rc
        ./WinControls/Grid/ShortcutMapper.rc
        ./WinControls/TaskList/TaskListDlg.rc
        ./ScintillaComponent/UserDefineDialog.rc
        ./WinControls/VerticalFileSwitcher/VerticalFileSwitcher.rc
        ./WinControls/WindowsDlg/WindowsDlg.rc
        ./WinControls/ColourPicker/WordStyleDlg.rc
    )

    SET(option WIN32)
    SET(win32_LIBRARIES comctl32 shlwapi dbghelp version crypt32 wintrust sensapi wininet imm32 msimg32 uxtheme dwmapi pathcch)
    set(CMAKE_CXX_STANDARD 20)

    if (MSVC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        SET(CMAKE_CXX_FLAGS "/EHa /MP /W4 /MT")
        SET(defs -DUNICODE -D_UNICODE -D_WIN32_WINNT=_WIN32_WINNT_WIN7 -DNTDDI_VERSION=NTDDI_WIN7 -D_USE_64BIT_TIME_T -DNOMINMAX -DOEMRESOURCE -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    else()
        SET(CMAKE_CXX_FLAGS "-include../gcc/gcc-fixes.h -std=c++20 -fpermissive -municode")
        SET(defs -DUNICODE -D_UNICODE -D_WIN32_WINNT=_WIN32_WINNT_WIN7 -DNTDDI_VERSION=NTDDI_WIN7 -D_USE_64BIT_TIME_T -DNOMINMAX -DOEMRESOURCE)
    endif()

    ADD_DEFINITIONS(${defs})
    INCLUDE_DIRECTORIES(${projIncludDir})

    find_library(SCINTILLA_STATIC_LIBRARY
        NAMES libscintilla
        HINTS "../../scintilla/bin"
        REQUIRED
    )

    find_library(LEXILLA_STATIC_LIBRARY
        NAMES liblexilla
        HINTS "../../lexilla/bin"
        REQUIRED
    )

    execute_process(
        COMMAND cmd /C "cd ${CMAKE_SOURCE_DIR} && NppLibsVersionH-generator.bat"
        RESULT_VARIABLE pre-build-result
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    ADD_EXECUTABLE(notepad++ ${option} ${src_files} ${rcFiles})
    if (MSVC)
        set_property(TARGET notepad++ PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
    TARGET_LINK_LIBRARIES(notepad++ ${SCINTILLA_STATIC_LIBRARY} ${LEXILLA_STATIC_LIBRARY} ${win32_LIBRARIES})


else()
    # ── Configuración CMake para Linux Nativo (Qt6) ───────────────────────────
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport DBus Network)

    find_library(QSCINTILLA_LIB NAMES qscintilla2_qt6 qscintilla2-qt6)
    find_path(QSCINTILLA_INCLUDE_DIR NAMES Qsci/qsciscintilla.h PATH_SUFFIXES qt6)

    if(NOT QSCINTILLA_LIB OR NOT QSCINTILLA_INCLUDE_DIR)
        message(FATAL_ERROR "Could NOT find QScintilla library or headers for Qt6.")
    endif()

    message(STATUS "Found QScintilla library: ${QSCINTILLA_LIB}")
    message(STATUS "Found QScintilla include dir: ${QSCINTILLA_INCLUDE_DIR}")

    add_compile_definitions(NPP_PLATFORM_LINUX)

    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Platform
        ${CMAKE_CURRENT_SOURCE_DIR}/WinControls/Qt
        ${QSCINTILLA_INCLUDE_DIR}
    )

    set(NPP_LINUX_SOURCES
        linuxmain.cpp
        NotepadPlusApp_linux.cpp
        Platform/PlatformMsg.cpp
    )

    set(NPP_LINUX_HEADERS
        Platform/PlatformTypes.h
        Platform/PlatformDesktopIntegration.h
        Platform/PlatformDpiManager.h
        Platform/PlatformIconProvider.h
        Platform/PlatformLexerManager.h
        Platform/PlatformString.h
        Platform/PlatformMsg.h
        Platform/PlatformWindow.h
        Platform/PlatformPlugin.h
        Platform/PlatformPluginManager.h
        Platform/PlatformFileMonitor.h
        Platform/PlatformFileDialog.h
        Platform/PlatformTheme.h
        Platform/PlatformCrashHandler.h
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
        WinControls/Qt/NppStyleConfigDlg.h
        WinControls/Qt/NppRunMacroDlg.h
        WinControls/Qt/NppFindCharsInRange.h
        WinControls/Qt/NppLocalization.h


        WinControls/Qt/NppToolTip.h
        WinControls/Qt/NppTrayIcon.h
    )

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    add_executable(notepadplusplus
        ${NPP_LINUX_SOURCES}
        ${NPP_LINUX_HEADERS}
        resources.qrc
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
endif()

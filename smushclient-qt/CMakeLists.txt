cmake_minimum_required(VERSION 3.24)

cmake_policy(SET CMP0091 NEW)
set(CMAKE_OSX_DEPLOYMENT_TARGET 26.2)
set(VCPKG_HOST_TRIPLET x64-windows-static)
set(VCPKG_TARGET_TRIPLET x64-windows-static)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

project(SmushClient VERSION 0.1.0 LANGUAGES CXX)
set(APP_NAME ${PROJECT_NAME})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network PrintSupport Widgets)

if(APPLE)
  enable_language(OBJCXX)
endif()

qt_standard_project_setup(
  REQUIRES 6.8.0
  SUPPORTS_UP_TO 7
  I18N_TRANSLATED_LANGUAGES en_GB
)

include(FetchContent)
FetchContent_Declare(
  CxxQt
  GIT_REPOSITORY https://github.com/kdab/cxx-qt-cmake.git
  GIT_TAG main
  SYSTEM
)

FetchContent_Declare(
  lualibs
  GIT_REPOSITORY https://github.com/jnbooth/lua-batteries-included.git
  GIT_TAG 912afb1004044585b9495106e45dd3427871309a
)

FetchContent_MakeAvailable(
  CxxQt
  lualibs
)

set(CRATE smushclient_qt)

if (MSVC)
  set(CRATE_FEATURES link_qt_object_files)
endif ()

cxx_qt_import_crate(
  MANIFEST_PATH rust/Cargo.toml
  CRATES ${CRATE}
  FEATURES ${CRATE_FEATURES}
  QT_MODULES Qt::Core Qt::Gui Qt::Network Qt::PrintSupport Qt::Widgets
)

set_source_files_properties(resources/appicon/appicon.icns PROPERTIES
  MACOSX_PACKAGE_LOCATION "Resources"
)

qt_add_executable(${APP_NAME}
    cpp/main.cpp
    cpp/bytes.h
    cpp/casting.h
    cpp/client.h cpp/client.cpp
    cpp/commandhistory.h cpp/commandhistory.cpp
    cpp/enumbuttongroup.h
    cpp/environment.h cpp/environment.cpp
    cpp/fieldconnector.h
    cpp/hotkeys.h cpp/hotkeys.cpp
    cpp/layout.h cpp/layout.cpp
    cpp/localization.h cpp/localization.cpp
    cpp/lookup.h
    cpp/settings.h cpp/settings.cpp
    cpp/spans.h cpp/spans.cpp
    cpp/timer_map.h

    cpp/bridge/document.h cpp/bridge/document.cpp
    cpp/bridge/timekeeper.h cpp/bridge/timekeeper.cpp
    cpp/bridge/variableview.h

    cpp/model/alias.h cpp/model/alias.cpp
    cpp/model/plugin.h cpp/model/plugin.cpp
    cpp/model/sender.h cpp/model/sender.cpp
    cpp/model/timer.h cpp/model/timer.cpp
    cpp/model/trigger.h cpp/model/trigger.cpp

    cpp/native/native.h cpp/native/native.cpp
    cpp/native/result.h
    cpp/native/window.h

    cpp/scripting/databaseconnection.h cpp/scripting/databaseconnection.cpp
    cpp/scripting/plugin.h cpp/scripting/plugin.cpp
    cpp/scripting/qlua.h cpp/scripting/qlua.cpp
    cpp/scripting/scriptapi_api.cpp
    cpp/scripting/scriptapi_getinfo.cpp
    cpp/scripting/scriptapi_senders.cpp
    cpp/scripting/scriptapi_window.cpp
    cpp/scripting/scriptapi.h cpp/scripting/scriptapi.cpp
    cpp/scripting/scriptthread.h cpp/scripting/scriptthread.cpp

    cpp/scripting/miniwindow/channel.h
    cpp/scripting/miniwindow/hotspot.h cpp/scripting/miniwindow/hotspot.cpp
    cpp/scripting/miniwindow/imagebits.h cpp/scripting/miniwindow/imagebits.cpp
    cpp/scripting/miniwindow/imagefilters.h cpp/scripting/miniwindow/imagefilters.cpp
    cpp/scripting/miniwindow/miniwindow.h cpp/scripting/miniwindow/miniwindow.cpp

    cpp/scripting/callback/filter.h cpp/scripting/callback/filter.cpp
    cpp/scripting/callback/key.h
    cpp/scripting/callback/plugincallback.h cpp/scripting/callback/plugincallback.cpp
    cpp/scripting/callback/trigger.h cpp/scripting/callback/trigger.cpp

    cpp/scripting/lua/api.h cpp/scripting/lua/api.cpp
    cpp/scripting/lua/errors.h cpp/scripting/lua/errors.cpp
    cpp/scripting/lua/globals.h cpp/scripting/lua/globals.cpp
    cpp/scripting/lua/init.h cpp/scripting/lua/init.cpp
    cpp/scripting/lua/utils.h cpp/scripting/lua/utils.cpp

    cpp/ui/finddialog.h cpp/ui/finddialog.cpp cpp/ui/finddialog.ui
    cpp/ui/mainwindow.h cpp/ui/mainwindow.cpp cpp/ui/mainwindow.ui
    cpp/ui/notepad.h cpp/ui/notepad.cpp cpp/ui/notepad.ui
    cpp/ui/serverstatus.h cpp/ui/serverstatus.cpp cpp/ui/serverstatus.ui
    cpp/ui/worldtab.h cpp/ui/worldtab.cpp cpp/ui/worldtab.ui

    cpp/ui/components/colorpickerbutton.h cpp/ui/components/colorpickerbutton.cpp
    cpp/ui/components/iconlabel.h cpp/ui/components/iconlabel.cpp
    cpp/ui/components/mudbrowser.h cpp/ui/components/mudbrowser.cpp
    cpp/ui/components/mudinput.h cpp/ui/components/mudinput.cpp
    cpp/ui/components/mudscrollbar.h cpp/ui/components/mudscrollbar.cpp

    cpp/ui/mudstatusbar/mudstatusbar.h cpp/ui/mudstatusbar/mudstatusbar.cpp cpp/ui/mudstatusbar/mudstatusbar.ui
    cpp/ui/mudstatusbar/statusbarstat.h cpp/ui/mudstatusbar/statusbarstat.cpp cpp/ui/mudstatusbar/statusbarstat.ui

    cpp/ui/scripting/abstractscriptdialog.h
    cpp/ui/scripting/choose.h cpp/ui/scripting/choose.cpp cpp/ui/scripting/choose.ui
    cpp/ui/scripting/listbox.h cpp/ui/scripting/listbox.cpp cpp/ui/scripting/listbox.ui

    cpp/ui/settings/appearance.h cpp/ui/settings/appearance.cpp cpp/ui/settings/appearance.ui
    cpp/ui/settings/closing.h cpp/ui/settings/closing.cpp cpp/ui/settings/closing.ui
    cpp/ui/settings/connection.h cpp/ui/settings/connection.cpp cpp/ui/settings/connection.ui
    cpp/ui/settings/history.h cpp/ui/settings/history.cpp cpp/ui/settings/history.ui
    cpp/ui/settings/notifier.h
    cpp/ui/settings/settings.h cpp/ui/settings/settings.cpp cpp/ui/settings/settings.ui
    cpp/ui/settings/sound.h cpp/ui/settings/sound.cpp cpp/ui/settings/sound.ui
    cpp/ui/settings/startup.h cpp/ui/settings/startup.cpp cpp/ui/settings/startup.ui

    cpp/ui/worlddetails/abstractprefstree.h cpp/ui/worlddetails/abstractprefstree.cpp
    cpp/ui/worlddetails/aliasedit.h cpp/ui/worlddetails/aliasedit.cpp cpp/ui/worlddetails/aliasedit.ui
    cpp/ui/worlddetails/aliases.h cpp/ui/worlddetails/aliases.cpp cpp/ui/worlddetails/aliases.ui
    cpp/ui/worlddetails/connecting.h cpp/ui/worlddetails/connecting.cpp cpp/ui/worlddetails/connecting.ui
    cpp/ui/worlddetails/login.h cpp/ui/worlddetails/login.cpp cpp/ui/worlddetails/login.ui
    cpp/ui/worlddetails/logging.h cpp/ui/worlddetails/logging.cpp cpp/ui/worlddetails/logging.ui
    cpp/ui/worlddetails/mud.h cpp/ui/worlddetails/mud.cpp cpp/ui/worlddetails/mud.ui
    cpp/ui/worlddetails/numpad.h cpp/ui/worlddetails/numpad.cpp cpp/ui/worlddetails/numpad.ui
    cpp/ui/worlddetails/output.h cpp/ui/worlddetails/output.cpp cpp/ui/worlddetails/output.ui
    cpp/ui/worlddetails/pluginpopup.h cpp/ui/worlddetails/pluginpopup.cpp cpp/ui/worlddetails/pluginpopup.ui
    cpp/ui/worlddetails/plugins.h cpp/ui/worlddetails/plugins.cpp cpp/ui/worlddetails/plugins.ui
    cpp/ui/worlddetails/regexdialog.h cpp/ui/worlddetails/regexdialog.cpp cpp/ui/worlddetails/regexdialog.ui
    cpp/ui/worlddetails/scripting.h cpp/ui/worlddetails/scripting.cpp cpp/ui/worlddetails/scripting.ui
    cpp/ui/worlddetails/specialhelp.h cpp/ui/worlddetails/specialhelp.cpp cpp/ui/worlddetails/specialhelp.ui
    cpp/ui/worlddetails/timeredit.h cpp/ui/worlddetails/timeredit.cpp cpp/ui/worlddetails/timeredit.ui
    cpp/ui/worlddetails/timers.h cpp/ui/worlddetails/timers.cpp cpp/ui/worlddetails/timers.ui
    cpp/ui/worlddetails/triggeredit.h cpp/ui/worlddetails/triggeredit.cpp cpp/ui/worlddetails/triggeredit.ui
    cpp/ui/worlddetails/triggers.h cpp/ui/worlddetails/triggers.cpp cpp/ui/worlddetails/triggers.ui
    cpp/ui/worlddetails/worlddetails.h cpp/ui/worlddetails/worlddetails.cpp cpp/ui/worlddetails/worlddetails.ui

    resources/resources.qrc
    resources/appicon/appicon.icns resources/appicon/appicon.rc
    cpp/ui/worlddetails/regexdialog.h cpp/ui/worlddetails/regexdialog.cpp cpp/ui/worlddetails/regexdialog.ui
)

set_property(SOURCE cpp/ui/settings/appearance.ui PROPERTY AUTOUIC_OPTIONS "--no-autoconnection")

if (APPLE)
  target_sources(${APP_NAME} PRIVATE cpp/native/window.mm)


    find_library(APPKIT_LIBRARY AppKit)
    find_library(FOUNDATION_LIBRARY Foundation)

    target_link_libraries(${APP_NAME} PRIVATE ${APPKIT_LIBRARY} ${FOUNDATION_LIBRARY})
endif()

target_include_directories(${APP_NAME}
  PRIVATE
  cpp/bridge
  cpp/ui/components
)

target_link_libraries(${APP_NAME}
  PRIVATE
  lualibs
  ${CRATE}
  Qt::Core
  Qt::Gui
  Qt::Network
  Qt::PrintSupport
  Qt::Widgets
)

add_compile_definitions(
  QT_NO_CAST_FROM_BYTEARRAY
  QT_NO_CAST_FROM_ASCII
  CMAKE_ORG_NAME="${CMAKE_PROJECT_NAME}"
  CMAKE_APP_NAME="${CMAKE_PROJECT_NAME}"
  CMAKE_APP_VERSION="${CMAKE_PROJECT_VERSION}"
)

# Enable Lua benchmarking
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  #add_compile_definitions(BENCHMARK_LUA)
else ()
  add_compile_definitions(BENCHMARK_LUA)
endif ()

if (MSVC)
  add_compile_definitions(STANDALONE_CLIENT)
  target_compile_options(
    ${APP_NAME}
    PRIVATE
    /W4
    /WX
    /wd4715
  )
else ()
  target_compile_options(
    ${APP_NAME}
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wno-dollar-in-identifier-extension
  )
  find_package(Qt6 REQUIRED COMPONENTS LinguistTools)
  qt_add_translations(
    TARGETS ${APP_NAME}
    SOURCE_TARGETS ${APP_NAME}
    TS_FILE_DIR languages
    LUPDATE_OPTIONS -no-obsolete
  )
endif ()

if (APPLE)
  find_library(AUDIO_UNIT AudioUnit)
  find_library(CORE_AUDIO CoreAudio)
  target_link_libraries(${APP_NAME} PRIVATE ${AUDIO_UNIT} ${CORE_AUDIO})
elseif (WIN32)
  target_link_libraries(${APP_NAME} PRIVATE propsys.lib)
endif ()

set_target_properties(${APP_NAME} PROPERTIES
    MACOSX_BUNDLE_ICON_FILE appicon.icns
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${APP_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
  add_compile_definitions(
    QT_NO_DEBUG_OUTPUT
    QT_NO_INFO_OUTPUT
    QT_NO_WARNING_OUTPUT
  )
endif ()

install(TARGETS ${APP_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (NOT MSVC)
  qt_generate_deploy_app_script(
    TARGET ${APP_NAME}
    OUTPUT_SCRIPT deploy_script
  )

  install(SCRIPT ${deploy_script})
endif ()

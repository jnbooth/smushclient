cmake_minimum_required(VERSION 3.5)

set(CMAKE_OSX_DEPLOYMENT_TARGET 15.1)
project(SmushClient VERSION 0.1 LANGUAGES CXX)
set(APP_NAME ${PROJECT_NAME})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(MSVC true)

# Rust always links against non-debug Windows runtime on *-msvc targets
# Note it is best to set this on the command line to ensure all targets are consistent
# https://github.com/corrosion-rs/corrosion/blob/master/doc/src/common_issues.md#linking-debug-cc-libraries-into-rust-fails-on-windows-msvc-targets
# https://github.com/rust-lang/rust/issues/39016
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
  Core
  Gui
  LinguistTools
  Multimedia
  Network
  Qml
  PrintSupport
  Widgets
)

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
  GIT_TAG 1031ea5b7a16e0365687c7e40c440dc9e1591274
)

FetchContent_MakeAvailable(
  CxxQt
  lualibs
)

set(LUACONF_PATH ${lualibs_SOURCE_DIR}/lua/lua-5.4.7/include/luaconf.h)
file(READ ${LUACONF_PATH} LUACONF_CONTENTS)
if(LUACONF_CONTENTS MATCHES "#define LUA_INT_TYPE	LUA_INT_DEFAULT")
  string(REPLACE
    "#define LUA_INT_TYPE	LUA_INT_DEFAULT"
    "#define LUA_INT_TYPE	LUA_INT_INT"
    LUACONF_CONTENTS
    "${LUACONF_CONTENTS}"
  )
  file(WRITE ${LUACONF_PATH} "${LUACONF_CONTENTS}")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cpp/bridge)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cpp/ui/components)

set(CRATE smushclient_qt)
set(cargo_target_dir cargo/build)

cxx_qt_import_crate(
  MANIFEST_PATH rust/Cargo.toml
  CRATES ${CRATE}
  QT_MODULES Qt::Core Qt::Gui Qt::Multimedia Qt::Qml Qt::PrintSupport Qt::Widgets
)

corrosion_set_env_vars(${CRATE} "CMAKE_DIR=${CMAKE_CURRENT_BINARY_DIR}")

cxx_qt_import_qml_module(${CRATE}_qml
    URI "com.jnbooth.smushclient"
    SOURCE_CRATE ${CRATE}
)

qt_add_executable(${APP_NAME}
    cpp/main.cpp
    cpp/audio.h cpp/audio.cpp
    cpp/commandhistory.h cpp/commandhistory.cpp
    cpp/enumbuttongroup.h
    cpp/environment.h cpp/environment.cpp
    cpp/fieldconnector.h
    cpp/hotkeys.h cpp/hotkeys.cpp
    cpp/layout.h cpp/layout.cpp
    cpp/localization.h cpp/localization.cpp
    cpp/settings.h cpp/settings.cpp
    cpp/spans.h cpp/spans.cpp

    cpp/bridge/document.h cpp/bridge/document.cpp
    cpp/bridge/timekeeper.h cpp/bridge/timekeeper.cpp

    cpp/model/alias.h cpp/model/alias.cpp
    cpp/model/plugin.h cpp/model/plugin.cpp
    cpp/model/sender.h cpp/model/sender.cpp
    cpp/model/timer.h cpp/model/timer.cpp
    cpp/model/trigger.h cpp/model/trigger.cpp

    cpp/scripting/callbacktrigger.h cpp/scripting/callbacktrigger.cpp
    cpp/scripting/databaseconnection.h cpp/scripting/databaseconnection.cpp
    cpp/scripting/errors.h cpp/scripting/errors.cpp
    cpp/scripting/hotspot.h cpp/scripting/hotspot.cpp
    cpp/scripting/imagefilters.h cpp/scripting/imagefilters.cpp
    cpp/scripting/luaapi.h cpp/scripting/luaapi.cpp
    cpp/scripting/luaglobals.h cpp/scripting/luaglobals.cpp
    cpp/scripting/luautils.cpp
    cpp/scripting/miniwindow.h cpp/scripting/miniwindow.cpp
    cpp/scripting/plugin.h cpp/scripting/plugin.cpp
    cpp/scripting/plugincallback.h cpp/scripting/plugincallback.cpp
    cpp/scripting/qlua.h cpp/scripting/qlua.cpp
    cpp/scripting/scriptapi_api.cpp
    cpp/scripting/scriptapi_getinfo.cpp
    cpp/scripting/scriptapi_senders.cpp
    cpp/scripting/scriptapi_window.cpp
    cpp/scripting/scriptapi.h cpp/scripting/scriptapi.cpp
    cpp/scripting/scriptthread.h cpp/scripting/scriptthread.cpp
    cpp/scripting/worldproperties.h cpp/scripting/worldproperties.cpp

    cpp/ui/finddialog.h cpp/ui/finddialog.cpp cpp/ui/finddialog.ui
    cpp/ui/mainwindow.h cpp/ui/mainwindow.cpp cpp/ui/mainwindow.ui
    cpp/ui/notepad.h cpp/ui/notepad.cpp cpp/ui/notepad.ui
    cpp/ui/worldtab.h cpp/ui/worldtab.cpp cpp/ui/worldtab.ui

    cpp/ui/components/colorpickerbutton.h cpp/ui/components/colorpickerbutton.cpp
    cpp/ui/components/iconlabel.h cpp/ui/components/iconlabel.cpp
    cpp/ui/components/mudbrowser.h cpp/ui/components/mudbrowser.cpp
    cpp/ui/components/mudinput.h cpp/ui/components/mudinput.cpp
    cpp/ui/components/mudscrollbar.h cpp/ui/components/mudscrollbar.cpp
    cpp/ui/components/mudstatusbar.h cpp/ui/components/mudstatusbar.cpp

    cpp/ui/scripting/abstractscriptdialog.h cpp/ui/scripting/abstractscriptdialog.cpp
    cpp/ui/scripting/choose.h cpp/ui/scripting/choose.cpp cpp/ui/scripting/choose.ui
    cpp/ui/scripting/listbox.h cpp/ui/scripting/listbox.cpp cpp/ui/scripting/listbox.ui

    cpp/ui/settings/appearance.h cpp/ui/settings/appearance.cpp cpp/ui/settings/appearance.ui
    cpp/ui/settings/history.h cpp/ui/settings/history.cpp cpp/ui/settings/history.ui
    cpp/ui/settings/notifier.h
    cpp/ui/settings/settings.h cpp/ui/settings/settings.cpp cpp/ui/settings/settings.ui
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
    cpp/ui/worlddetails/scripting.h cpp/ui/worlddetails/scripting.cpp cpp/ui/worlddetails/scripting.ui
    cpp/ui/worlddetails/specialhelp.h cpp/ui/worlddetails/specialhelp.cpp cpp/ui/worlddetails/specialhelp.ui
    cpp/ui/worlddetails/timeredit.h cpp/ui/worlddetails/timeredit.cpp cpp/ui/worlddetails/timeredit.ui
    cpp/ui/worlddetails/timers.h cpp/ui/worlddetails/timers.cpp cpp/ui/worlddetails/timers.ui
    cpp/ui/worlddetails/triggeredit.h cpp/ui/worlddetails/triggeredit.cpp cpp/ui/worlddetails/triggeredit.ui
    cpp/ui/worlddetails/triggers.h cpp/ui/worlddetails/triggers.cpp cpp/ui/worlddetails/triggers.ui
    cpp/ui/worlddetails/worlddetails.h cpp/ui/worlddetails/worlddetails.cpp cpp/ui/worlddetails/worlddetails.ui

    resources/resources.qrc
    cpp/ui/settings/sound.h cpp/ui/settings/sound.cpp cpp/ui/settings/sound.ui
    cpp/ui/settings/connection.h cpp/ui/settings/connection.cpp cpp/ui/settings/connection.ui
    cpp/ui/settings/closing.h cpp/ui/settings/closing.cpp cpp/ui/settings/closing.ui
    cpp/ui/serverstatus.h cpp/ui/serverstatus.cpp cpp/ui/serverstatus.ui
)

qt_add_translations(
  TARGETS ${APP_NAME}
  SOURCE_TARGETS ${APP_NAME}
  TS_FILE_DIR languages
  LUPDATE_OPTIONS -no-obsolete
)

target_link_libraries(${APP_NAME} PRIVATE lualibs ${CRATE}_qml)

add_compile_definitions(
  QT_NO_CAST_FROM_BYTEARRAY
  QT_NO_CAST_FROM_ASCII
)

# Enable Lua benchmarking
if (CMAKE_BUILD_TYPE STREQUAL "Release")
#add_compile_definitions(BENCHMARK_LUA)
else ()
add_compile_definitions(BENCHMARK_LUA)
endif()


if(MSVC)
  target_compile_definitions(lualibs PRIVATE strncasecmp=_strnicmp)
  target_compile_options(
    lualibs
    PRIVATE
    /wd4267
    /wd4244
    /wd4005
    /wd4996
    /wd4311
  )
  target_compile_options(
    ${APP_NAME}
    PRIVATE
    /W4
    /WX
    /wd4715
  )
else()
  target_compile_options(
    ${APP_NAME}
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wno-dollar-in-identifier-extension
)
endif()

qt_import_qml_plugins(${APP_NAME})

set_target_properties(${APP_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ${APP_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
  TARGET ${APP_NAME}
  OUTPUT_SCRIPT deploy_script
)

install(SCRIPT ${deploy_script})

cmake_minimum_required(VERSION 3.24)

set(CMAKE_OSX_DEPLOYMENT_TARGET 15)
project(SmushClient VERSION 0.1 LANGUAGES CXX)
set(APP_NAME ${PROJECT_NAME})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Rust always links against non-debug Windows runtime on *-msvc targets
# Note it is best to set this on the command line to ensure all targets are consistent
# https://github.com/corrosion-rs/corrosion/blob/master/doc/src/common_issues.md#linking-debug-cc-libraries-into-rust-fails-on-windows-msvc-targets
# https://github.com/rust-lang/rust/issues/39016
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
  Qt6
  COMPONENTS
  Core
  Gui
  Multimedia
  Network
  PrintSupport
  Qml
  Widgets
  REQUIRED
)

include(FetchContent)
FetchContent_Declare(
  Corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG v0.5
  FIND_PACKAGE_ARGS
)

FetchContent_Declare(
  lualibs
  GIT_REPOSITORY https://github.com/jnbooth/lua-batteries-included.git
  GIT_TAG 84faa49f324b8dd5c1b249b73cf2bd59e1f3b97a
)

FetchContent_MakeAvailable(
  Corrosion
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

# The path to the qmake executable path needs to be passed to the Rust
# library's build script to ensure it uses the same installation of Qt as CMake.
get_target_property(QMAKE Qt::qmake IMPORTED_LOCATION)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cpp/ui/components)

set(PROJECT_SOURCES
        cpp/main.cpp
)

qt_add_executable(${APP_NAME}
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    cpp/audio.h cpp/audio.cpp
    cpp/commandhistory.h cpp/commandhistory.cpp
    cpp/enumbuttongroup.h
    cpp/environment.h cpp/environment.cpp
    cpp/fieldconnector.h
    cpp/hotkeys.h cpp/hotkeys.cpp
    cpp/settings.h cpp/settings.cpp
    cpp/spans.h cpp/spans.cpp

    cpp/bridge/document.h cpp/bridge/document.cpp
    cpp/bridge/timekeeper.h cpp/bridge/timekeeper.cpp
    cpp/bridge/viewbuilder.h cpp/bridge/viewbuilder.cpp

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
    cpp/ui/settings.h cpp/ui/settings.cpp cpp/ui/settings.ui
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
    cpp/ui/settings/notifier.h

    cpp/ui/worlddetails/abstractprefstree.h cpp/ui/worlddetails/abstractprefstree.cpp
    cpp/ui/worlddetails/aliasedit.h cpp/ui/worlddetails/aliasedit.cpp cpp/ui/worlddetails/aliasedit.ui
    cpp/ui/worlddetails/aliases.h cpp/ui/worlddetails/aliases.cpp cpp/ui/worlddetails/aliases.ui
    cpp/ui/worlddetails/connecting.h cpp/ui/worlddetails/connecting.cpp cpp/ui/worlddetails/connecting.ui
    cpp/ui/worlddetails/login.h cpp/ui/worlddetails/login.cpp cpp/ui/worlddetails/login.ui
    cpp/ui/worlddetails/logging.h cpp/ui/worlddetails/logging.cpp cpp/ui/worlddetails/logging.ui
    cpp/ui/worlddetails/mud.h cpp/ui/worlddetails/mud.cpp cpp/ui/worlddetails/mud.ui
    cpp/ui/worlddetails/numpad.h cpp/ui/worlddetails/numpad.cpp cpp/ui/worlddetails/numpad.ui
    cpp/ui/worlddetails/output.h cpp/ui/worlddetails/output.cpp cpp/ui/worlddetails/output.ui
    cpp/ui/worlddetails/plugins.h cpp/ui/worlddetails/plugins.cpp cpp/ui/worlddetails/plugins.ui
    cpp/ui/worlddetails/scripting.h cpp/ui/worlddetails/scripting.cpp cpp/ui/worlddetails/scripting.ui
    cpp/ui/worlddetails/specialhelp.h cpp/ui/worlddetails/specialhelp.cpp cpp/ui/worlddetails/specialhelp.ui
    cpp/ui/worlddetails/timeredit.h cpp/ui/worlddetails/timeredit.cpp cpp/ui/worlddetails/timeredit.ui
    cpp/ui/worlddetails/timers.h cpp/ui/worlddetails/timers.cpp cpp/ui/worlddetails/timers.ui
    cpp/ui/worlddetails/triggeredit.h cpp/ui/worlddetails/triggeredit.cpp cpp/ui/worlddetails/triggeredit.ui
    cpp/ui/worlddetails/triggers.h cpp/ui/worlddetails/triggers.cpp cpp/ui/worlddetails/triggers.ui
    cpp/ui/worlddetails/worlddetails.h cpp/ui/worlddetails/worlddetails.cpp cpp/ui/worlddetails/worlddetails.ui

    resources/resources.qrc
    cpp/ui/worlddetails/pluginpopup.h cpp/ui/worlddetails/pluginpopup.cpp cpp/ui/worlddetails/pluginpopup.ui
)
if(MSVC)
  target_compile_options(${APP_NAME} PRIVATE /W4 /WX)
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

target_link_libraries(${APP_NAME} PUBLIC lualibs)

set(CRATE smushclient_qt)
# Corrosion creates a CMake target with the same name as the crate.
corrosion_import_crate(MANIFEST_PATH rust/Cargo.toml CRATES ${CRATE})

# The Rust library's build script needs to be told where to output the
# generated headers so CMake can find them. To do this, tell Corrosion
# to set the CXXQT_EXPORT_DIR environment variable when calling `cargo build`.
# Also, set the QMAKE environment variable to ensure the Rust library uses
# the same installation of Qt as CMake.
set(CXXQT_EXPORT_DIR "${CMAKE_CURRENT_BINARY_DIR}/cxxqt")
corrosion_set_env_vars(${CRATE}
    "CXXQT_EXPORT_DIR=${CXXQT_EXPORT_DIR}"
    "QMAKE=${QMAKE}"
)

# Create an INTERFACE library target to link libraries to and add include paths.
# Linking this to both the application and the tests avoids having to setup
# the include paths and linked libraries for both of those.
add_library(${APP_NAME}_lib INTERFACE)

# Include the headers generated by the Rust library's build script. Each
# crate gets its own subdirectory under CXXQT_EXPORT_DIR. This allows you
# to include headers generated by multiple crates without risk of one crate
# overwriting another's files.
target_include_directories(${APP_NAME}_lib INTERFACE "${CXXQT_EXPORT_DIR}/${CRATE}")

target_link_libraries(${APP_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(${APP_NAME}_lib INTERFACE
    # WHOLE_ARCHIVE is needed for the generated QML plugin to register on startup,
    # otherwise the linker will discard the static variables that initialize it.
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,${CRATE}-static>"
    Qt::Core
    Qt::Gui
    Qt::Multimedia
    Qt::Network
    Qt::PrintSupport
    Qt::Qml
    Qt::Widgets
)

# Link to the Rust library
target_link_libraries(${APP_NAME} PRIVATE ${APP_NAME}_lib)
target_include_directories(${APP_NAME} PUBLIC ${SOURCE_DIR}/src)

add_compile_definitions(QT_NO_CAST_FROM_BYTEARRAY QT_NO_CAST_FROM_ASCII)

# Enable Lua benchmarking
if (CMAKE_BUILD_TYPE STREQUAL "Release")
#add_compile_definitions(BENCHMARK_LUA)
else ()
add_compile_definitions(BENCHMARK_LUA)
endif()

# If we are using a statically linked Qt then we need to import any qml plugins
qt_import_qml_plugins(${APP_NAME})

set_target_properties(${APP_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${APP_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(${APP_NAME})

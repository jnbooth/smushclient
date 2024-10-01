cmake_minimum_required(VERSION 3.24)

set(CMAKE_OSX_DEPLOYMENT_TARGET 15.0)
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

find_package(Qt6 COMPONENTS Core Gui Network Qml Widgets REQUIRED)

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
  GIT_TAG 7135e9961d421e57db7316a5b8869baf21d91984
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
    cpp/enumbuttongroup.h
    cpp/fieldconnector.h
    cpp/settings.h cpp/settings.cpp

    cpp/bridge/document.h cpp/bridge/document.cpp
    cpp/bridge/viewbuilder.h cpp/bridge/viewbuilder.cpp

    cpp/scripting/databaseconnection.h cpp/scripting/databaseconnection.cpp
    cpp/scripting/hotspot.h cpp/scripting/hotspot.cpp
    cpp/scripting/imagefilters.h cpp/scripting/imagefilters.cpp
    cpp/scripting/luaapi.h cpp/scripting/luaapi.cpp
    cpp/scripting/luautils.cpp
    cpp/scripting/luaglobals.h cpp/scripting/luaglobals.cpp
    cpp/scripting/miniwindow.h cpp/scripting/miniwindow.cpp
    cpp/scripting/plugincallback.h cpp/scripting/plugincallback.cpp
    cpp/scripting/qlua.h cpp/scripting/qlua.cpp
    cpp/scripting/scriptapi.h cpp/scripting/scriptapi.cpp
    cpp/scripting/scriptapi_api.cpp
    cpp/scripting/scriptapi_getinfo.cpp
    cpp/scripting/scriptapi_window.cpp
    cpp/scripting/scriptthread.h cpp/scripting/scriptthread.cpp
    cpp/scripting/plugin.h cpp/scripting/plugin.cpp
    cpp/scripting/worldproperties.h cpp/scripting/worldproperties.cpp

    cpp/ui/app.h cpp/ui/app.cpp cpp/ui/app.ui
    cpp/ui/components/colorpickerbutton.h cpp/ui/components/colorpickerbutton.cpp
    cpp/ui/pluginsdialog.h cpp/ui/pluginsdialog.cpp cpp/ui/pluginsdialog.ui
    cpp/ui/prefs/abstractprefstree.h cpp/ui/prefs/abstractprefstree.cpp
    cpp/ui/prefs/address.h cpp/ui/prefs/address.cpp cpp/ui/prefs/address.ui
    cpp/ui/prefs/aliasedit.h cpp/ui/prefs/aliasedit.cpp cpp/ui/prefs/aliasedit.ui
    cpp/ui/prefs/aliases.h cpp/ui/prefs/aliases.cpp cpp/ui/prefs/aliases.ui
    cpp/ui/prefs/chat.h cpp/ui/prefs/chat.cpp cpp/ui/prefs/chat.ui
    cpp/ui/prefs/color.h cpp/ui/prefs/color.cpp cpp/ui/prefs/color.ui
    cpp/ui/prefs/commands.h cpp/ui/prefs/commands.cpp cpp/ui/prefs/commands.ui
    cpp/ui/prefs/connecting.h cpp/ui/prefs/connecting.cpp cpp/ui/prefs/connecting.ui
    cpp/ui/prefs/logging.h cpp/ui/prefs/logging.cpp cpp/ui/prefs/logging.ui
    cpp/ui/prefs/mxp.h cpp/ui/prefs/mxp.cpp cpp/ui/prefs/mxp.ui
    cpp/ui/prefs/output.h cpp/ui/prefs/output.cpp cpp/ui/prefs/output.ui
    cpp/ui/prefs/proxy.h cpp/ui/prefs/proxy.cpp cpp/ui/prefs/proxy.ui
    cpp/ui/prefs/specialhelp.h cpp/ui/prefs/specialhelp.cpp cpp/ui/prefs/specialhelp.ui
    cpp/ui/prefs/timeredit.h cpp/ui/prefs/timeredit.cpp cpp/ui/prefs/timeredit.ui
    cpp/ui/prefs/timers.h cpp/ui/prefs/timers.cpp cpp/ui/prefs/timers.ui
    cpp/ui/prefs/triggeredit.h cpp/ui/prefs/triggeredit.cpp cpp/ui/prefs/triggeredit.ui
    cpp/ui/prefs/triggers.h cpp/ui/prefs/triggers.cpp cpp/ui/prefs/triggers.ui
    cpp/ui/worldprefs.h cpp/ui/worldprefs.cpp cpp/ui/worldprefs.ui
    cpp/ui/worldtab.h cpp/ui/worldtab.cpp cpp/ui/worldtab.ui
)
if(MSVC)
  target_compile_options(${APP_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra -Wpedantic)
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
    Qt::Network
    Qt::Qml
    Qt::Widgets
)

# Link to the Rust library
target_link_libraries(${APP_NAME} PRIVATE ${APP_NAME}_lib)
target_include_directories(${APP_NAME} PUBLIC ${SOURCE_DIR}/src)

add_compile_definitions(QT_NO_CAST_FROM_BYTEARRAY QT_NO_CAST_FROM_ASCII)

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

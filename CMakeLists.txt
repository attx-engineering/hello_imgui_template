cmake_minimum_required(VERSION 3.12)
project(helloworld_with_helloimgui)
set(CMAKE_CXX_STANDARD 17)

# Build hello_imgui
# =================
# 1/  Option 1: if you added hello_imgui as ak subfolder, you can add it to your project with:
if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/hello_imgui)
    add_subdirectory(hello_imgui)
endif()

# 2/  Option 2: simply fetch hello_imgui during the build
if (NOT TARGET hello_imgui)
    message(STATUS "Fetching hello_imgui")
    include(FetchContent)
    FetchContent_Declare(hello_imgui GIT_REPOSITORY https://github.com/attx-engineering/hello_imgui.git GIT_TAG imgui_v1-91_docking)
    FetchContent_MakeAvailable(hello_imgui)
endif()

# 3/  Option 3: via vcpkg
# i/ You can install hello_imgui via vcpkg with:
#     vcpkg install "hello-imgui[opengl3-binding,glfw-binding]"
# ii/ Then you can use it inside CMake with:
#     find_package(hello-imgui CONFIG REQUIRED)


# Build your app
# ==============
# hello_imgui_add_app is a helper function, similar to cmake's "add_executable"
# Usage:
# hello_imgui_add_app(app_name file1.cpp file2.cpp ...)
#
# Features:
# * It will automatically link the target to the required libraries (hello_imgui, OpenGl, glad, etc)
# * It will embed the assets (for desktop, mobile, and emscripten apps)
# * It will perform additional customization (app icon and name on mobile platforms, etc)

# Now you can build your app with
#     mkdir build && cd build && cmake .. && cmake --build .

# Export our project dir for access in C++
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    # Do nothing. We have a parent directory that will define locations
else()
    if(CONFIG_RELEASE)
        # We are configuring for release and should define our locations to generic ones
        add_definitions(-DMODELSPACE_ROOT_DIR="/usr/share/modelspace")
        add_definitions(-DCLOCKWERK_ROOT_DIR="/usr/share/modelspace")
        add_definitions(-DPROJECT_ROOT_DIR="/usr/share/modelspace")
        add_definitions(-DLICENSE_ROOT_DIR="/usr/share/modelspace")
    else()
        # We are configuring to run from a local system and should set our locations accordingly
        add_definitions(-DMODELSPACE_ROOT_DIR="${PROJECT_SOURCE_DIR}")
        add_definitions(-DCLOCKWERK_ROOT_DIR="${PROJECT_SOURCE_DIR}/clockwerk")
        add_definitions(-DPROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")
        add_definitions(-DLICENSE_ROOT_DIR="${CMAKE_SOURCE_DIR}/license")
    endif()
endif()
if(CONFIG_RELEASE)
else()
  add_definitions(-DSIM_NO_CHECK_LICENSE)
endif()

# Some dependencies
add_subdirectory(${PROJECT_SOURCE_DIR}/json)
include_directories(${PROJECT_SOURCE_DIR}/json/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/imgui-node-editor)
include_directories(${PROJECT_SOURCE_DIR}/modelspace/cpp/src)
include_directories(${PROJECT_SOURCE_DIR}/ms-gui-app)
include_directories(${PROJECT_SOURCE_DIR}/assets)
# add_subdirectory(${PROJECT_SOURCE_DIR}/ms-gui-app)
include_directories(${PROJECT_SOURCE_DIR}/hello_imgui/external/imgui)
include_directories(${PROJECT_SOURCE_DIR}/clockwerk/src)
# This call to the clockwerk submodule cmake to build out our core utils
add_subdirectory(clockwerk)
include_directories(clockwerk/src)

# Add the slimmed-down modelspace library
add_subdirectory(${PROJECT_SOURCE_DIR}/modelspace/cpp)
include_directories(${PROJECT_SOURCE_DIR}/modelspace/cpp/include)

# Find our packages
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/imgui-node-editor/misc/cmake-modules)
find_package(stb_image REQUIRED)
find_package(ScopeGuard REQUIRED)

# Add our app resources
file(GLOB _App_CommonResources CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/ms-gui-app/data/*")
set(_AppBinDir ${CMAKE_BINARY_DIR}/bin)

# Build our application -- set initial app source list to be the source files in app/application
set(APP_SRC ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/application.cpp 
            ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/widgets.cpp
            ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/drawing.cpp
            ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/entry_point.cpp 
            ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/imgui_extra_keys.h 
            ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/config.h.in 
            ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/setup.h 
            ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/platform.h 
            ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/platform_win32.cpp 
            ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/platform_glfw.cpp 
            ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/renderer.h 
            ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/renderer_dx11.cpp 
            ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/renderer_ogl3.cpp)

# Append all the features developed in the app directory
file(GLOB APP_CUSTOM_SRC ${PROJECT_SOURCE_DIR}/ms-gui-app/app/*.cpp)
file(GLOB APP_CONFIG_SRC ${PROJECT_SOURCE_DIR}/ms-gui-app/configuration/*.cpp)

# Also add some modelspace source files
file(GLOB APP_MODELSPACE_SRC ${PROJECT_SOURCE_DIR}/modelspace/cpp/src/*.cpp)

# And also local assets
file(GLOB APP_ASSETS_SRC ${PROJECT_SOURCE_DIR}/assets/*.cpp)

list(APPEND APP_SRC ${APP_CUSTOM_SRC} ${APP_CONFIG_SRC} ${APP_MODELSPACE_SRC} ${APP_ASSETS_SRC})

# Add an application executable from our sources
hello_imgui_add_app(ms-gui ${APP_SRC} ${_App_CommonResources})

set_target_properties(ms-gui PROPERTIES
    FOLDER "app"
    RUNTIME_OUTPUT_DIRECTORY                "${_AppBinDir}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${_AppBinDir}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_AppBinDir}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${_AppBinDir}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${_AppBinDir}"
    DEBUG_POSTFIX                           _d
    RELWITHDEBINGO_POSTFIX                  _rd
    MINSIZEREL_POSTFIX                      _r
    VS_DEBUGGER_WORKING_DIRECTORY           ${_AppBinDir}
    MACOSX_BUNDLE_INFO_PLIST                "${IMGUI_NODE_EDITOR_ROOT_DIR}/examples/application/support/Info.plist.in"
    MACOSX_BUNDLE_BUNDLE_NAME               "${PACKAGE_NAME}"
    MACOSX_BUNDLE_GUI_IDENTIFIER            "com.sandbox.collisions"
    MACOSX_BUNDLE_LONG_VERSION_STRING       "${PACKAGE_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING      "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}"
    MACOSX_BUNDLE_ICON_FILE                 Icon.icns
)

set(_ResourceRoot ${CMAKE_CURRENT_SOURCE_DIR})
foreach(_Resource ROOT "${PROJECT_SOURCE_DIR}/ms-gui-app/data" ${_App_CommonResources} ROOT "${PROJECT_SOURCE_DIR}/ms-gui-app/data" ${_App_Resources})
    if (_Resource STREQUAL ROOT)
        set(_ResourceRoot FALSE)
        continue()
    elseif(NOT _ResourceRoot)
        set(_ResourceRoot ${_Resource})
        continue()
    endif()

    if ("${_Resource}" MATCHES "\.DS_Store$")
        list(REMOVE_ITEM _App_Resources ${_Resource})
        list(REMOVE_ITEM _App_CommonResources ${_Resource})
        continue()
    endif()

    file(RELATIVE_PATH _RelResource ${_ResourceRoot} ${_Resource})

    add_custom_command(
        TARGET ms-gui
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ARGS ${_Resource} ${_AppBinDir}/../data/${_RelResource}
    )
endforeach()

# And set target include directories to be tied to the app directory
target_include_directories(ms-gui PUBLIC ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/include)
target_include_directories(ms-gui PUBLIC ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source)
target_include_directories(ms-gui PUBLIC ${PROJECT_SOURCE_DIR}/ms-gui-app/app/)
target_include_directories(ms-gui PUBLIC ${PROJECT_SOURCE_DIR}/ms-gui-app/configuration/)
target_include_directories(ms-gui PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/source)
# target_include_directories(ms-gui PUBLIC ${PROJECT_SOURCE_DIR}/assets)
target_include_directories(ms-gui PUBLIC ${PROJECT_SOURCE_DIR}/hello_imgui/external)
target_include_directories(ms-gui PUBLIC ${PROJECT_SOURCE_DIR}/hello_imgui/external/stb_image)
target_include_directories(ms-gui PUBLIC ${PROJECT_SOURCE_DIR}/clockwerk/src)

# Set target compile options
target_compile_options(ms-gui PRIVATE "-Wno-unused-function")

# And link libraries
target_link_libraries(ms-gui PUBLIC imgui imgui_node_editor modelspace clockwerk)
target_link_libraries(ms-gui PRIVATE stb_image ScopeGuard)

# Link the slimmed-down modelspace library to the ms-gui target
target_link_libraries(ms-gui PUBLIC modelspace)

if (WIN32)
    list(APPEND APP_SRC
        ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/imgui_impl_dx11.cpp
        ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/imgui_impl_dx11.h
        ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/imgui_impl_win32.cpp
        ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/imgui_impl_win32.h
    )

    set(_DXSDK_Dir  ${IMGUI_NODE_EDITOR_ROOT_DIR}/external/DXSDK)
    set(_DXSDK_Arch x86)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        set(_DXSDK_Arch x64)
    endif()

    add_library(dxerr STATIC ${_DXSDK_Dir}/src/dxerr.cpp)
    target_include_directories(dxerr PUBLIC "${_DXSDK_Dir}/include")
    set_property(TARGET dxerr PROPERTY FOLDER "external")

    add_library(d3dx11 UNKNOWN IMPORTED)
    set_target_properties(d3dx11 PROPERTIES
        IMPORTED_LOCATION               "${_DXSDK_Dir}/lib/${_DXSDK_Arch}/d3dx11.lib"
        IMPORTED_LOCATION_DEBUG         "${_DXSDK_Dir}/lib/${_DXSDK_Arch}/d3dx11d.lib"
        INTERFACE_INCLUDE_DIRECTORIES   "${_DXSDK_Dir}/include"
        INTERFACE_LINK_LIBRARIES        "$<$<CONFIG:Debug>:dxerr>"
    )

    target_link_libraries(ms-gui PRIVATE d3d11.lib d3dcompiler.lib d3dx11)
else()
    find_package(OpenGL REQUIRED)
    find_package(glfw3 3 REQUIRED)

    if (APPLE)
        target_link_libraries(ms-gui PRIVATE "-framework CoreFoundation" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
    endif()
endif()

if (OpenGL_FOUND)
    set(HAVE_OPENGL YES)

    target_include_directories(ms-gui PRIVATE ${OPENGL_INCLUDE_DIR})
    target_link_libraries(ms-gui PRIVATE ${OPENGL_gl_LIBRARY})
    list(APPEND APP_SRC
        ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/imgui_impl_opengl3.cpp
        ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/imgui_impl_opengl3.h
        ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/imgui_impl_opengl3_loader.h
    )
endif()

if (glfw3_FOUND)
    set(HAVE_GLFW3 YES)

    list(APPEND APP_SRC
        ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/imgui_impl_glfw.cpp
        ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/imgui_impl_glfw.h
    )
    target_link_libraries(ms-gui PRIVATE glfw)
endif()

configure_file(
    ${PROJECT_SOURCE_DIR}/ms-gui-app/app/application/source/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/source/config.h
)

target_compile_definitions(ms-gui PRIVATE
    #BACKEND_CONFIG=IMGUI_GLFW
    #RENDERER_CONFIG=IMGUI_OGL3
)

target_include_directories(ms-gui PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/source)
# include_directories(${PROJECT_SOURCE_DIR}/build/source)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${APP_SRC})

target_sources(ms-gui PRIVATE ${APP_SRC})

set_property(TARGET ms-gui PROPERTY FOLDER "app")

# By default, we install in a portable way (i.e. assets and executable are in the same folder)
option(EXAMPLE_INTEGRATION_PORTABLE_INSTALL "Install in a portable way" ON)


if (EXAMPLE_INTEGRATION_PORTABLE_INSTALL)
    # portable installation is the easiest way to install the app
    hello_imgui_add_app(hello_world_ hello_world.main.cpp)
else()
    # Disable HelloImGui default install behavior
    set(HELLOIMGUI_ADD_APP_WITH_INSTALL OFF CACHE BOOL "" FORCE)

    imgui_bundle_add_app(hello_world_ hello_world.main.cpp)

    # If not portable, we have to
    # - install the app manually
    # - install the assets manually
    # - pass the assets location to the app, and set this assets location in main() via the compile definition ASSETS_LOCATION:
    #     #ifdef ASSETS_LOCATION \n HelloImGui::SetAssetsFolder(ASSETS_LOCATION); \n #endif
    if (NOT EXAMPLE_INTEGRATION_PORTABLE_INSTALL)
        include(GNUInstallDirs)
        install(TARGETS hello_world_ DESTINATION ${CMAKE_INSTALL_BINDIR})
        set(assets_install_dir ${CMAKE_INSTALL_DATADIR}/hello_world_)
        if (NOT IS_ABSOLUTE ${assets_install_dir})
            set(assets_install_dir ${CMAKE_INSTALL_PREFIX}/${assets_install_dir})
        endif()
        install(DIRECTORY assets DESTINATION ${assets_install_dir})
        target_compile_definitions(imgui_bundle_example_integration PRIVATE ASSETS_LOCATION="${assets_install_dir}/assets")
    endif()
endif()

# Print included directories for ms-gui target
get_target_property(ms_gui_included_dirs ms-gui INCLUDE_DIRECTORIES)
message(STATUS "ms-gui included directories: ${ms_gui_included_dirs}")

# Print source files for ms-gui target
get_target_property(ms_gui_sources ms-gui SOURCES)
message(STATUS "ms-gui sources: ${ms_gui_sources}")

message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
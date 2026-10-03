cmake_minimum_required(VERSION 3.16)

project(sciterui)

set(SCITERUI_SOURCES
    src/sciterui/base_element.cpp
    src/sciterui/event_handler.cpp
    src/sciterui/file.cpp
    src/sciterui/module_disk.cpp
    src/sciterui/module_resource.cpp
    src/sciterui/path.cpp
    src/sciterui/path_finder.cpp
    src/sciterui/resource_manager.cpp
    src/sciterui/sciter.cpp
    src/sciterui/sciter_dpi.cpp
    src/sciterui/sciter_element.cpp
    src/sciterui/sciter_ui.cpp
    src/sciterui/sciter_value.cpp
    src/sciterui/sciter_value_internal.cpp
    src/sciterui/sciter_window.cpp
    src/sciterui/std_string.cpp
    src/sciterui/widgets/combo_box.cpp
    src/sciterui/widgets/list_box.cpp
    src/sciterui/widgets/menubar.cpp
    src/sciterui/widgets/page_nav.cpp
    src/sciterui/widgets/tooltip_host.cpp
)

add_library(sciterui STATIC ${SCITERUI_SOURCES})

if(APPLE)
    enable_language(OBJCXX)
    target_sources(sciterui PRIVATE src/sciterui/sciter_window_macos.mm)
    target_link_libraries(sciterui PUBLIC "-framework AppKit")
endif()

target_include_directories(sciterui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sciterui
        ${CMAKE_CURRENT_SOURCE_DIR}/src/3rd_party/sciter-js-sdk-main/include
)

target_compile_features(sciterui PUBLIC cxx_std_20)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Use one loader so Sciter's bundled codecs cannot bind to GTK's codecs.
    target_compile_definitions(sciterui PUBLIC STATIC_LIB=1)
    target_link_libraries(sciterui PUBLIC ${CMAKE_DL_LIBS})
    # Linux delegates window creation and dispatch to Sciter's native engine.
    target_sources(sciterui PRIVATE
        src/sciterui/linux_engine_loop.cpp
        src/sciterui/sciter_runtime_linux.cpp
        src/sciterui/sciter_wayland_native.cpp
    )
    # The bundled Sciter runtime uses GTK4 windows on GNOME and wl_surface on
    # other Wayland desktops. AUTO supports both verified representations.
    set(SCITERUI_NATIVE_WINDOW_ABI "AUTO" CACHE STRING "Sciter Linux native window ABI")
    set_property(CACHE SCITERUI_NATIVE_WINDOW_ABI PROPERTY STRINGS AUTO GTK4_WIDGET WL_SURFACE)
    if(SCITERUI_NATIVE_WINDOW_ABI STREQUAL "AUTO" OR SCITERUI_NATIVE_WINDOW_ABI STREQUAL "GTK4_WIDGET")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK4_WAYLAND REQUIRED IMPORTED_TARGET gtk4-wayland)
        if(SCITERUI_NATIVE_WINDOW_ABI STREQUAL "AUTO")
            target_compile_definitions(sciterui PRIVATE SCITERUI_NATIVE_GTK4_OR_WL_SURFACE=1)
        else()
            target_compile_definitions(sciterui PRIVATE SCITERUI_NATIVE_GTK4_WIDGET=1)
        endif()
        target_link_libraries(sciterui PRIVATE PkgConfig::GTK4_WAYLAND)
    elseif(SCITERUI_NATIVE_WINDOW_ABI STREQUAL "WL_SURFACE")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
        target_compile_definitions(sciterui PRIVATE SCITERUI_NATIVE_WL_SURFACE=1)
        target_link_libraries(sciterui PRIVATE PkgConfig::WAYLAND_CLIENT)
    else()
        message(FATAL_ERROR "Unknown Sciter native ABI '${SCITERUI_NATIVE_WINDOW_ABI}'; select AUTO, GTK4_WIDGET or WL_SURFACE")
    endif()
endif()

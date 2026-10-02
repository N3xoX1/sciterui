cmake_minimum_required(VERSION 3.16)

project(sciterui)

option(SCITERUI_BUILD_TESTS "Build SciterUI regression tests" OFF)

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

if(UNIX AND NOT APPLE)
    target_sources(sciterui PRIVATE src/sciterui/x11_host.cpp)
    find_package(X11 REQUIRED)
    target_link_libraries(sciterui PUBLIC X11::X11)
endif()

if(SCITERUI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

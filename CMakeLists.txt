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
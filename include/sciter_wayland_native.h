#pragma once

struct SciterWaylandNative {
    void* display = nullptr; // borrowed from Sciter; valid until window destruction
    void* surface = nullptr; // borrowed from Sciter; valid until window destruction
};

// Resolve the bundled runtime's GTK4 or direct Wayland handle without relying
// on the desktop name. Other runtime handle representations are unsupported.
bool SciterUIGetWaylandNative(const void* sciter_window, SciterWaylandNative& output);

// Supply desktop identity and ownership through GTK's public API when Sciter
// owns a GTK window. Direct Wayland windows require support in the engine.
void SciterUIConfigureGtkWindow(const void* sciter_window, const void* parent, const char* application_id);

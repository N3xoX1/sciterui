#pragma once

struct SciterWaylandNative {
    void* display = nullptr; // borrowed from Sciter; valid until window destruction
    void* surface = nullptr; // borrowed from Sciter; valid until window destruction
};

// Resolve the bundled runtime's GTK4 or direct Wayland handle without relying
// on the desktop name. Other runtime handle representations are unsupported.
bool SciterUIGetWaylandNative(const void* sciter_window, SciterWaylandNative& output);

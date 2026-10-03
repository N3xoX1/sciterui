#include <sciter_wayland_native.h>

#include <cstring>

#if defined(SCITERUI_NATIVE_GTK4_OR_WL_SURFACE)
namespace
{
enum class NativeAbi { Unknown, Gtk4, Wayland };
// Sciter chooses its Linux window representation once for the process.
// Called on the Sciter UI thread, like all other window operations.
NativeAbi resolvedAbi = NativeAbi::Unknown;
}
#endif

#if defined(SCITERUI_NATIVE_GTK4_WIDGET) || defined(SCITERUI_NATIVE_GTK4_OR_WL_SURFACE)
#include <gtk/gtk.h>
#include <gdk/wayland/gdkwayland.h>

namespace
{
GtkWindow* FindGtkWindow(const void* handle)
{
    if (!handle)
        return nullptr;
    auto* windows = gtk_window_get_toplevels();
    for (guint i = 0, count = g_list_model_get_n_items(windows); i < count; ++i)
    {
        auto* object = g_list_model_get_item(windows, i);
        const bool matches = object == handle;
        // GTK and Sciter retain the window; return a borrowed reference.
        g_object_unref(object);
        if (matches)
            return GTK_WINDOW(object);
    }
    return nullptr;
}
}
#endif
#if defined(SCITERUI_NATIVE_WL_SURFACE) || defined(SCITERUI_NATIVE_GTK4_OR_WL_SURFACE)
#include <wayland-client.h>
#endif

void SciterUIConfigureGtkWindow(const void* sciter_window, const void* parent, const char* application_id)
{
#if defined(SCITERUI_NATIVE_GTK4_WIDGET) || defined(SCITERUI_NATIVE_GTK4_OR_WL_SURFACE)
    auto* window = FindGtkWindow(sciter_window);
    if (!window)
        return;
    if (auto* owner = FindGtkWindow(parent))
    {
        gtk_window_set_transient_for(window, owner);
        gtk_window_set_modal(window, TRUE);
    }
    auto* surface = gtk_native_get_surface(GTK_NATIVE(window));
    if (application_id && *application_id && surface && GDK_IS_WAYLAND_SURFACE(surface))
    {
        gdk_wayland_toplevel_set_application_id(GDK_TOPLEVEL(surface), application_id);
        gtk_window_set_icon_name(window, application_id);
    }
#else
    (void)sciter_window;
    (void)parent;
    (void)application_id;
#endif
}

bool SciterUIGetWaylandNative(const void* sciter_window, SciterWaylandNative& output)
{
    output = {};
    if (!sciter_window)
        return false;
#if defined(SCITERUI_NATIVE_GTK4_WIDGET) || defined(SCITERUI_NATIVE_GTK4_OR_WL_SURFACE)
    // Compare opaque handles against GTK's owned windows. Never cast a
    // potential wl_surface pointer to a GObject.
#if defined(SCITERUI_NATIVE_GTK4_OR_WL_SURFACE)
    if (resolvedAbi != NativeAbi::Wayland)
#endif
    {
        if (auto* window = FindGtkWindow(sciter_window))
        {
            auto* surface = gtk_native_get_surface(GTK_NATIVE(window));
            if (surface && GDK_IS_WAYLAND_SURFACE(surface))
            {
                auto* display = gdk_surface_get_display(surface);
                output.display = gdk_wayland_display_get_wl_display(display);
                output.surface = gdk_wayland_surface_get_wl_surface(surface);
            }
#if defined(SCITERUI_NATIVE_GTK4_OR_WL_SURFACE)
            if (output.display && output.surface)
                resolvedAbi = NativeAbi::Gtk4;
#endif
            return output.display && output.surface;
        }
#if defined(SCITERUI_NATIVE_GTK4_OR_WL_SURFACE)
        // When GTK is our backend, an unrecognised GTK window cannot be a
        // Wayland proxy. This also protects an initial unresolved GTK window.
        if (resolvedAbi == NativeAbi::Gtk4 || g_list_model_get_n_items(gtk_window_get_toplevels()) != 0)
            return false;
#endif
    }
#endif
#if defined(SCITERUI_NATIVE_WL_SURFACE) || defined(SCITERUI_NATIVE_GTK4_OR_WL_SURFACE)
    // The bundled runtime's direct Wayland backend returns wl_surface*.
    auto* surface = static_cast<wl_surface*>(const_cast<void*>(sciter_window));
    const char* type = wl_proxy_get_class(reinterpret_cast<wl_proxy*>(surface));
    if (!type || std::strcmp(type, "wl_surface") != 0)
        return false;
    output.display = wl_proxy_get_display(reinterpret_cast<wl_proxy*>(surface));
    output.surface = surface;
#if defined(SCITERUI_NATIVE_GTK4_OR_WL_SURFACE)
    if (output.display && output.surface)
        resolvedAbi = NativeAbi::Wayland;
#endif
#endif
    return output.display && output.surface;
}

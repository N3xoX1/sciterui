#pragma once

#include <sciter_ui.h>

namespace SciterUI
{

class SciterWindow;

// Owns the X11 display connection and the windows Sciter.Lite renders into.
class X11Host
{
public:
    static X11Host & Instance();

    bool Init();
    void Shutdown();
    void Run();
    void Stop();
    void RunModal(SciterWindow * window);

    HWINDOW Create(SciterWindow & window, HWINDOW parent, int x, int y, int width, int height, unsigned flags, bool startHidden);
    void Show(SciterWindow & window);
    void Hide(SciterWindow & window);
    void Center(SciterWindow & window);
    void ApplySize(SciterWindow & window, int x, int y, int width, int height);
    void FixMinSize(SciterWindow & window, int layoutWidth, int layoutHeight);
    void SetDocumentMinSize(SciterWindow & window, int cssWidth, int cssHeight);
    void SetResizable(SciterWindow & window, bool resizable);
    void SetTitle(SciterWindow & window, const char * title);
    void SetIcon(SciterWindow & window, const uint8_t * png, uint32_t size);
    void Close(SciterWindow & window);
    void Abandon(SciterWindow & window);

    void Invalidate(HWINDOW hwnd, int left, int top, int right, int bottom);
    void SetCursor(HWINDOW hwnd, uint32_t cursorId);

    int Dpi() const;
    void ScaleSize(int & width, int & height) const;
    void ClampToWorkArea(int & width, int & height) const;

private:
    X11Host() = default;
    X11Host(const X11Host &) = delete;
    X11Host & operator=(const X11Host &) = delete;

    bool m_ready = false;
};
}

#include "sciter_dpi.h"

#ifdef WIN32
#include <Windows.h>
#endif

#include <algorithm>

namespace SciterUI
{

void ScaleWindowSizeForDpi(HWINDOW parentWindow, int & width, int & height)
{
#ifdef WIN32
    HWND parent = (HWND)parentWindow;
    UINT dpi = USER_DEFAULT_SCREEN_DPI;

    typedef UINT(WINAPI * PFN_GetDpiForWindow)(HWND);
    typedef UINT(WINAPI * PFN_GetDpiForSystem)();
    static PFN_GetDpiForWindow pGetDpiForWindow = reinterpret_cast<PFN_GetDpiForWindow>(::GetProcAddress(::GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));
    static PFN_GetDpiForSystem pGetDpiForSystem = reinterpret_cast<PFN_GetDpiForSystem>(::GetProcAddress(::GetModuleHandleW(L"user32.dll"), "GetDpiForSystem"));

    if (parent != nullptr && pGetDpiForWindow != nullptr)
    {
        UINT v = pGetDpiForWindow(parent);
        if (v != 0)
        {
            dpi = v;
        }
    }
    else if (pGetDpiForSystem != nullptr)
    {
        UINT v = pGetDpiForSystem();
        if (v != 0)
        {
            dpi = v;
        }
    }
    else
    {
        HDC hdc = ::GetDC(nullptr);
        if (hdc != nullptr)
        {
            int v = ::GetDeviceCaps(hdc, LOGPIXELSX);
            if (v > 0)
            {
                dpi = static_cast<UINT>(v);
            }
            ::ReleaseDC(nullptr, hdc);
        }
    }

    width = ::MulDiv(width, static_cast<int>(dpi), USER_DEFAULT_SCREEN_DPI);
    height = ::MulDiv(height, static_cast<int>(dpi), USER_DEFAULT_SCREEN_DPI);

    RECT workArea{};
    HMONITOR monitor = parent != nullptr ? ::MonitorFromWindow(parent, MONITOR_DEFAULTTONEAREST) : ::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi{sizeof(MONITORINFO)};
    if (monitor != nullptr && ::GetMonitorInfoW(monitor, &mi))
    {
        workArea = mi.rcWork;
    }
    else if (!::SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0))
    {
        workArea.right = ::GetSystemMetrics(SM_CXSCREEN);
        workArea.bottom = ::GetSystemMetrics(SM_CYSCREEN);
    }

    const int maxWidth = std::max<int>(320, workArea.right - workArea.left);
    const int maxHeight = std::max<int>(240, workArea.bottom - workArea.top);
    if (width > maxWidth)
    {
        width = maxWidth;
    }
    if (height > maxHeight)
    {
        height = maxHeight;
    }
#else
    (void)parentWindow;
    (void)width;
    (void)height;
#endif
}

} // namespace SciterUI

#pragma once
#include "sciter_window.h"

namespace SciterUI
{
bool ShowMacOSContextMenu(std::shared_ptr<SciterWindow> window, SCITER_ELEMENT menu, SCITER_POINT point,
                         SCITER_ELEMENT source, bool & nativeShown);
}

#ifdef __OBJC__
@class NSMenu;
@class NSView;
@class NSEvent;
namespace SciterUI
{
NSMenu * CreateMacOSContextMenu(std::shared_ptr<SciterWindow> window, SCITER_ELEMENT menu,
                              SCITER_ELEMENT source = nullptr);
NSPoint MacOSContextMenuPoint(NSView * view, SCITER_POINT point, SciterElement root, NSEvent * event);
}
#endif

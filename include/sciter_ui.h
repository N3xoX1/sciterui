#pragma once
#include <memory>
#include <vector>

#ifdef _MSC_VER
#define suinterface __interface
#else
#define suinterface struct
#endif

typedef const void * SCITER_ELEMENT;
typedef const void * HWINDOW;

typedef struct
{
    int32_t x;
    int32_t y;
} SCITER_POINT;

suinterface IWindowDestroySink
{
    virtual void OnWindowDestroy(HWINDOW hWnd) = 0;
};

suinterface IWindowCloseSink
{
    virtual bool OnWindowCloseRequest(HWINDOW hWnd) = 0;
};

suinterface ISciterWindow
{
    virtual void Show() = 0;
    virtual void CenterWindow() = 0;
    virtual void FixMinSize() = 0;
    virtual HWINDOW GetHandle() const = 0;
    virtual uint32_t GetMinWidth() const = 0;
    virtual uint32_t GetMinHeight(uint32_t width) const = 0;
    virtual SCITER_ELEMENT GetRootElement() const = 0;
    virtual void OnDestroySinkAdd(IWindowDestroySink * Sink) = 0;
    virtual void OnDestroySinkRemove(IWindowDestroySink * Sink) = 0;
    virtual void OnCloseSinkAdd(IWindowCloseSink * Sink) = 0;
    virtual void OnCloseSinkRemove(IWindowCloseSink * Sink) = 0;
    virtual bool Destroy() = 0;
    virtual void RunModal() = 0;
    virtual bool IsClosed() const = 0;
};

suinterface IBaseElement
{
    virtual std::shared_ptr<void> GetInterface(const char * riid) = 0;
};

suinterface IWidget
{
    virtual void Attached(SCITER_ELEMENT element, IBaseElement * baseElement) = 0;
    virtual void Detached(SCITER_ELEMENT element) = 0;
    virtual std::shared_ptr<void> GetInterface(const char * riid) = 0;
};

suinterface ISciterUI;
typedef IWidget * (__stdcall * tyCreateWidget)(ISciterUI & SciterUI);
typedef void(__stdcall * tyReleaseWidget)(IWidget * widget);

enum SCITERUI_WINDOW_CREATE_FLAGS {
    SUIW_CHILD = (1 << 0), // child window only, if this flag is set all other flags ignored
    SUIW_ALPHA = (1 << 6), // transparent window ( e.g. WS_EX_LAYERED on Windows )
    SUIW_MAIN = (1 << 7), // main window of the app, will terminate the app on close
    SUIW_POPUP = (1 << 8), // the window is created as topmost window.
    SUIW_ENABLE_DEBUG = (1 << 9), // make this window inspector ready
    SUIW_HIDDEN = (1 << 10),  // Create window hidden, caller must show it explicitly
};

suinterface ISciterUI
{
    virtual bool AttachHandler(SCITER_ELEMENT elemHandle, const char * riid, void * pinterface) = 0;
    virtual bool DetachHandler(SCITER_ELEMENT elemHandle, const char * riid, void * pinterface) = 0;
    virtual std::shared_ptr<void> GetElementInterface(SCITER_ELEMENT elemHandle, const char * riid) = 0;
    virtual bool SetElementHtmlFromResource(SCITER_ELEMENT elemHandle, const char * uri) = 0;
    virtual bool LoadResource(const char * uri, std::vector<uint8_t> & data) = 0;
    virtual bool WindowCreate(HWINDOW parent, const char * baseHtml, int x, int y, int width, int height, unsigned int flags, ISciterWindow *& window) = 0;
    virtual void PopupShow(SCITER_ELEMENT hePopup, SCITER_ELEMENT heAnchor, uint32_t placement) = 0;
    virtual void PopupShowAt(SCITER_ELEMENT hePopup, SCITER_POINT pos, uint32_t placement) = 0;
    virtual void PopupHide(SCITER_ELEMENT he) = 0;
    virtual bool RegisterWidgetType(const char * name, tyCreateWidget createWidget, tyReleaseWidget releaseWidget,  const char * widgetCss) = 0;
    virtual void UpdateWindow(HWINDOW hwnd) = 0;
    virtual void Run() = 0;
    virtual void Stop() = 0;
    virtual void Shutdown() = 0;
};

bool SciterUIInit(const char * languageDir, const char * baseLanguage, const char * currentLanguage, bool Console, ISciterUI *& sciterUI);

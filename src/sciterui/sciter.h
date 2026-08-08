#pragma once

#include "base_element.h"
#include "resource_manager.h"
#include "sciter_window.h"
#include <memory>
#include <sciter_ui.h>
#include <set>
#include <stdint.h>
#include <vector>

#ifdef WIN32
#include <Windows.h>
#endif

namespace SciterUI
{

class Sciter :
    public ISciterUI
{
    typedef std::map<uint32_t, IWidget *> IWidgetMap;
    typedef std::map<uint32_t, std::shared_ptr<BaseElement>> ElementMap;
    typedef std::set<SciterWindow *> WindowSet;

    struct WidgetCallbackInfo
    {
        WidgetCallbackInfo()
        {
            sciter = nullptr;
            createWidget = nullptr;
            releaseWidget = nullptr;
        }
        IWidgetMap widgets;
        Sciter * sciter;
        tyCreateWidget createWidget;
        tyReleaseWidget releaseWidget;
    };

    typedef std::vector<std::unique_ptr<SciterWindow>> WindowList;
    typedef std::map<std::string, WidgetCallbackInfo> WidgetMap;

public:
    typedef int sui_callback ElementEventProc(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms);

    /**This structure is used by #SC_ATTACH_BEHAVIOR notification.
     *\copydoc SC_ATTACH_BEHAVIOR **/
    typedef struct SCN_ATTACH_BEHAVIOR
    {
        uint32_t code; /**< [in] one of the codes above.*/
        void * hwnd;   /**< [in] HWINDOW of the window this callback was attached to.*/

        SCITER_ELEMENT element;    /**< [in] target DOM element handle*/
        const char * behaviorName; /**< [in] zero terminated string, string appears as value of CSS behavior:"???" attribute.*/

        ElementEventProc * elementProc; /**< [out] pointer to ElementEventProc function.*/
        void * elementTag;              /**< [out] tag value, passed as is into pointer ElementEventProc function.*/

    } SCN_ATTACH_BEHAVIOR;
    typedef SCN_ATTACH_BEHAVIOR * LPSCN_ATTACH_BEHAVIOR;

    Sciter(const char * languageDir);

    bool Initialize(const char * baseLanguage, const char * currentLanguage, bool console);
    void WindowCreated(SciterWindow * window);
    void WindowDestroyed(SciterWindow * window);
    SciterWindow * FindSciterWindow(HWINDOW hwnd) const;
    uint32_t AttachWidget(LPSCN_ATTACH_BEHAVIOR lpab);

    // ISciterUI
    bool AttachHandler(SCITER_ELEMENT elemHandle, const char * riid, void * pinterface) override;
    bool DetachHandler(SCITER_ELEMENT elemHandle, const char * riid, void * pinterface) override;
    std::shared_ptr<void> GetElementInterface(SCITER_ELEMENT he, const char * riid) override;
    bool SetElementHtmlFromResource(SCITER_ELEMENT elemHandle, const char * uri) override;
    bool LoadResource(const char * uri, std::vector<uint8_t> & data) override;
    bool WindowCreate(HWINDOW parent, const char * baseHtml, int x, int y, int width, int height, unsigned int flags, ISciterWindow *& window) override;
    void PopupShow(SCITER_ELEMENT hePopup, SCITER_ELEMENT heAnchor, uint32_t placement) override;
    void PopupShowAt(SCITER_ELEMENT hePopup, SCITER_POINT pos, uint32_t placement) override;
    void PopupHide(SCITER_ELEMENT he) override;
    bool RegisterWidgetType(const char * name, tyCreateWidget createWidget, tyReleaseWidget releaseWidget, const char * widgetCss) override;
    void UpdateWindow(HWINDOW hwnd) override;
    void Run() override;
    void Stop() override;
    void Shutdown() override;

    ResourceManager & GetResourceManager(void);
#ifdef WIN32
    const std::wstring & WindowClass();
#endif

private:
    Sciter() = delete;
    Sciter(const Sciter &) = delete;
    Sciter & operator=(const Sciter &) = delete;

    int AttachWidgetProc(WidgetCallbackInfo * info, SCITER_ELEMENT he, uint32_t evtg, void * prms);
    static int sui_callback stAttachWidgetProc(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms);
    ResourceManager m_resourceManager;
    WindowSet m_windows;
    WindowList m_CreatedWindows;
    WidgetMap m_widgetFactory;
    ElementMap m_elementBases;
    uint32_t m_nextWidgetId;

#ifdef WIN32
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    std::wstring m_windowClass;
#endif

};

} // namespace SciterUI
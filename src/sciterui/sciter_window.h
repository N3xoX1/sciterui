#pragma once
#include "event_handler.h"
#include <memory>
#include <sciter_ui.h>
#include <set>
#include <string>
#include <vector>

namespace SciterUI
{
class Sciter;

#define SC_CALLBACK __stdcall

class SciterWindow :
    public ISciterWindow
{
    /**Notification callback structure.**/
    typedef struct SCITER_CALLBACK_NOTIFICATION
    {
        uint32_t code; /**< [in] one of the codes above.*/
        HWINDOW hwnd;  /**< [in] HWINDOW of the window this callback was attached to.*/
    } SCITER_CALLBACK_NOTIFICATION;

    typedef SCITER_CALLBACK_NOTIFICATION * LPSCITER_CALLBACK_NOTIFICATION;

    typedef struct SCN_LOAD_DATA
    {
        uint32_t code; /**< [in] one of the codes above.*/
        HWINDOW hwnd;  /**< [in] HWINDOW of the window this callback was attached to.*/

        const wchar_t * uri; /**< [in] Zero terminated string, fully qualified uri, for example "http://server/folder/file.ext".*/

        const uint8_t * outData; /**< [in,out] pointer to loaded data to return. if data exists in the cache then this field contain pointer to it*/
        uint32_t outDataSize;    /**< [in,out] loaded data size to return.*/
        uint32_t dataType;       /**< [in] SciterResourceType */

        void * requestId; /**< [in] request handle that can be used with sciter-x-request API */

        SCITER_ELEMENT principal;
        SCITER_ELEMENT initiator;
    } SCN_LOAD_DATA;

    typedef SCN_LOAD_DATA * LPSCN_LOAD_DATA;
    typedef int SC_CALLBACK ElementEventProc(void * tag, SCITER_ELEMENT he, uint32_t evtg, void * prms);

    /**This structure is used by #SC_ATTACH_BEHAVIOR notification.
     *\copydoc SC_ATTACH_BEHAVIOR **/
    typedef struct SCN_ATTACH_BEHAVIOR
    {
        uint32_t code; /**< [in] one of the codes above.*/
        HWINDOW hwnd;  /**< [in] HWINDOW of the window this callback was attached to.*/

        SCITER_ELEMENT element;    /**< [in] target DOM element handle*/
        const char * behaviorName; /**< [in] zero terminated string, string appears as value of CSS behavior:"???" attribute.*/

        ElementEventProc * elementProc; /**< [out] pointer to ElementEventProc function.*/
        void * elementTag;              /**< [out] tag value, passed as is into pointer ElementEventProc function.*/

    } SCN_ATTACH_BEHAVIOR;
    typedef SCN_ATTACH_BEHAVIOR * LPSCN_ATTACH_BEHAVIOR;

    typedef ElementEventProc * LPELEMENT_EVENT_PROC;

    typedef std::set<IWindowDestroySink *> WinDestroySinks;
    typedef std::set<IWindowCloseSink *> WinCloseSinks;

public:
    SciterWindow(Sciter & sciter);
    ~SciterWindow();

    void Show();
    bool Create(HWINDOW parentWinow, const char * htmlFile, int x, int y, int width, int height, unsigned int flags);
    bool GetDestroyed(void) const;
    void SetDestroyed(void);
    bool AttachHandler(SCITER_ELEMENT element, const char * riid, void * interfacePtr);
    bool DetachHandler(SCITER_ELEMENT Element, const char * riid, void * interfacePtr);

    //ISciterWindow
    void CenterWindow() override;
    void FixMinSize() override;
    HWINDOW GetHandle() const override;
    uint32_t GetMinWidth() const override;
    uint32_t GetMinHeight(uint32_t width) const override;
    SCITER_ELEMENT GetRootElement() const override;
    void OnDestroySinkAdd(IWindowDestroySink * Sink) override;
    void OnDestroySinkRemove(IWindowDestroySink * Sink) override;
    void OnCloseSinkAdd(IWindowCloseSink * Sink) override;
    void OnCloseSinkRemove(IWindowCloseSink * Sink) override;
    bool Destroy() override;
    void RunModal() override;
    bool IsClosed() const override;

    friend class Sciter;

private:
    SciterWindow(void) = delete;
    SciterWindow(const SciterWindow &) = delete;
    SciterWindow & operator=(const SciterWindow &) = delete;

    struct RegisteredSink
    {
        RegisteredSink(SCITER_ELEMENT Element_, const char * riid_, void * Interface_, std::unique_ptr<EventHandler> Sink_) :
            Element(Element_),
            Interface(Interface_),
            Sink(std::move(Sink_)),
            riid(riid_)
        {
        }
        bool operator==(const RegisteredSink & lSink)
        {
            return (Element == lSink.Element) && (Interface == lSink.Interface) && riid == lSink.riid;
        }
        SCITER_ELEMENT Element;
        void * Interface;
        std::unique_ptr<EventHandler> Sink;
        std::string riid;
    };
    typedef std::vector<RegisteredSink> EventSinks;

    void Bind();
    bool LoadHtml(const char * url);
    bool GetEventProc(const char * riid, LPELEMENT_EVENT_PROC & eventProc, uint32_t & subscription);
    void SetDefaultWindowSize(int x, int y, int width, int height);
    int64_t HandleNotification(LPSCITER_CALLBACK_NOTIFICATION pnm);
    int64_t OnLoadData(LPSCN_LOAD_DATA pnmld);
    int64_t OnAttachBehavior(LPSCN_ATTACH_BEHAVIOR pnmld);
    int64_t OnEngineDestroyed(void);
    bool QueryClose() const;

    static uint32_t SC_CALLBACK SciterCallback(LPSCITER_CALLBACK_NOTIFICATION pnm, void * param);

    Sciter & m_sciter;
    HWINDOW m_hWnd;
    HWINDOW m_hParent;
    HWINDOW m_createParent;
    int m_layoutWidth;
    int m_layoutHeight;
    EventSinks m_eventSinks;
    WinDestroySinks m_onDestroySink;
    WinCloseSinks m_onCloseSink;
    bool m_bound;
    bool m_destroyed;
};

} // namespace SciterUI

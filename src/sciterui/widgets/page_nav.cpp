#include <widgets/page_nav.h>
#include <sciter_element.h>
#include <sciter_handler.h>
#include "std_string.h"
#include <map>
#include <memory>
#include <set>

namespace SciterUI
{

class WidgetPageNav :
    public std::enable_shared_from_this<WidgetPageNav>,
    public IWidget,
    public IPageNav,
    public IClickSink,
    public ISciterElementCallback
{
    struct InsensitiveCompare
    {
        bool operator()(const std::string & a, const std::string & b) const
        {
            return _stricmp(a.c_str(), b.c_str()) < 0;
        }
    };

    struct DisplayPage
    {
    public:
        DisplayPage(SCITER_ELEMENT elem_) :
            elem(elem_),
            pageExternal(false)
        {
        }

        SciterElement elem;
        SciterElement content;
        std::string pageName;
        std::string pageContents;
        std::string pageLabel;
        bool pageExternal;

    private:
        DisplayPage() = delete;
        DisplayPage(const DisplayPage &) = delete;
        DisplayPage& operator=(const DisplayPage &) = delete;
    };
    typedef std::map<std::string, std::unique_ptr<DisplayPage>, InsensitiveCompare> NavPages;
    typedef std::map<SCITER_ELEMENT, DisplayPage *> ElemPageMap;
    typedef std::map<IWidget *, std::shared_ptr<WidgetPageNav>> PageNavs;
    typedef std::set<IPagesSink *> IPagesSinkSet;

public:
    static bool Register(ISciterUI & sciterUI);

private:
    WidgetPageNav() = delete;
    WidgetPageNav(const WidgetPageNav &) = delete;
    WidgetPageNav& operator=(const WidgetPageNav &) = delete;

    WidgetPageNav(ISciterUI & sciterUI);
    void HideCurrentPage(DisplayPage * page);
    bool ShouldChangePage(DisplayPage * newPage, DisplayPage * currentPage);
    void TryToShowPage(SCITER_ELEMENT element);
    void ShowPage(DisplayPage * page);

    //IWidget
    void Attached(SCITER_ELEMENT element, IBaseElement * baseElement) override;
    void Detached(SCITER_ELEMENT element) override;
    std::shared_ptr<void> GetInterface(const char * riid) override;

    // ISciterElementCallback
    bool OnSciterElement(SCITER_ELEMENT he) override;

    // IClickSink
    bool OnClick(SCITER_ELEMENT element, SCITER_ELEMENT sourceElem, uint32_t eventReason) override;

    // IPageNav
    std::string GetCurrentPage() override;
    bool SetCurrentPage(const char * pageName);
    void AddSink(IPagesSink * sink) override;
    void RemoveSink(IPagesSink * sink) override;

    static IWidget * sui_callback CreateWidget(ISciterUI & sciterUI);
    static void sui_callback ReleaseWidget(IWidget * widget);

    static PageNavs m_instances;

    ISciterUI & m_sciterUI;
    IBaseElement * m_baseElement;
    SciterElement m_pageNavElem;
    IPagesSinkSet m_sinks;
    SciterElement m_targetFrame;
    NavPages m_pages;
    ElemPageMap m_elemPages;
    std::string m_firstPage;
    std::string m_currentPage;
};

WidgetPageNav::PageNavs WidgetPageNav::m_instances;

WidgetPageNav::WidgetPageNav(ISciterUI & sciterUI) :
    m_sciterUI(sciterUI),
    m_baseElement(nullptr)
{
}

bool WidgetPageNav::Register(ISciterUI & sciterUI)
{
    const char * WidgetCss =
        "PageNav {"
        "    display-model: block-inside;"
        "    display: block;"
        "    behavior: PageNav;"
        "}"
        "Page {"
        "   display-model: block-inside;"
        "   display: block;"
        "}";

    return sciterUI.RegisterWidgetType("PageNav", WidgetPageNav::CreateWidget, WidgetPageNav::ReleaseWidget, WidgetCss);
}

void WidgetPageNav::Attached(SCITER_ELEMENT element, IBaseElement * baseElement)
{
    m_baseElement = baseElement;
    m_pageNavElem = element;
    SciterElement ElementRoot(m_pageNavElem.GetRoot());
    if (ElementRoot.IsValid())
    {
        m_pageNavElem.SelectElements((ISciterElementCallback *)this, "Page");
        m_targetFrame = SciterElement(ElementRoot.GetElementByID(m_pageNavElem.GetAttributeByName("target").c_str()));
        if (m_targetFrame.IsValid() && m_firstPage.length() != 0)
        {
            NavPages::iterator iter = m_pages.find(m_firstPage);
            if (iter != m_pages.end())
            {
                DisplayPage & Page = *iter->second;
                TryToShowPage(Page.elem);
            }
        }
    }
}

void WidgetPageNav::Detached(SCITER_ELEMENT /*Element*/)
{
    NavPages::const_iterator CurrentPageItr = m_pages.find(m_currentPage);
    if (CurrentPageItr != m_pages.end())
    {
        DisplayPage * page = CurrentPageItr->second.get();
        page->content = nullptr;
    }
    m_baseElement = nullptr;
    m_elemPages.clear();
    m_pages.clear();
}

std::shared_ptr<void> WidgetPageNav::GetInterface(const char * riid)
{
    if (strcmp(riid, IID_IPAGENAV) == 0)
    {
        return std::static_pointer_cast<IPageNav>(shared_from_this());
    }
    return nullptr;
}

bool WidgetPageNav::ShouldChangePage(DisplayPage * newPage, DisplayPage * currentPage)
{
    if (newPage == nullptr || currentPage == newPage)
    {
        return false;
    }

    if (currentPage)
    {
        for (IPagesSinkSet::iterator itr = m_sinks.begin(); itr != m_sinks.end(); itr++)
        {
            if (!(*itr)->PageNavChangeFrom(currentPage->pageName, m_pageNavElem))
            {
                return false;
            }
        }
    }
    for (IPagesSinkSet::iterator itr = m_sinks.begin(); itr != m_sinks.end(); itr++)
    {
        if (!(*itr)->PageNavChangeTo(newPage->pageName, m_pageNavElem))
        {
            return false;
        }
    }
    return true;
}

void WidgetPageNav::HideCurrentPage(DisplayPage * page)
{
    if (page == nullptr)
    {
        return;
    }
    if (page->content.IsValid())
    {
        page->content.Detach();
    }
    m_currentPage.clear();
    page->elem.SetState(0, SciterElement::STATE_CURRENT | SciterElement::STATE_VISITED, true);
}

void WidgetPageNav::TryToShowPage(SCITER_ELEMENT element)
{
    std::string pageId = SciterElement(element).GetAttribute("data-page_id");
    if (!pageId.empty())
    {
        element = (SCITER_ELEMENT)std::stoull(pageId.c_str(), nullptr, 16);
    }
    ElemPageMap::iterator iter = m_elemPages.find(element);
    if (iter == m_elemPages.end())
    {
        return;
    }
    DisplayPage * currentPage = nullptr;
    DisplayPage * newPage = iter->second;
    if (m_currentPage.length() > 0)
    {
        NavPages::const_iterator CurrentPageItr = m_pages.find(m_currentPage);
        if (CurrentPageItr != m_pages.end())
        {
            currentPage = CurrentPageItr->second.get();
        }
    }

    if (!ShouldChangePage(newPage, currentPage))
    {
        return;
    }
    HideCurrentPage(currentPage);
    ShowPage(newPage);
}

void WidgetPageNav::ShowPage(DisplayPage * page)
{
    if (page->content.IsValid())
    {
        m_targetFrame.Insert(page->content, m_targetFrame.GetChildCount());
        page->content.SetStyleAttribute("display", "block");
    }
    else
    {
        SciterElement & content = page->content;
        content.Create("div", "");
        m_targetFrame.Insert(content, m_targetFrame.GetChildCount());
        content.SetStyleAttribute("width", "100%%");
        content.SetStyleAttribute("height", "100%%");
        if (page->pageExternal)
        {
            m_sciterUI.SetElementHtmlFromResource(page->content, page->pageContents.c_str());
        }
        else
        {
            content.SetHTML((const uint8_t*)page->pageContents.c_str(), page->pageContents.length(), SciterElement::SIH_REPLACE_CONTENT);
        }
        m_sciterUI.UpdateWindow(m_pageNavElem.GetElementHwnd(true));
        for (IPagesSinkSet::iterator itr = m_sinks.begin(); itr != m_sinks.end(); itr++)
        {
            (*itr)->PageNavCreatedPage(page->pageName, page->content);
        }
    }
    m_currentPage = page->pageName;
    page->elem.SetState(SciterElement::STATE_CURRENT | SciterElement::STATE_VISITED, 0, true);
    for (IPagesSinkSet::iterator itr = m_sinks.begin(); itr != m_sinks.end(); itr++)
    {
        (*itr)->PageNavPageChanged(page->pageName, m_pageNavElem);
    }
}

bool WidgetPageNav::OnSciterElement(SCITER_ELEMENT element)
{
    std::unique_ptr<DisplayPage> Page(new DisplayPage(element));
    if (Page.get() == nullptr || !Page->elem.IsValid())
    {
        return false;
    }
    Page->pageName = Page->elem.GetAttributeByName("name");
    if (Page->pageName.length() == 0)
    {
        Page->pageName = stdstr_f("Page_%d", m_pages.size() + 1);
    }
    Page->pageLabel = Page->elem.GetAttributeByName("label");
    Page->pageContents = Page->elem.GetAttributeByName("contents");
    Page->pageExternal = true;
    if (Page->pageContents.length() == 0)
    {
        Page->pageExternal = false;
        Page->pageContents = Page->elem.GetHTML(false);
    }
    Page->elem.SetHTML((const uint8_t *)Page->pageLabel.c_str(), (int)Page->pageLabel.length(), SciterElement::SIH_REPLACE_CONTENT);
    Page->elem.SetAttribute("data-page_id", stdstr_f("%llx", (SCITER_ELEMENT)Page->elem).c_str());
    m_sciterUI.AttachHandler(Page->elem, IID_ICLICKSINK, (IClickSink *)this);
    std::pair<NavPages::iterator, bool> Item = m_pages.insert(NavPages::value_type(Page->pageName, std::move(Page)));
    if (Item.second)
    {
        m_elemPages.insert(ElemPageMap::value_type(element, Item.first->second.get()));
        if (m_firstPage.empty())
        {
            m_firstPage = Item.first->second->pageName;
        }
    }
    return false;
}

bool WidgetPageNav::OnClick(SCITER_ELEMENT Element, SCITER_ELEMENT /*SourceElem*/, uint32_t /*EventReason*/)
{
    TryToShowPage(Element);
    return false;
}

std::string WidgetPageNav::GetCurrentPage()
{
    return m_currentPage;
}

bool WidgetPageNav::SetCurrentPage(const char * pageName)
{
    NavPages::const_iterator iter = m_pages.find(pageName);
    if (iter == m_pages.end())
    {
        return false;
    }
    DisplayPage & Page = *iter->second;
    TryToShowPage(Page.elem);
    return true;
}

void WidgetPageNav::AddSink(IPagesSink * sink)
{
    m_sinks.insert(sink);
    for (NavPages::const_iterator itr = m_pages.begin(); itr != m_pages.end(); itr++)
    {
        const DisplayPage * page = itr->second.get();
        if (page->content.IsValid())
        {
            sink->PageNavCreatedPage(page->pageName, page->content);
        }
    }
    if (!m_currentPage.empty())
    {
        sink->PageNavPageChanged(m_currentPage, m_pageNavElem);
    }
}

void WidgetPageNav::RemoveSink(IPagesSink * Sink)
{
    IPagesSinkSet::iterator itr = m_sinks.find(Sink);
    if (itr != m_sinks.end())
    {
        m_sinks.erase(itr);
    }
}

IWidget * sui_callback WidgetPageNav::CreateWidget(ISciterUI & sciterUI)
{
    std::shared_ptr<WidgetPageNav> instance(new WidgetPageNav(sciterUI));
    IWidget* widget = (IWidget*)instance.get();
    WidgetPageNav * pagenav = instance.get();
    m_instances.insert(PageNavs::value_type(pagenav, std::move(instance)));
    return widget;
}

void sui_callback WidgetPageNav::ReleaseWidget(IWidget* widget)
{
    PageNavs::iterator it = m_instances.find(widget);
    if (it != m_instances.end())
    {
        m_instances.erase(it);
    }
}

}

bool Register_WidgetPageNav(ISciterUI& sciterUI)
{
    return SciterUI::WidgetPageNav::Register(sciterUI);
}
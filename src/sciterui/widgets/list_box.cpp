#include <widgets/list_box.h>
#include <map>
#include <sciter_element.h>
#include <sciter_handler.h>
#include "sciter_handler_internal.h"
#include <vector>

namespace SciterUI
{
    class WidgetListBox :
        public std::enable_shared_from_this<WidgetListBox>,
        public IWidget,
        public IListBox
    {
        typedef std::map<IWidget *, std::shared_ptr<WidgetListBox>> ListBoxes;
        typedef std::vector<std::string> Items;

    public:
        static bool Register(ISciterUI& SciterUI);

    private:
        WidgetListBox() = delete;
        WidgetListBox(const WidgetListBox&) = delete;
        WidgetListBox& operator=(const WidgetListBox&) = delete;

        WidgetListBox(ISciterUI& SciterUI);

        //IWidget
        void Attached(SCITER_ELEMENT element, IBaseElement* baseElement);
        void Detached(SCITER_ELEMENT element);
        std::shared_ptr<void> GetInterface(const char* riid);

        //IListBox
        int32_t AddItem(const char* item, const char * value) override;
        bool RemoveItem(int32_t index) override;
        void ClearContents() override;
        uint32_t GetCount() const override;
        int32_t CurrentIndex() const override;
        SCITER_ELEMENT GetItem(uint32_t index) const override;
        SCITER_ELEMENT GetSelectedItem() const override;
        bool SelectItem(int32_t index) override;

        static IWidget * sui_callback CreateWidget(ISciterUI & SciterUI);
        static void sui_callback ReleaseWidget(IWidget * widget);

        static ListBoxes m_instances;

        ISciterUI & m_sciterUI;
        IBaseElement * m_baseElement;
        SciterElement m_listBoxElem;
        SciterElement m_select;
        Items m_items;
    };

    WidgetListBox::ListBoxes WidgetListBox::m_instances;

    WidgetListBox::WidgetListBox(ISciterUI& sciterUI) :
        m_sciterUI(sciterUI),
        m_baseElement(nullptr)
    {
    }

    bool WidgetListBox::Register(ISciterUI& SciterUI)
    {
        const char * WidgetCss =
            "ListBox {"
            "    display-model: block-inside;"
            "    display: inline-block;"
            "    behavior: ListBox;"
            "}";

        return SciterUI.RegisterWidgetType("ListBox", WidgetListBox::CreateWidget, WidgetListBox::ReleaseWidget, WidgetCss);
    }

    void WidgetListBox::Attached(SCITER_ELEMENT element, IBaseElement* baseElement)
    {
        m_baseElement = baseElement;
        m_listBoxElem = element;

        m_select.Create("select", "");
        m_select.SetAttribute("type", "list");
        m_listBoxElem.Insert(m_select, m_listBoxElem.GetChildCount());
        m_sciterUI.AttachHandler(m_select, IID_FORWARD_BEHAVIOUR, (void *)((SCITER_ELEMENT)m_listBoxElem));
    }

    void WidgetListBox::Detached(SCITER_ELEMENT /*Element*/)
    {
        m_sciterUI.DetachHandler(m_select, IID_FORWARD_BEHAVIOUR, (void*)((SCITER_ELEMENT)m_listBoxElem));
        m_baseElement = nullptr;
        m_listBoxElem = nullptr;
    }

    std::shared_ptr<void> WidgetListBox::GetInterface(const char* riid)
    {
        if (strcmp(riid, IID_ILISTBOX) == 0)
        {
            return std::static_pointer_cast<IListBox>(shared_from_this());
        }
        return nullptr;
    }

    int32_t WidgetListBox::AddItem(const char * item, const char * value)
    {
        int32_t index = -1;
        Items::iterator itr = find(m_items.begin(), m_items.end(), item);
        if (itr != m_items.end())
        {
            return index;
        }

        index = (int32_t)m_items.size();
        m_items.push_back(item);
        SciterElement optionElement;
        optionElement.Create("option", item);
        if (value != nullptr)
        {
            optionElement.SetAttribute("value", value);
        }
        SciterElement popup = m_select.FindFirst("popup");
        if (popup.IsValid())
        {
            popup.Insert(optionElement, popup.GetChildCount());
        }
        else
        {
            m_select.Insert(optionElement, m_select.GetChildCount());
        }
        return index;
    }

    bool WidgetListBox::RemoveItem(int32_t index)
    {
        if (index < 0 || index >= (int32_t)m_items.size())
        {
            return false;
        }

        // Remove from internal vector
        m_items.erase(m_items.begin() + index);

        // Remove from DOM
        SciterElement list = m_select.FindFirst("popup");
        if (!list)
        {
            list = m_select;
        }

        SciterElement option = list.GetChild(index);
        if (option)
        {
            option.Destroy();
        }

        if (m_items.empty())
        {
            SciterElement caption = m_select.FindFirst("caption");
            if (caption)
            {
                caption.SetHTML((const uint8_t*)"&nbsp;", 6, SciterElement::SIH_REPLACE_CONTENT);
            }
        }

        return true;
    }

    void WidgetListBox::ClearContents()
    {
        SciterElement caption = m_select.FindFirst("caption");
        if (caption)
        {
            caption.SetHTML((const uint8_t *)"&nbsp;", 6, SciterElement::SIH_REPLACE_CONTENT);
        }
        SciterElement popup = m_select.FindFirst("popup");
        if (popup.IsValid())
        {
            popup.Clear();
        }
        m_items.clear();
    }

    uint32_t WidgetListBox::GetCount() const
    {
        if (!m_select)
        {
            return 0;
        }
        SciterElement list = m_select.FindFirst("popup");
        if (!list)
        {
            list = m_select;
        }
        return list.GetChildCount();
    }

    int32_t WidgetListBox::CurrentIndex() const
    {
        if (!m_select)
        {
            return -1;
        }
        SciterElement list = m_select.FindFirst("popup");
        if (!list)
        {
            list = m_select;
        }

        for (uint32_t i = 0, n = list.GetChildCount(); i < n; i++)
        {
            SciterElement option = list.GetChild(i);
            if (option && (option.GetState() & SciterElement::STATE_CURRENT) != 0)
            {
                return i;
            }
        }
        return -1;
    }

    SCITER_ELEMENT WidgetListBox::GetItem(uint32_t index) const
    {
        if (!m_select)
        {
            return nullptr;
        }
        SciterElement list = m_select.FindFirst("popup");
        if (!list)
        {
            list = m_select;
        }

        if (index >= list.GetChildCount())
        {
            return nullptr;
        }
        return list.GetChild(index);
    }

    SCITER_ELEMENT WidgetListBox::GetSelectedItem() const
    {
        if (!m_select)
        {
            return nullptr;
        }
        SciterElement list = m_select.FindFirst("popup");
        if (!list)
        {
            list = m_select;
        }

        for (uint32_t i = 0, n = list.GetChildCount(); i < n; i++)
        {
            SciterElement option = list.GetChild(i);
            if (option && (option.GetState() & SciterElement::STATE_CURRENT) != 0)
            {
                return option;
            }
        }
        return nullptr;
    }

    bool WidgetListBox::SelectItem(int32_t index)
    {
        if (!m_select || index < 0 || index >= (int32_t)m_items.size())
        {
            return false;
        }
        SciterElement list = m_select.FindFirst("popup");
        if (!list)
        {
            list = m_select;
        }
        SciterElement option = list.GetChild(index);
        if (!option)
        {
            return false;
        }
        option.SetState(SciterElement::STATE_CURRENT, 0, true);
        SciterElement caption = m_select.FindFirst("caption");
        if (caption)
        {
            caption.SetHTML((const uint8_t *)m_items[index].c_str(), m_items[index].size(), SciterElement::SIH_REPLACE_CONTENT);
        }
        return true;
    }

    IWidget * sui_callback WidgetListBox::CreateWidget(ISciterUI & sciterUI)
    {
        std::shared_ptr<WidgetListBox> instance(new WidgetListBox(sciterUI));
        IWidget * widget = (IWidget*)instance.get();
        WidgetListBox * combobox = instance.get();
        m_instances.insert(ListBoxes::value_type(combobox, std::move(instance)));
        return widget;
    }

    void WidgetListBox::ReleaseWidget(IWidget * widget)
    {
        ListBoxes::iterator it = m_instances.find(widget);
        if (it != m_instances.end())
        {
            m_instances.erase(it);
        }
    }
}

bool Register_WidgetListBox(ISciterUI & sciterUI)
{
    return SciterUI::WidgetListBox::Register(sciterUI);
}
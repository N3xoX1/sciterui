#pragma once
#include <sciter_ui.h>
#include <stdint.h>

static const char * IID_ICOMBOBOX = "2981D907-6EA1-43B4-B4FC-DBC23980A15D";

suinterface IComboBox
{
    virtual int32_t AddItem(const char * item, const char * value) = 0;
    virtual void ClearContents() = 0;
    virtual uint32_t GetCount() const = 0;
    virtual int32_t CurrentIndex() const = 0;
    virtual SCITER_ELEMENT GetSelectedItem() const = 0;
    virtual bool SelectItem(int32_t index) = 0;
};

bool Register_WidgetComboBox(ISciterUI & SciterUI);
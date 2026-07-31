#pragma once
#include <sciter_ui.h>
#include <stdint.h>

static const char * IID_ILISTBOX = "B334D118-EFBA-4200-8A95-5E2866874D4C";

suinterface IListBox
{
    virtual int32_t AddItem(const char * item, const char * value) = 0;
    virtual bool RemoveItem(int32_t index) = 0;
    virtual void ClearContents() = 0;
    virtual uint32_t GetCount() const = 0;
    virtual int32_t CurrentIndex() const = 0;
    virtual SCITER_ELEMENT GetItem(uint32_t index) const = 0;
    virtual SCITER_ELEMENT GetSelectedItem() const = 0;
    virtual bool SelectItem(int32_t index) = 0;
};

bool Register_WidgetListBox(ISciterUI & SciterUI);
#pragma once
#include <sciter_ui.h>
#include <string>

suinterface IPagesSink
{
    virtual bool PageNavChangeFrom(const std::string & pageName, SCITER_ELEMENT pageNav) = 0;
    virtual bool PageNavChangeTo(const std::string & pageName, SCITER_ELEMENT pageNav) = 0;
    virtual void PageNavCreatedPage(const std::string & pageName, SCITER_ELEMENT page) = 0;
    virtual void PageNavPageChanged(const std::string & pageName, SCITER_ELEMENT pageNav) = 0;
};

static const char * IID_IPAGENAV = "A1FD4FA4-6BEE-4166-AD9D-D7BF867B0B3E";

suinterface IPageNav
{
    virtual std::string GetCurrentPage() = 0;
    virtual bool SetCurrentPage(const char * pageName) = 0;
    virtual void AddSink(IPagesSink * sink) = 0;
    virtual void RemoveSink(IPagesSink * sink) = 0;
};

bool Register_WidgetPageNav(ISciterUI & sciterUI);
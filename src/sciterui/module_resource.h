#pragma once
#include <sciter_ui.h>
#include "std_string.h"
#include <memory>

namespace SciterUI
{
suinterface IModuleResource
{
    virtual bool LoadResource(const sui_wchar * name, const sui_wchar * type, std::unique_ptr<uint8_t> & data, uint32_t & size) = 0;
};

class ModuleResource : public IModuleResource
{
public:
    ModuleResource();
    ~ModuleResource();

    bool LoadModule(const char * path);

    // IResourceModule
    bool LoadResource(const sui_wchar * name, const sui_wchar * type, std::unique_ptr<uint8_t> & data, uint32_t & size);

private:
    ModuleResource(const ModuleResource &) = delete;
    ModuleResource & operator=(const ModuleResource &) = delete;

    void * m_module;
};

} // namespace SciterUI

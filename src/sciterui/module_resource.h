#pragma once
#include <sciter_ui.h>
#include <memory>

namespace SciterUI
{
suinterface IModuleResource
{
    virtual bool LoadResource(const wchar_t * name, const wchar_t * type, std::unique_ptr<uint8_t> & data, uint32_t & size) = 0;
};

class ModuleResource : public IModuleResource
{
public:
    ModuleResource();
    ~ModuleResource();

    bool LoadModule(const char * path);

    // IResourceModule
    bool LoadResource(const wchar_t * name, const wchar_t * type, std::unique_ptr<uint8_t> & data, uint32_t & size);

private:
    ModuleResource(const ModuleResource &) = delete;
    ModuleResource & operator=(const ModuleResource &) = delete;

    void * m_module;
};

} // namespace SciterUI
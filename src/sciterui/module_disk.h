#pragma once
#include "module_resource.h"
#include "path.h"

namespace SciterUI
{

class ModuleDisk :
    public IModuleResource
{
public:
    ModuleDisk();
    ~ModuleDisk();

    bool LoadModule(const char * path);

    // IResourceModule
    bool LoadResource(const sui_wchar * name, const sui_wchar * type, std::unique_ptr<uint8_t> & data, uint32_t & size);

private:
    ModuleDisk(const ModuleDisk &) = delete;
    ModuleDisk & operator=(const ModuleDisk &) = delete;

    Path GetResPath(const sui_wchar * name, const sui_wchar * type);
    bool m_loaded;
    Path m_path;
};

} // namespace SciterUI

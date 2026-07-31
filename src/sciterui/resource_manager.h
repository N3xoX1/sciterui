#pragma once
#include "module_disk.h"
#include "path.h"
#include <map>
#include <memory>
#include <vector>

namespace SciterUI
{

suinterface IModuleResource;

class ResourceManager
{
    typedef std::map<std::wstring, const wchar_t *> RESOURCE_MAP;
    typedef std::vector<std::unique_ptr<ModuleDisk>> ModulesDisk;
    typedef std::vector<std::unique_ptr<ModuleResource>> ModulesResource;

public:
    enum
    {
        RT_BITMAP = 2,
        RT_GROUP_CURSOR = 12,
        RT_GROUP_ICON = 14,
        RT_HTML = 23
    };

    ResourceManager(const char * languageDir);

    bool Initialize(const char * baseLanguage, const char * currentLanguage);
    bool LoadResource(const wchar_t * uri, std::unique_ptr<uint8_t> & data, uint32_t & size);

private:
    ResourceManager(void) = delete;
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager & operator=(const ResourceManager &) = delete;

    IModuleResource * LoadLanguageFile(const char * language);
    ModulesDisk m_modulesDisk;
    ModulesResource m_modulesResource;
    RESOURCE_MAP m_resourceMap;
    Path m_languageDir;
    IModuleResource * m_moduleBase;
    IModuleResource * m_moduleCurrent;
};

} // namespace SciterUI

#include "resource_manager.h"
#include <algorithm>

namespace SciterUI
{

ResourceManager::ResourceManager(const char * languageDir) :
    m_languageDir(languageDir, "")
{
    m_resourceMap.emplace(RESOURCE_MAP::value_type(SUI_WSTR("PNG"), SUI_WSTR("PNG")));
    m_resourceMap.emplace(RESOURCE_MAP::value_type(SUI_WSTR("JPG"), SUI_WSTR("JPG")));
    m_resourceMap.emplace(RESOURCE_MAP::value_type(SUI_WSTR("GIF"), SUI_WSTR("GIF")));
    m_resourceMap.emplace(RESOURCE_MAP::value_type(SUI_WSTR("SVG"), SUI_WSTR("SVG")));
    m_resourceMap.emplace(RESOURCE_MAP::value_type(SUI_WSTR("BMP"), (const sui_wchar *)RT_BITMAP));
    m_resourceMap.emplace(RESOURCE_MAP::value_type(SUI_WSTR("ICO"), (const sui_wchar *)RT_GROUP_ICON));
    m_resourceMap.emplace(RESOURCE_MAP::value_type(SUI_WSTR("CUR"), (const sui_wchar *)RT_GROUP_CURSOR));
    m_resourceMap.emplace(RESOURCE_MAP::value_type(SUI_WSTR("HTM"), (const sui_wchar *)RT_HTML));
    m_resourceMap.emplace(RESOURCE_MAP::value_type(SUI_WSTR("HTML"), (const sui_wchar *)RT_HTML));
    m_resourceMap.emplace(RESOURCE_MAP::value_type(SUI_WSTR("CSS"), SUI_WSTR("CSS")));
    m_resourceMap.emplace(RESOURCE_MAP::value_type(SUI_WSTR("INI"), SUI_WSTR("LANG")));
    m_resourceMap.emplace(RESOURCE_MAP::value_type(SUI_WSTR("LNG"), SUI_WSTR("LANG")));
}

bool ResourceManager::Initialize(const char * baseLanguage, const char * currentLanguage)
{
    if (!m_languageDir.DirectoryExists())
    {
        return false;
    }
    m_moduleBase = LoadLanguageFile(baseLanguage);
    if (m_moduleBase == nullptr)
    {
        return false;
    }
    if (_stricmp(baseLanguage, currentLanguage) != 0)
    {
        m_moduleCurrent = LoadLanguageFile(currentLanguage);
        if (m_moduleCurrent == nullptr)
        {
            return false;
        }
    }
    else
    {
        m_moduleCurrent = m_moduleBase;
    }
    return true;
}

bool ResourceManager::LoadResource(const sui_wchar * uri, std::unique_ptr<uint8_t> & data, uint32_t & size)
{
    if (m_moduleBase == nullptr || m_moduleCurrent == nullptr)
    {
        return false;
    }
    const sui_wchar * ResourceType = wcsrchr(uri, '.');
    if (ResourceType == nullptr)
    {
        return false;
    }

    ResourceType += 1;
    sui_ustring resourceTypeUpper = ResourceType;
    std::transform(resourceTypeUpper.begin(), resourceTypeUpper.end(), resourceTypeUpper.begin(), (sui_wchar(*)(int))towupper);

    RESOURCE_MAP::const_iterator iter = m_resourceMap.find(resourceTypeUpper);
    if (iter == m_resourceMap.end())
    {
        return false;
    }
    std::unique_ptr<uint8_t> ResourceData;
    uint32_t ResourceSize = 0;
    if (!m_moduleCurrent->LoadResource(uri, iter->second, ResourceData, ResourceSize))
    {
        return false;
    }
    data.reset(ResourceData.release());
    size = ResourceSize;
    return true;
}

IModuleResource * ResourceManager::LoadLanguageFile(const char * language)
{
    Path moduleFile(m_languageDir, language);
    moduleFile.SetExtension("lang");
    if (moduleFile.FileExists())
    {
        std::unique_ptr<ModuleResource> resourceModule(new ModuleResource());
        if (resourceModule.get() != nullptr)
        {
            if (!resourceModule->LoadModule(moduleFile))
            {
                return nullptr;
            }
            IModuleResource * module = resourceModule.get();
            m_modulesResource.emplace_back(std::move(resourceModule));
            return module;
        }
    }

    Path moduleDir(m_languageDir);
    moduleDir.AppendDirectory(language);
    if (moduleDir.DirectoryExists())
    {
        std::unique_ptr<ModuleDisk> diskModule(new ModuleDisk());
        if (diskModule.get() != nullptr)
        {
            if (!diskModule->LoadModule(moduleDir))
            {
                return nullptr;
            }
            IModuleResource * module = diskModule.get();
            m_modulesDisk.emplace_back(std::move(diskModule));
            return module;
        }
    }
    return nullptr;
}

} // namespace SciterUI

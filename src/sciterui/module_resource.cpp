#include "module_resource.h"
#include "path.h"
#include "std_string.h"
#include <Windows.h>

namespace SciterUI
{

ModuleResource::ModuleResource() :
    m_module(nullptr)
{
}

ModuleResource::~ModuleResource()
{
    if (m_module != nullptr)
    {
        FreeLibrary((HMODULE)m_module);
        m_module = nullptr;
    }
}

bool ModuleResource::LoadModule(const char * path)
{
    if (!Path(path).FileExists())
    {
        return false;
    }
    m_module = ::LoadLibraryEx(stdstr(path).ToUTF16().c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
    if (m_module == nullptr)
    {
        return false;
    }
    return true;
}

bool ModuleResource::LoadResource(const sui_wchar * name, const sui_wchar * type, std::unique_ptr<uint8_t> & data, uint32_t & size)
{
    if (m_module == nullptr)
    {
        return false;
    }
    sui_ustring fileName = name;
    if (sui_wcsnicmp(fileName.c_str(), SUI_WSTR("file:///"), 8) == 0)
    {
        fileName = fileName.substr(8, fileName.size() - 8);
    }
    if (sui_wcsnicmp(fileName.c_str(), SUI_WSTR("file://"), 7) == 0)
    {
        fileName = fileName.substr(7, fileName.size() - 7);
    }
    HRSRC resource = ::FindResource((HMODULE)m_module, fileName.c_str(), type);
    if (resource == nullptr)
    {
        return false;
    }
    HGLOBAL res = ::LoadResource((HMODULE)m_module, resource);
    if (res == nullptr)
    {
        return false;
    }
    uint8_t * resData = (uint8_t *)LockResource(res);
    if (resData == nullptr)
    {
        return false;
    }
    uint32_t resLen = SizeofResource((HMODULE)m_module, resource);
    data.reset(new uint8_t[resLen]);
    memcpy(data.get(), resData, resLen);
    size = resLen;
    return true;
}

} // namespace SciterUI
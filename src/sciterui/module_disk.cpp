#include "module_disk.h"
#include "file.h"
#include "resource_manager.h"
#include "std_string.h"

namespace SciterUI
{

ModuleDisk::ModuleDisk() :
    m_loaded(false)
{
}

ModuleDisk::~ModuleDisk()
{
}

bool ModuleDisk::LoadModule(const char * path)
{
    if (m_loaded)
    {
        return false;
    }
    m_path = Path(path, "");
    if (!m_path.DirectoryExists())
    {
        return false;
    }
    m_loaded = true;
    return true;
}

bool ModuleDisk::LoadResource(const sui_wchar * name, const sui_wchar * type, std::unique_ptr<uint8_t> & data, uint32_t & size)
{
    Path ResPath = GetResPath(name, type);
    if (!ResPath.FileExists())
    {
        return false;
    }

    File resFile;
    if (!resFile.Open(ResPath, File::modeRead))
    {
        return false;
    }
    uint32_t ulLen = resFile.GetLength();
    if (ulLen <= 0)
    {
        return false;
    }
    data.reset(new uint8_t[ulLen]);
    size = resFile.Read(data.get(), ulLen);
    return true;
}

Path ModuleDisk::GetResPath(const sui_wchar * name, const sui_wchar * type)
{
    Path path(m_path);

    if ((uint64_t)type <= 0xFFFF)
    {
        if (type == (const sui_wchar *)ResourceManager::RT_BITMAP || type == (const sui_wchar *)ResourceManager::RT_GROUP_ICON || type == (const sui_wchar *)ResourceManager::RT_GROUP_CURSOR)
        {
            path.AppendDirectory("image");
        }
        else if (type == (const sui_wchar *)ResourceManager::RT_HTML)
        {
            path.AppendDirectory("html");
        }
    }
    else
    {
        if (_wcsicmp(type, SUI_WSTR("png")) == 0 || _wcsicmp(type, SUI_WSTR("jpg")) == 0 || _wcsicmp(type, SUI_WSTR("gif")) == 0 || _wcsicmp(type, SUI_WSTR("svg")) == 0)
        {
            path.AppendDirectory("image");
        }
        else if (_wcsicmp(type, SUI_WSTR("css")) == 0)
        {
            path.AppendDirectory("css");
        }
        else if (_wcsicmp(type, SUI_WSTR("lang")) == 0)
        {
            path.AppendDirectory("lang");
        }
    }
    sui_ustring fileName = name;
    if (_wcsnicmp(fileName.c_str(), SUI_WSTR("file://"), 7) == 0)
    {
        fileName = fileName.substr(7, fileName.size() - 7);
    }
    path.SetNameExtension(stdstr().FromUTF16(fileName.c_str()).c_str());
    if (path.FileExists())
    {
        return path;
    }
    return Path();
}

} // namespace SciterUI

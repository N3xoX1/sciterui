#include "base_dll.h"
#include <Windows.h>
#include <iostream>
#include <map>
#include <sciterui/file.h>
#include <sciterui/path.h>
#include <sciterui/path_finder.h>
#include <sciterui/std_string.h>

using namespace SciterUI;

bool ProcessDir(const Path & targetFile, const Path & sourceDir, const char * subdir, bool verbose)
{
    typedef std::map<std::string, LPCWSTR> RESOURCE_MAP;
    RESOURCE_MAP ResourceMap;

    ResourceMap.insert(RESOURCE_MAP::value_type("PNG", L"PNG"));
    ResourceMap.insert(RESOURCE_MAP::value_type("JPG", L"JPG"));
    ResourceMap.insert(RESOURCE_MAP::value_type("GIF", L"GIF"));
    ResourceMap.insert(RESOURCE_MAP::value_type("SVG", L"SVG"));
    ResourceMap.insert(RESOURCE_MAP::value_type("BMP", RT_BITMAP));
    ResourceMap.insert(RESOURCE_MAP::value_type("ICO", RT_GROUP_ICON));
    ResourceMap.insert(RESOURCE_MAP::value_type("CUR", RT_GROUP_CURSOR));
    ResourceMap.insert(RESOURCE_MAP::value_type("HTM", RT_HTML));
    ResourceMap.insert(RESOURCE_MAP::value_type("HTML", RT_HTML));
    ResourceMap.insert(RESOURCE_MAP::value_type("CSS", L"CSS"));
    ResourceMap.insert(RESOURCE_MAP::value_type("INI", L"LANG"));
    ResourceMap.insert(RESOURCE_MAP::value_type("LNG", L"LANG"));

    HANDLE update = BeginUpdateResourceW(stdstr(targetFile).ToUTF16().c_str(), false);
    if (update == nullptr)
    {
        std::cout << "Error: failed to BeginUpdateResource on \"" << targetFile << "\n" << std::endl;
        targetFile.FileDelete();
        return false;
    }

    Path targetSearchSpec(sourceDir, "*.*"), findTarget;
    targetSearchSpec.AppendDirectory(subdir);
    uint32_t itemCount = 0;
    PathFinder targetFinder(targetSearchSpec);
    if (targetFinder.FindFirst(findTarget))
    {
        do
        {
            RESOURCE_MAP::const_iterator iter = ResourceMap.find(stdstr(findTarget.GetExtension()).ToUpper().c_str());
            if (iter == ResourceMap.end())
            {
                continue;
            }
            itemCount += 1;
            if (itemCount == 20)
            {
                itemCount = 0;
                BOOL closed = ::EndUpdateResource(update, FALSE);
                if (closed)
                {
                    update = ::BeginUpdateResource(stdstr(targetFile).ToUTF16().c_str(), FALSE);
                    if (update == nullptr)
                    {
                        std::cout << "Error: Failed to BeginUpdateResource on \"" << targetFile << "\"" << std::endl;
                        targetFile.FileDelete();
                        return false;
                    }
                }
            }

            const wchar_t * strType = iter->second;
            File fileData;
            if (verbose)
            {
                std::cout << "Processing " << (((ULONG_PTR)strType) <= 0xFFFF ? stdstr_f("%d", (WORD)(ULONG_PTR)strType).c_str() : stdstr().FromUTF16(strType).c_str()) << " - " << findTarget.GetNameExtension().c_str() << std::endl;
            }
            if (fileData.Open(findTarget, File::modeRead))
            {
                uint32_t dataSize = fileData.GetLength();
                std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(dataSize);
                fileData.Read(data.get(), dataSize);
                if (!::UpdateResource(update, strType, stdstr(findTarget.GetNameExtension()).ToUpper().ToUTF16().c_str(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPVOID)data.get(), dataSize))
                {
                    std::cout << "Error: Failed to UpdateResource on \"" << targetFile << "\"" << std::endl;
                    targetFile.FileDelete();
                    return false;
                }
            }
            else
            {
                std::cout << "Error: Failed to open \"" << findTarget << "\"" << std::endl;
                targetFile.FileDelete();
                return false;
            }
        } while (targetFinder.FindNext(findTarget));
    }
    ::EndUpdateResource(update, FALSE);
    return true;
}

bool ProcessResource(const Path & sourceDir, const Path & targetFile, bool verbose)
{
    {
        File targetResource;
        if (!targetResource.Open(targetFile, File::modeCreate | File::modeWrite))
        {
            std::cout << "Error: Failed to open \"" << targetFile << "\"" << std::endl;
            return false;
        }
        targetResource.Write(basedll, sizeof(basedll));
        targetResource.Close();
    }
    if (!ProcessDir(targetFile, sourceDir, "html", verbose))
    {
        return false;
    }
    if (!ProcessDir(targetFile, sourceDir, "image", verbose))
    {
        return false;
    }
    if (!ProcessDir(targetFile, sourceDir, "css", verbose))
    {
        return false;
    }
    return true;
}

int main(int argc, char * argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: " << Path(argv[0]).GetNameExtension().c_str() << " <source_dir> <output_file>" << std::endl;
        return 1;
    }
    Path sourceDir(argv[1], "");
    sourceDir.DirectoryNormalize(Path(Path::MODULE_DIRECTORY));
    if (!sourceDir.DirectoryExists())
    {
        std::cout << "Error: Source directory does not exist" << std::endl;
        return 1;
    }
    Path targetFile(argv[2]);
    if (!targetFile.DirectoryExists())
    {
        std::cout << "Error: Path for the file to be generated does not exist" << std::endl;
        return 1;
    }
    bool verbose = argc > 3 && _stricmp(argv[3], "-v") == 0;
    return ProcessResource(sourceDir, targetFile, verbose);
}
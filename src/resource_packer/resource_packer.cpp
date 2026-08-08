#include "base_dll.h"
#include "pe_resource_dll.h"
#include <iostream>
#include <map>
#include <sciterui/file.h>
#include <sciterui/path.h>
#include <sciterui/path_finder.h>
#include <sciterui/std_string.h>


namespace
{

// Win32 resource type IDs used by the language packs.
constexpr uint32_t RT_BITMAP = 2;
constexpr uint32_t RT_GROUP_CURSOR = 12;
constexpr uint32_t RT_GROUP_ICON = 14;
constexpr uint32_t RT_HTML = 23;
constexpr uint16_t RESOURCE_LANGUAGE = 0x400; // MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)

struct ResourceType
{
    bool isString = false;
    uint32_t id = 0;
    const char * name = nullptr;
};

bool ReadFileBytes(const SciterUI::Path & path, std::vector<uint8_t> & out)
{
    SciterUI::File file;
    if (!file.Open(path, SciterUI::File::modeRead))
    {
        return false;
    }

    const uint64_t length = file.GetLength();
    if (length > 0xffffffffull)
    {
        return false;
    }

    out.resize((size_t)length);
    if (length == 0)
    {
        return true;
    }
    return file.Read(out.data(), (uint32_t)length) == length;
}

bool CollectDir(const SciterUI::Path & sourceDir, const char * subdir, bool verbose, std::vector<PeResource> & resources)
{
    static const std::map<std::string, ResourceType> resourceMap = {
        {"PNG", {true, 0, "PNG"}},
        {"JPG", {true, 0, "JPG"}},
        {"GIF", {true, 0, "GIF"}},
        {"SVG", {true, 0, "SVG"}},
        {"BMP", {false, RT_BITMAP, nullptr}},
        {"ICO", {false, RT_GROUP_ICON, nullptr}},
        {"CUR", {false, RT_GROUP_CURSOR, nullptr}},
        {"HTM", {false, RT_HTML, nullptr}},
        {"HTML", {false, RT_HTML, nullptr}},
        {"CSS", {true, 0, "CSS"}},
        {"INI", {true, 0, "LANG"}},
        {"LNG", {true, 0, "LANG"}},
    };

    SciterUI::Path targetSearchSpec(sourceDir, "*.*");
    targetSearchSpec.AppendDirectory(subdir);

    SciterUI::Path findTarget;
    SciterUI::PathFinder targetFinder(targetSearchSpec);
    if (!targetFinder.FindFirst(findTarget))
    {
        return true;
    }

    do
    {
        const std::string extension = SciterUI::stdstr(findTarget.GetExtension()).ToUpper();
        auto iter = resourceMap.find(extension);
        if (iter == resourceMap.end())
        {
            continue;
        }

        const ResourceType & type = iter->second;
        if (verbose)
        {
            if (type.isString)
            {
                std::cout << "Processing " << type.name << " - " << findTarget.GetNameExtension() << std::endl;
            }
            else
            {
                std::cout << "Processing " << type.id << " - " << findTarget.GetNameExtension() << std::endl;
            }
        }

        PeResource resource;
        resource.typeIsString = type.isString;
        resource.typeId = type.id;
        if (type.isString)
        {
            resource.typeName = type.name;
        }
        resource.nameIsString = true;
        resource.name = SciterUI::stdstr(findTarget.GetNameExtension()).ToUpper();
        resource.language = RESOURCE_LANGUAGE;
        resource.codePage = 1252;

        if (!ReadFileBytes(findTarget, resource.data))
        {
            std::cout << "Error: Failed to open \"" << findTarget << "\"" << std::endl;
            return false;
        }
        resources.push_back(std::move(resource));
    } while (targetFinder.FindNext(findTarget));

    return true;
}

bool ProcessResource(const SciterUI::Path & sourceDir, const SciterUI::Path & targetFile, bool verbose)
{
    std::vector<PeResource> resources;
    if (!CollectDir(sourceDir, "html", verbose, resources) ||
        !CollectDir(sourceDir, "image", verbose, resources) ||
        !CollectDir(sourceDir, "css", verbose, resources))
    {
        return false;
    }

    if (resources.empty())
    {
        std::cout << "Error: No resources found in \"" << sourceDir << "\"" << std::endl;
        return false;
    }

    std::vector<uint8_t> image;
    if (!BuildPeResourceDll(basedll, sizeof(basedll), resources, image))
    {
        std::cout << "Error: Failed to build PE resource DLL" << std::endl;
        return false;
    }

    SciterUI::File targetResource;
    if (!targetResource.Open(targetFile, SciterUI::File::modeCreate | SciterUI::File::modeWrite))
    {
        std::cout << "Error: Failed to open \"" << targetFile << "\"" << std::endl;
        return false;
    }
    if (!targetResource.Write(image.data(), (uint32_t)image.size()))
    {
        std::cout << "Error: Failed to write \"" << targetFile << "\"" << std::endl;
        targetFile.FileDelete();
        return false;
    }
    return true;
}

} // namespace

int main(int argc, char * argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: " << SciterUI::Path(argv[0]).GetNameExtension().c_str() << " <source_dir> <output_file>" << std::endl;
        return 1;
    }
    SciterUI::Path sourceDir(argv[1], "");
    sourceDir.DirectoryNormalize(SciterUI::Path(SciterUI::Path::MODULE_DIRECTORY));
    if (!sourceDir.DirectoryExists())
    {
        std::cout << "Error: Source directory does not exist" << std::endl;
        return 1;
    }
    SciterUI::Path targetFile(argv[2]);
    if (!targetFile.DirectoryExists())
    {
        std::cout << "Error: Path for the file to be generated does not exist" << std::endl;
        return 1;
    }
    const bool verbose = argc > 3 && strcmp(argv[3], "-v") == 0;
    if (!ProcessResource(sourceDir, targetFile, verbose))
    {
        return 1;
    }
    return 0;
}
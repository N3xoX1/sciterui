#include "module_resource.h"
#include "file.h"
#include "path.h"
#include "std_string.h"
#include <cstring>
#include <stdint.h>
#include <vector>

namespace SciterUI
{

namespace
{

constexpr uint32_t IMAGE_RESOURCE_NAME_IS_STRING = 0x80000000u;
constexpr uint32_t IMAGE_RESOURCE_DATA_IS_DIRECTORY = 0x80000000u;
constexpr uint16_t IMAGE_NT_OPTIONAL_HDR32_MAGIC = 0x10B;
constexpr uint16_t IMAGE_NT_OPTIONAL_HDR64_MAGIC = 0x20B;

bool ReadU16(const uint8_t * data, size_t size, size_t offset, uint16_t & value)
{
    if (offset + 2 > size)
    {
        return false;
    }
    value = (uint16_t)(data[offset] | (data[offset + 1] << 8));
    return true;
}

bool ReadU32(const uint8_t * data, size_t size, size_t offset, uint32_t & value)
{
    if (offset + 4 > size)
    {
        return false;
    }
    value = (uint32_t)(data[offset] | (data[offset + 1] << 8) | (data[offset + 2] << 16) | (data[offset + 3] << 24));
    return true;
}

bool IsIntResource(const sui_wchar * value)
{
    return reinterpret_cast<uintptr_t>(value) <= 0xFFFFu;
}

} // namespace

struct ModuleResource::Impl
{
    std::vector<uint8_t> bytes;
    const uint8_t * image = nullptr;
    size_t imageSize = 0;
    uint32_t sectionTable = 0;
    uint16_t numberOfSections = 0;
    uint32_t rsrcRva = 0;
    uint32_t rsrcSize = 0;
    uint32_t rsrcFileOffset = 0;

    bool RvaToFileOffset(uint32_t rva, uint32_t & fileOffset) const;
    bool Parse();
    bool ResourceDirOffsetToFile(uint32_t offsetFromRsrc, uint32_t bytesNeeded, uint32_t & fileOffset) const;
    bool ResourceStringEquals(uint32_t stringOffset, const sui_wchar * name) const;
    bool FindDirectoryEntry(uint32_t directoryOffset, const sui_wchar * key, uint32_t & offsetToData) const;
    bool Load(const sui_wchar * name, const sui_wchar * type, const uint8_t *& data, uint32_t & size) const;
};

bool ModuleResource::Impl::RvaToFileOffset(uint32_t rva, uint32_t & fileOffset) const
{
    for (uint16_t i = 0; i < numberOfSections; ++i)
    {
        const uint32_t header = sectionTable + (uint32_t)i * 40u;
        uint32_t virtualSize = 0;
        uint32_t virtualAddress = 0;
        uint32_t sizeOfRawData = 0;
        uint32_t pointerToRawData = 0;
        if (!ReadU32(image, imageSize, header + 8, virtualSize) ||
            !ReadU32(image, imageSize, header + 12, virtualAddress) ||
            !ReadU32(image, imageSize, header + 16, sizeOfRawData) ||
            !ReadU32(image, imageSize, header + 20, pointerToRawData))
        {
            return false;
        }
        uint32_t sectionSpan = virtualSize > sizeOfRawData ? virtualSize : sizeOfRawData;
        if (sectionSpan == 0)
        {
            continue;
        }
        if (rva >= virtualAddress && (rva - virtualAddress) < sectionSpan)
        {
            const uint32_t sectionOffset = rva - virtualAddress;
            if (sectionOffset >= sizeOfRawData)
            {
                return false;
            }
            fileOffset = pointerToRawData + sectionOffset;
            return fileOffset < imageSize;
        }
    }
    return false;
}

bool ModuleResource::Impl::Parse()
{
    if (bytes.size() < 0x40 || bytes[0] != 'M' || bytes[1] != 'Z')
    {
        return false;
    }

    uint32_t eLfanew = 0;
    if (!ReadU32(bytes.data(), bytes.size(), 0x3C, eLfanew) || eLfanew + 24 > bytes.size())
    {
        return false;
    }
    if (bytes[eLfanew] != 'P' || bytes[eLfanew + 1] != 'E' || bytes[eLfanew + 2] != 0 || bytes[eLfanew + 3] != 0)
    {
        return false;
    }

    const uint32_t coff = eLfanew + 4;
    uint16_t parsedNumberOfSections = 0;
    uint16_t sizeOfOptionalHeader = 0;
    if (!ReadU16(bytes.data(), bytes.size(), coff + 2, parsedNumberOfSections) ||
        !ReadU16(bytes.data(), bytes.size(), coff + 16, sizeOfOptionalHeader) ||
        parsedNumberOfSections == 0 || sizeOfOptionalHeader < 2)
    {
        return false;
    }

    const uint32_t optionalHeader = coff + 20;
    uint16_t magic = 0;
    if (!ReadU16(bytes.data(), bytes.size(), optionalHeader, magic))
    {
        return false;
    }

    uint32_t numberOfRvaAndSizesOffset = 0;
    uint32_t dataDirectories = 0;
    if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
    {
        numberOfRvaAndSizesOffset = optionalHeader + 92;
        dataDirectories = optionalHeader + 96;
    }
    else if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    {
        numberOfRvaAndSizesOffset = optionalHeader + 108;
        dataDirectories = optionalHeader + 112;
    }
    else
    {
        return false;
    }

    uint32_t numberOfRvaAndSizes = 0;
    if (!ReadU32(bytes.data(), bytes.size(), numberOfRvaAndSizesOffset, numberOfRvaAndSizes) || numberOfRvaAndSizes < 3)
    {
        return false;
    }

    const uint32_t parsedSectionTable = optionalHeader + sizeOfOptionalHeader;
    if (parsedSectionTable + (uint32_t)parsedNumberOfSections * 40u > bytes.size())
    {
        return false;
    }

    uint32_t parsedRsrcRva = 0;
    uint32_t parsedRsrcSize = 0;
    if (!ReadU32(bytes.data(), bytes.size(), dataDirectories + 2 * 8, parsedRsrcRva) ||
        !ReadU32(bytes.data(), bytes.size(), dataDirectories + 2 * 8 + 4, parsedRsrcSize) ||
        parsedRsrcRva == 0 || parsedRsrcSize < 16)
    {
        return false;
    }

    image = bytes.data();
    imageSize = bytes.size();
    sectionTable = parsedSectionTable;
    numberOfSections = parsedNumberOfSections;
    rsrcRva = parsedRsrcRva;
    rsrcSize = parsedRsrcSize;
    return RvaToFileOffset(rsrcRva, rsrcFileOffset);
}

bool ModuleResource::Impl::ResourceDirOffsetToFile(uint32_t offsetFromRsrc, uint32_t bytesNeeded, uint32_t & fileOffset) const
{
    if (offsetFromRsrc + bytesNeeded < offsetFromRsrc || offsetFromRsrc + bytesNeeded > rsrcSize)
    {
        return false;
    }
    if (rsrcFileOffset > imageSize ||
        (size_t)rsrcFileOffset + (size_t)offsetFromRsrc + (size_t)bytesNeeded > imageSize)
    {
        return false;
    }
    fileOffset = rsrcFileOffset + offsetFromRsrc;
    return true;
}

bool ModuleResource::Impl::ResourceStringEquals(uint32_t stringOffset, const sui_wchar * name) const
{
    uint32_t lengthOffset = 0;
    if (!ResourceDirOffsetToFile(stringOffset, 2, lengthOffset))
    {
        return false;
    }
    uint16_t length = 0;
    if (!ReadU16(image, imageSize, lengthOffset, length))
    {
        return false;
    }
    uint32_t charsOffset = 0;
    if (!ResourceDirOffsetToFile(stringOffset + 2, (uint32_t)length * 2u, charsOffset))
    {
        return false;
    }
    const size_t nameLength = sui_wcslen(name);
    if (nameLength != length)
    {
        return false;
    }
    for (uint16_t i = 0; i < length; ++i)
    {
        uint16_t ch = 0;
        if (!ReadU16(image, imageSize, charsOffset + (uint32_t)i * 2u, ch))
        {
            return false;
        }
        if (sui_towupper((sui_wchar)ch) != sui_towupper(name[i]))
        {
            return false;
        }
    }
    return true;
}

bool ModuleResource::Impl::FindDirectoryEntry(uint32_t directoryOffset, const sui_wchar * key, uint32_t & offsetToData) const
{
    uint32_t headerOffset = 0;
    if (!ResourceDirOffsetToFile(directoryOffset, 16, headerOffset))
    {
        return false;
    }
    uint16_t namedCount = 0;
    uint16_t idCount = 0;
    if (!ReadU16(image, imageSize, headerOffset + 12, namedCount) ||
        !ReadU16(image, imageSize, headerOffset + 14, idCount))
    {
        return false;
    }

    const uint32_t totalEntries = (uint32_t)namedCount + (uint32_t)idCount;
    uint32_t entriesOffset = 0;
    if (!ResourceDirOffsetToFile(directoryOffset + 16, totalEntries * 8u, entriesOffset))
    {
        return false;
    }

    const bool wantId = IsIntResource(key);
    const uint32_t start = wantId ? namedCount : 0;
    const uint32_t count = wantId ? idCount : namedCount;
    const uint32_t wantedId = wantId ? (uint32_t)(uintptr_t)key : 0;

    for (uint32_t i = 0; i < count; ++i)
    {
        const uint32_t fileEntry = entriesOffset + (start + i) * 8u;
        uint32_t nameOrId = 0;
        uint32_t dataOffset = 0;
        if (!ReadU32(image, imageSize, fileEntry, nameOrId) ||
            !ReadU32(image, imageSize, fileEntry + 4, dataOffset))
        {
            return false;
        }
        if (wantId)
        {
            if ((nameOrId & IMAGE_RESOURCE_NAME_IS_STRING) != 0)
            {
                continue;
            }
            if (nameOrId == wantedId)
            {
                offsetToData = dataOffset;
                return true;
            }
        }
        else
        {
            if ((nameOrId & IMAGE_RESOURCE_NAME_IS_STRING) == 0)
            {
                continue;
            }
            if (ResourceStringEquals(nameOrId & ~IMAGE_RESOURCE_NAME_IS_STRING, key))
            {
                offsetToData = dataOffset;
                return true;
            }
        }
    }
    return false;
}

bool ModuleResource::Impl::Load(const sui_wchar * name, const sui_wchar * type, const uint8_t *& data, uint32_t & size) const
{
    if (name == nullptr || type == nullptr || image == nullptr)
    {
        return false;
    }

    uint32_t typeOffsetToData = 0;
    if (!FindDirectoryEntry(0, type, typeOffsetToData) ||
        (typeOffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY) == 0)
    {
        return false;
    }

    uint32_t nameOffsetToData = 0;
    if (!FindDirectoryEntry(typeOffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY, name, nameOffsetToData) ||
        (nameOffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY) == 0)
    {
        return false;
    }

    const uint32_t languageDirectory = nameOffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY;
    uint32_t languageHeader = 0;
    if (!ResourceDirOffsetToFile(languageDirectory, 16, languageHeader))
    {
        return false;
    }
    uint16_t namedCount = 0;
    uint16_t idCount = 0;
    if (!ReadU16(image, imageSize, languageHeader + 12, namedCount) ||
        !ReadU16(image, imageSize, languageHeader + 14, idCount))
    {
        return false;
    }
    if ((uint32_t)namedCount + (uint32_t)idCount == 0)
    {
        return false;
    }
    uint32_t firstLanguageEntry = 0;
    if (!ResourceDirOffsetToFile(languageDirectory + 16, 8, firstLanguageEntry))
    {
        return false;
    }
    uint32_t languageDataOffset = 0;
    if (!ReadU32(image, imageSize, firstLanguageEntry + 4, languageDataOffset) ||
        (languageDataOffset & IMAGE_RESOURCE_DATA_IS_DIRECTORY) != 0)
    {
        return false;
    }

    uint32_t dataEntryOffset = 0;
    if (!ResourceDirOffsetToFile(languageDataOffset, 16, dataEntryOffset))
    {
        return false;
    }
    uint32_t dataRva = 0;
    uint32_t dataSize = 0;
    if (!ReadU32(image, imageSize, dataEntryOffset, dataRva) ||
        !ReadU32(image, imageSize, dataEntryOffset + 4, dataSize))
    {
        return false;
    }

    uint32_t dataFileOffset = 0;
    if (!RvaToFileOffset(dataRva, dataFileOffset) ||
        (size_t)dataFileOffset + (size_t)dataSize > imageSize)
    {
        return false;
    }

    data = image + dataFileOffset;
    size = dataSize;
    return true;
}

ModuleResource::ModuleResource() = default;

ModuleResource::~ModuleResource() = default;

bool ModuleResource::LoadModule(const char * path)
{
    if (impl || !Path(path).FileExists())
    {
        return false;
    }

    File moduleFile;
    if (!moduleFile.Open(path, File::modeRead))
    {
        return false;
    }
    const uint64_t length = moduleFile.GetLength();
    if (length == 0 || length > UINT32_MAX)
    {
        return false;
    }

    std::unique_ptr<Impl> module(new Impl());
    module->bytes.resize((size_t)length);
    uint8_t * dest = module->bytes.data();
    uint32_t remaining = (uint32_t)length;
    while (remaining > 0)
    {
        const uint32_t got = moduleFile.Read(dest, remaining);
        if (got == 0)
        {
            return false;
        }
        dest += got;
        remaining -= got;
    }

    if (!module->Parse())
    {
        return false;
    }
    impl = std::move(module);
    return true;
}

bool ModuleResource::LoadResource(const sui_wchar * name, const sui_wchar * type, std::unique_ptr<uint8_t[]> & data, uint32_t & size)
{
    if (!impl || name == nullptr)
    {
        return false;
    }
    sui_ustring fileName = name;
    if (sui_wcsnicmp(fileName.c_str(), SUI_WSTR("file:///"), 8) == 0)
    {
        fileName = fileName.substr(8, fileName.size() - 8);
    }
    else if (sui_wcsnicmp(fileName.c_str(), SUI_WSTR("file://"), 7) == 0)
    {
        fileName = fileName.substr(7, fileName.size() - 7);
    }

    const uint8_t * resourceData = nullptr;
    uint32_t resourceSize = 0;
    if (!impl->Load(fileName.c_str(), type, resourceData, resourceSize))
    {
        return false;
    }

    data.reset(new uint8_t[resourceSize]);
    if (resourceSize > 0)
    {
        memcpy(data.get(), resourceData, resourceSize);
    }
    size = resourceSize;
    return true;
}

} // namespace SciterUI
#pragma once
#include <stdint.h>

namespace SciterUI
{

class File
{
public:
    enum OpenFlags
    {
        modeRead = 0x0000,
        modeWrite = 0x0001,
        modeReadWrite = 0x0002,
        shareCompat = 0x0000,
        shareExclusive = 0x0010,
        shareDenyWrite = 0x0020,
        shareDenyRead = 0x0030,
        shareDenyNone = 0x0040,
        modeNoInherit = 0x0080,
        modeCreate = 0x1000,
        modeNoTruncate = 0x2000,
    };

    File();
    ~File();

    bool Open(const char * fileName, uint32_t openFlags);
    bool Close();

    uint64_t GetLength() const;

    uint32_t Read(void * buffer, uint32_t bufferSize);
    bool Write(const void * buffer, uint32_t bufferSize);

private:
    File(const File &) = delete;
    File & operator=(const File &) = delete;

    void * m_file;
};

} // namespace SciterUI

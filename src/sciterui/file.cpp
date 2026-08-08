#include "file.h"
#include <cstring>

#ifdef _WIN32
#include <Windows.h>
#else
#include <cstdint>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define INVALID_HANDLE_VALUE ((void *)(intptr_t)-1)

static int HandleToFd(void * h)
{
    return (int)(intptr_t)h;
}

static void * FdToHandle(int fd)
{
    return (void *)(intptr_t)fd;
}
#endif

namespace SciterUI
{

File::File() :
    m_file(INVALID_HANDLE_VALUE)
{
}

File::~File()
{
    Close();
}

bool File::Open(const char * fileName, uint32_t openFlags)
{
    if (!Close())
    {
        return false;
    }

    if (fileName == nullptr || strlen(fileName) == 0)
    {
        return false;
    }

#ifdef _WIN32
    ULONG dwAccess = 0;
    switch (openFlags & 3)
    {
    case modeRead:
        dwAccess = GENERIC_READ;
        break;
    case modeWrite:
        dwAccess = GENERIC_WRITE;
        break;
    case modeReadWrite:
        dwAccess = GENERIC_READ | GENERIC_WRITE;
        break;
    default:
        return false;
    }

    ULONG shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    if ((openFlags & shareDenyWrite) == shareDenyWrite)
    {
        shareMode &= ~FILE_SHARE_WRITE;
    }
    if ((openFlags & shareDenyRead) == shareDenyRead)
    {
        shareMode &= ~FILE_SHARE_READ;
    }
    if ((openFlags & shareExclusive) == shareExclusive)
    {
        shareMode = 0;
    }

    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = (openFlags & modeNoInherit) == 0;

    ULONG createFlag = OPEN_EXISTING;
    if (openFlags & modeCreate)
    {
        createFlag = ((openFlags & modeNoTruncate) != 0) ? OPEN_ALWAYS : CREATE_ALWAYS;
    }

    HANDLE hFile = ::CreateFileA(fileName, dwAccess, shareMode, &sa, createFlag, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    m_file = hFile;
#else
    int posixFlags = 0;
    switch (openFlags & 3)
    {
    case modeRead:
        posixFlags = O_RDONLY;
        break;
    case modeWrite:
        posixFlags = O_WRONLY;
        break;
    case modeReadWrite:
        posixFlags = O_RDWR;
        break;
    default:
        return false;
    }

    if (openFlags & modeCreate)
    {
        posixFlags |= O_CREAT;
        if ((openFlags & modeNoTruncate) == 0)
        {
            posixFlags |= O_TRUNC;
        }
    }
    if (openFlags & modeNoInherit)
    {
        posixFlags |= O_CLOEXEC;
    }

    int fd = ::open(fileName, posixFlags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        return false;
    }

    // Best-effort advisory locking; not a full Windows share-mode equivalent.
    if ((openFlags & shareExclusive) == shareExclusive || (openFlags & shareDenyWrite) == shareDenyWrite)
    {
        struct flock fl = {};
        fl.l_type = ((openFlags & shareExclusive) == shareExclusive) ? F_WRLCK : F_RDLCK;
        fl.l_whence = SEEK_SET;
        if (::fcntl(fd, F_SETLK, &fl) == -1)
        {
            ::close(fd);
            return false;
        }
    }

    m_file = FdToHandle(fd);
#endif
    return true;
}

bool File::Close()
{
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return true;
    }

    bool success = false;
#ifdef _WIN32
    success = ::CloseHandle(m_file) != 0;
#else
    success = ::close(HandleToFd(m_file)) == 0;
#endif
    m_file = INVALID_HANDLE_VALUE;
    return success;
}

uint64_t File::GetLength() const
{
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return (uint64_t)-1;
    }

#ifdef _WIN32
    DWORD hiWord = 0;
    DWORD loWord = ::GetFileSize(m_file, &hiWord);
    if (loWord == INVALID_FILE_SIZE && ::GetLastError() != NO_ERROR)
    {
        return (uint64_t)-1;
    }
    return ((uint64_t)hiWord << 32) | (uint64_t)loWord;
#else
    struct stat st;
    if (::fstat(HandleToFd(m_file), &st) != 0)
    {
        return (uint64_t)-1;
    }
    return (uint64_t)st.st_size;
#endif
}

uint32_t File::Read(void * buffer, uint32_t bufferSize)
{
    if (bufferSize == 0 || m_file == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

#ifdef _WIN32
    DWORD read = 0;
    if (!::ReadFile(m_file, buffer, bufferSize, &read, nullptr))
    {
        return 0;
    }
    return (uint32_t)read;
#else
    ssize_t result = ::read(HandleToFd(m_file), buffer, bufferSize);
    if (result < 0)
    {
        return 0;
    }
    return (uint32_t)result;
#endif
}

bool File::Write(const void * buffer, uint32_t bufferSize)
{
    if (bufferSize == 0)
    {
        return true;
    }
    if (m_file == INVALID_HANDLE_VALUE)
    {
        return false;
    }

#ifdef _WIN32
    DWORD written = 0;
    if (!::WriteFile(m_file, buffer, bufferSize, &written, nullptr))
    {
        return false;
    }
    return written == bufferSize;
#else
    ssize_t written = ::write(HandleToFd(m_file), buffer, bufferSize);
    return written >= 0 && (uint32_t)written == bufferSize;
#endif
}

} // namespace SciterUI

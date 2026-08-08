#include "std_string.h"
#include <algorithm>
#include <cstdint>
#include <stdarg.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <stdio.h>
#endif

char sui_toupper_ascii(unsigned char c)
{
    return (c >= 'a' && c <= 'z') ? static_cast<char>(c - ('a' - 'A')) : static_cast<char>(c);
}

sui_wchar sui_towupper(sui_wchar c)
{
    return (c >= static_cast<sui_wchar>('a') && c <= static_cast<sui_wchar>('z'))
        ? static_cast<sui_wchar>(c - (static_cast<sui_wchar>('a') - static_cast<sui_wchar>('A')))
        : c;
}

int sui_stricmp(const char * a, const char * b)
{
    while (*a && *b)
    {
        const int d = sui_toupper_ascii(static_cast<unsigned char>(*a)) - sui_toupper_ascii(static_cast<unsigned char>(*b));
        if (d != 0)
        {
            return d;
        }
        ++a;
        ++b;
    }
    return sui_toupper_ascii(static_cast<unsigned char>(*a)) - sui_toupper_ascii(static_cast<unsigned char>(*b));
}

int sui_strnicmp(const char * a, const char * b, size_t count)
{
    while (count != 0 && *a && *b)
    {
        const int d = sui_toupper_ascii(static_cast<unsigned char>(*a)) - sui_toupper_ascii(static_cast<unsigned char>(*b));
        if (d != 0)
        {
            return d;
        }
        ++a;
        ++b;
        --count;
    }
    if (count == 0)
    {
        return 0;
    }
    return sui_toupper_ascii(static_cast<unsigned char>(*a)) - sui_toupper_ascii(static_cast<unsigned char>(*b));
}

int sui_wcsicmp(const sui_wchar * a, const sui_wchar * b)
{
    while (*a && *b)
    {
        const int d = static_cast<int>(sui_towupper(*a)) - static_cast<int>(sui_towupper(*b));
        if (d != 0)
        {
            return d;
        }
        ++a;
        ++b;
    }
    return static_cast<int>(sui_towupper(*a)) - static_cast<int>(sui_towupper(*b));
}

int sui_wcsnicmp(const sui_wchar * a, const sui_wchar * b, size_t count)
{
    while (count != 0 && *a && *b)
    {
        const int d = static_cast<int>(sui_towupper(*a)) - static_cast<int>(sui_towupper(*b));
        if (d != 0)
        {
            return d;
        }
        ++a;
        ++b;
        --count;
    }
    if (count == 0)
    {
        return 0;
    }
    return static_cast<int>(sui_towupper(*a)) - static_cast<int>(sui_towupper(*b));
}

const sui_wchar * sui_wcsrchr(const sui_wchar * s, sui_wchar c)
{
    const sui_wchar * last = nullptr;
    if (s == nullptr)
    {
        return nullptr;
    }
    for (; *s; ++s)
    {
        if (*s == c)
        {
            last = s;
        }
    }
    return last;
}

size_t sui_wcslen(const sui_wchar * s)
{
    size_t n = 0;
    if (s != nullptr)
    {
        while (*s++)
        {
            ++n;
        }
    }
    return n;
}

namespace SciterUI
{

stdstr::stdstr()
{
}

stdstr::stdstr(const std::string & str) :
    std::string(str)
{
}

stdstr::stdstr(const stdstr & str) :
    std::string((const std::string &)str)
{
}

stdstr::stdstr(const char * str) :
    std::string(str ? str : "")
{
}

strvector stdstr::Tokenize(char delimiter) const
{
    strvector tokens;

    stdstr::size_type lastPos = find_first_not_of(delimiter, 0);
    stdstr::size_type pos = find_first_of(delimiter, lastPos);
    while (stdstr::npos != pos)
    {
        tokens.push_back(substr(lastPos, pos - lastPos));
        lastPos = pos + 1;
        pos = find_first_of(delimiter, lastPos);
    }
    if (stdstr::npos != lastPos)
    {
        tokens.push_back(substr(lastPos));
    }
    return tokens;
}

stdstr & stdstr::ToUpper()
{
    std::transform(begin(), end(), begin(), (char (*)(int))toupper);
    return *this;
}

stdstr & stdstr::Replace(const char search, const char replace)
{
    std::string & str = *this;
    std::string::size_type pos = str.find(search);
    while (pos != std::string::npos)
    {
        str.replace(pos, 1, &replace);
        pos = str.find(search, pos + 1);
    }
    return *this;
}

stdstr & stdstr::Replace(const char * search, const char replace)
{
    std::string & str = *this;
    std::string::size_type pos = str.find(search);
    size_t SearchSize = strlen(search);
    while (pos != std::string::npos)
    {
        str.replace(pos, SearchSize, &replace);
        pos = str.find(search, pos + 1);
    }
    return *this;
}

stdstr & stdstr::Replace(const std::string & search, const std::string & replace)
{
    std::string & str = *this;
    std::string::size_type pos = str.find(search);
    size_t SearchSize = search.size();
    while (pos != std::string::npos)
    {
        str.replace(pos, SearchSize, replace);
        pos = str.find(search, pos + replace.length());
    }
    return *this;
}

#ifdef _WIN32

stdstr & stdstr::FromUTF16(const sui_wchar * utf16Source, bool * success)
{
    bool converted = false;

    if (utf16Source == nullptr)
    {
        *this = "";
        converted = true;
    }
    else if (sui_wcslen(utf16Source) > 0)
    {
        uint32_t needed = WideCharToMultiByte(CP_UTF8, 0, utf16Source, -1, nullptr, 0, nullptr, nullptr);
        if (needed > 0)
        {
            char * buf = (char *)alloca(needed + 1);
            if (buf != nullptr)
            {
                memset(buf, 0, needed + 1);

                needed = WideCharToMultiByte(CP_UTF8, 0, utf16Source, -1, buf, needed, nullptr, nullptr);
                if (needed)
                {
                    *this = buf;
                    converted = true;
                }
            }
        }
    }
    if (success)
    {
        *success = converted;
    }
    return *this;
}

sui_ustring stdstr::ToUTF16(bool * success) const
{
    bool converted = false;
    sui_ustring res;

    DWORD needed = MultiByteToWideChar(CP_UTF8, 0, this->c_str(), (int)this->length(), nullptr, 0);
    if (needed > 0)
    {
        wchar_t * buf = (wchar_t *)alloca((needed + 1) * sizeof(wchar_t));
        if (buf != nullptr)
        {
            memset(buf, 0, (needed + 1) * sizeof(wchar_t));

            needed = MultiByteToWideChar(CP_UTF8, 0, this->c_str(), (int)this->length(), buf, needed);
            if (needed)
            {
                res = buf;
                converted = true;
            }
        }
    }
    if (success)
    {
        *success = converted;
    }
    return res;
}

#else

static void append_utf8(std::string & out, uint32_t cp)
{
    if (cp <= 0x7Fu)
    {
        out.push_back(static_cast<char>(cp));
    }
    else if (cp <= 0x7FFu)
    {
        out.push_back(static_cast<char>(0xC0u | (cp >> 6)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
    }
    else if (cp <= 0xFFFFu)
    {
        out.push_back(static_cast<char>(0xE0u | (cp >> 12)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
    }
    else
    {
        out.push_back(static_cast<char>(0xF0u | (cp >> 18)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 12) & 0x3Fu)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
    }
}

static uint32_t decode_utf8_codepoint(const std::string & s, size_t & i)
{
    const unsigned char c0 = static_cast<unsigned char>(s[i]);
    if (c0 < 0x80u)
    {
        ++i;
        return c0;
    }
    if ((c0 >> 5) == 0x6u && i + 1 < s.size())
    {
        const unsigned char c1 = static_cast<unsigned char>(s[i + 1]);
        if ((c1 & 0xC0u) == 0x80u)
        {
            uint32_t cp = ((c0 & 0x1Fu) << 6) | (c1 & 0x3Fu);
            i += 2;
            return cp;
        }
    }
    if ((c0 >> 4) == 0xEu && i + 2 < s.size())
    {
        const unsigned char c1 = static_cast<unsigned char>(s[i + 1]);
        const unsigned char c2 = static_cast<unsigned char>(s[i + 2]);
        if ((c1 & 0xC0u) == 0x80u && (c2 & 0xC0u) == 0x80u)
        {
            uint32_t cp = ((c0 & 0x0Fu) << 12) | ((c1 & 0x3Fu) << 6) | (c2 & 0x3Fu);
            i += 3;
            return cp;
        }
    }
    if ((c0 >> 3) == 0x1Eu && i + 3 < s.size())
    {
        const unsigned char c1 = static_cast<unsigned char>(s[i + 1]);
        const unsigned char c2 = static_cast<unsigned char>(s[i + 2]);
        const unsigned char c3 = static_cast<unsigned char>(s[i + 3]);
        if ((c1 & 0xC0u) == 0x80u && (c2 & 0xC0u) == 0x80u && (c3 & 0xC0u) == 0x80u)
        {
            uint32_t cp = ((c0 & 0x07u) << 18) | ((c1 & 0x3Fu) << 12) | ((c2 & 0x3Fu) << 6) | (c3 & 0x3Fu);
            i += 4;
            return cp;
        }
    }

    // Invalid UTF-8 sequence.
    ++i;
    return 0xFFFDu;
}

stdstr & stdstr::FromUTF16(const sui_wchar * utf16Source, bool * success)
{
    bool converted = false;
    if (utf16Source == nullptr)
    {
        *this = "";
        converted = true;
        if (success)
        {
            *success = converted;
        }
        return *this;
    }

    std::string out;
    const sui_wchar * p = utf16Source;
    if (*p == 0)
    {
        *this = "";
        if (success)
        {
            *success = converted;
        }
        return *this;
    }

    while (*p)
    {
        uint32_t cp = static_cast<uint32_t>(*p);
        if (cp >= 0xD800u && cp <= 0xDBFFu)
        {
            const uint32_t lo = static_cast<uint32_t>(p[1]);
            if (p[1] != 0 && lo >= 0xDC00u && lo <= 0xDFFFu)
            {
                cp = 0x10000u + (((cp - 0xD800u) << 10) | (lo - 0xDC00u));
                p += 2;
            }
            else
            {
                cp = 0xFFFDu;
                ++p;
            }
        }
        else if (cp >= 0xDC00u && cp <= 0xDFFFu)
        {
            cp = 0xFFFDu;
            ++p;
        }
        else
        {
            ++p;
        }

        append_utf8(out, cp);
        converted = true;
    }

    *this = out;
    if (success)
    {
        *success = converted;
    }
    return *this;
}

sui_ustring stdstr::ToUTF16(bool * success) const
{
    bool converted = false;
    sui_ustring out;

    if (empty())
    {
        if (success)
        {
            *success = converted;
        }
        return out;
    }

    size_t i = 0;
    while (i < this->size())
    {
        const uint32_t cp = decode_utf8_codepoint(*this, i);
        if (cp <= 0xFFFFu)
        {
            out.push_back(static_cast<sui_wchar>(cp));
        }
        else
        {
            const uint32_t v = cp - 0x10000u;
            out.push_back(static_cast<sui_wchar>(0xD800u + (v >> 10)));
            out.push_back(static_cast<sui_wchar>(0xDC00u + (v & 0x3FFu)));
        }
        converted = true;
    }

    if (success)
    {
        *success = converted;
    }
    return out;
}

#endif

void stdstr::ArgFormat(const char * strFormat, va_list & args)
{
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4996)

    size_t nlen = _vscprintf(strFormat, args) + 1;
    char * buffer = (char *)alloca(nlen * sizeof(char));
    buffer[nlen - 1] = 0;
    if (buffer != nullptr)
    {
        vsprintf(buffer, strFormat, args);
        *this = buffer;
    }
#pragma warning(pop)
#else
    va_list argsCopy;
    va_copy(argsCopy, args);
    int nlen = vsnprintf(nullptr, 0, strFormat, argsCopy);
    va_end(argsCopy);
    if (nlen < 0)
    {
        *this = "";
        return;
    }
    std::string buffer(static_cast<size_t>(nlen) + 1, '\0');
    vsnprintf(buffer.data(), buffer.size(), strFormat, args);
    buffer.resize(static_cast<size_t>(nlen));
    *this = buffer;
#endif
}

stdstr_f::stdstr_f(const char * strFormat, ...)
{
    va_list args;
    va_start(args, strFormat);
    ArgFormat(strFormat, args);
    va_end(args);
}

} // namespace SciterUI

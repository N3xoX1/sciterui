#pragma once

#include <cstddef>
#include <string>
#include <vector>

#ifdef _WIN32
using sui_wchar = wchar_t;
#define SUI_WSTR(quote) L##quote
#else
using sui_wchar = char16_t;
#define SUI_WSTR(quote) u##quote
#endif
using sui_ustring = std::basic_string<sui_wchar>;

char sui_toupper_ascii(unsigned char c);
sui_wchar sui_towupper(sui_wchar c);
int sui_stricmp(const char * a, const char * b);
int sui_strnicmp(const char * a, const char * b, size_t count);
int sui_wcsicmp(const sui_wchar * a, const sui_wchar * b);
int sui_wcsnicmp(const sui_wchar * a, const sui_wchar * b, size_t count);
const sui_wchar * sui_wcsrchr(const sui_wchar * s, sui_wchar c);
size_t sui_wcslen(const sui_wchar * s);

namespace SciterUI
{

class stdstr;
typedef std::vector<stdstr> strvector;

class stdstr :
    public std::string
{
public:
    stdstr();
    stdstr(const std::string & str);
    stdstr(const stdstr & str);
    stdstr(const char * str);

    strvector Tokenize(char delimiter) const;
    stdstr & ToUpper();

    stdstr & Replace(const char search, const char replace);
    stdstr & Replace(const char * search, const char replace);
    stdstr & Replace(const std::string & search, const std::string & replace);

    stdstr & FromUTF16(const sui_wchar * utf16Source, bool * success = nullptr);
    sui_ustring ToUTF16(bool * success = nullptr) const;

    void ArgFormat(const char * format, va_list & args);
};

class stdstr_f : public stdstr
{
public:
    stdstr_f(const char * strFormat, ...);
};

} // namespace SciterUI

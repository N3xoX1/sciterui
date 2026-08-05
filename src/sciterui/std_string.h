#pragma once

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

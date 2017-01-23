#include "Globals.h"
#include <codecvt>

std::string ToUTF8(const std::wstring & str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(str);
}

std::wstring ToUTF16(const std::string & str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(str);
}

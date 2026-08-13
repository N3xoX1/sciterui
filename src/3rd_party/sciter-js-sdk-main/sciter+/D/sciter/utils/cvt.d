module sciter.utils.cvt;

import sciter.api;
import sciter.utils.cvt;
import std.string;
import std.utf : toUTF8;
import std.exception : enforce;

public string charsOf(LPCWSTR w) {
  return toUTF8(fromStringz!wchar(w));
}

public string charsOf(LPCSTR pc) {
  return fromStringz!char(pc).idup;
}


/// Converts a `wstring` to a null-terminated UTF-16 string.
/// Returns a pointer to the first element.
/// The returned pointer remains valid as long as the returned `wstring` is kept alive.
const(wchar)* toWstringz(wstring s)
{
    // Append null terminator if missing
    if (s.length == 0 || s[$ - 1] != '\0')
        s ~= '\0';
    return s.ptr;
}



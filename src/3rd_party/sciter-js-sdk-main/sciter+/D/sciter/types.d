module sciter.types;

alias INT_PTR = ptrdiff_t;
alias UINT_PTR = size_t;
alias INT = int;
alias UINT = uint;
alias LPUINT = UINT*;
alias UINT64 = ulong;
alias SBOOL = int;
alias LPCBYTE = const(ubyte)*;
alias LPCWSTR = const(wchar)*;
alias LPWSTR = wchar*;
alias LPCSTR = const(char)*;
alias VOID = void;
alias LPVOID = void*;
struct RECT { int left; int top; int right; int bottom; 
              int width() const { return right - left;} 
              int height() const { return bottom - top;} }
alias LPRECT = RECT*;
struct POINT { int x; int y; }
struct SIZE  { int w; int h; }
alias LPPOINT = POINT*;
alias LPSIZE = SIZE*;
alias ATOM = ulong;
alias HWINDOW = const(void)*;
alias HREQUEST = const(void)*;
alias HARCHIVE = const(void)*; 
alias bytes = const(ubyte)[];

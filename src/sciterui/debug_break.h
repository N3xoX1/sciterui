#pragma once

namespace SciterUI
{

inline void DebugBreak()
{
#ifdef _WIN32
    __debugbreak();
#else
    __builtin_trap();
#endif
}

} // namespace SciterUI

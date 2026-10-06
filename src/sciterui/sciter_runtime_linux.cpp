#include <sciter-x-api.h>
#include <dlfcn.h>
#include <unistd.h>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

extern "C" ISciterAPI * getSciterAPI()
{
    static ISciterAPI * api = [] {
        int flags = RTLD_LOCAL | RTLD_LAZY;
#ifdef RTLD_DEEPBIND
        // Sciter exports its own libjpeg ABI 62. GTK may have already loaded
        // libjpeg ABI 80; interposing those symbols aborts on JPEG decoding.
        flags |= RTLD_DEEPBIND;
#endif
        std::array<char, 4096> executable{};
        const auto size = readlink("/proc/self/exe", executable.data(), executable.size() - 1);
        void * library = nullptr;
        if (size > 0)
        {
            executable[static_cast<size_t>(size)] = '\0';
            const auto path = std::filesystem::path(executable.data()).parent_path() / SCITER_DLL_NAME;
            library = dlopen(path.c_str(), flags);
        }
        if (!library)
            library = dlopen(SCITER_DLL_NAME, flags);
        if (!library)
        {
            std::fprintf(stderr, "SciterUI: cannot load Sciter: %s\n", dlerror());
            std::exit(EXIT_FAILURE);
        }
        const auto entry = reinterpret_cast<ISciterAPI * (*)()>(dlsym(library, "SciterAPI"));
        if (!entry)
        {
            std::fprintf(stderr, "SciterUI: SciterAPI unavailable: %s\n", dlerror());
            std::exit(EXIT_FAILURE);
        }
        // The API table and native windows require the runtime for process life.
        return entry();
    }();
    return api;
}

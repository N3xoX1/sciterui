# Regression tests

Build and run the callback lifetime tests:

```sh
cmake -S . -B build -DSCITERUI_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

On macOS this also runs the native window suite using the bundled Sciter
library. It requires a graphical user session. It checks the runtime Cocoa
handle type, 32 explicit/native close cycles, DOM replacement, close vetoes,
recursive destroy, timer-driven modal close, failed creation, SIGTERM, native
Quit, retained content views, and Quit with a retained or expired main wrapper.

The callback suite uses a minimal API table and drops the registration's last
external owner inside each sink. It checks that all 10 trampolines keep the
handler alive through the sink call and release it when the call returns.

To instrument the library and tests with Clang AddressSanitizer and UBSan:

```sh
cmake -S . -B build-asan -DSCITERUI_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_OBJCXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build build-asan
ASAN_OPTIONS=malloc_fill_byte=0 ctest --test-dir build-asan --output-on-failure
```

The bundled Sciter binary is precompiled and is not instrumented by these flags.
Its native window creation fails under ASan's default allocation fill (`0xbe`),
before the wrapper's close code runs. The native suite uses zero allocation fill
to work around this SDK limitation; address and undefined behavior checks remain
enabled for the wrapper and tests. The callback suite also passes with ASan's
default settings.

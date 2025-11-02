# chipate

[![CI](https://github.com/kryksyh/chipate/actions/workflows/ci.yml/badge.svg)](https://github.com/kryksyh/chipate/actions/workflows/ci.yml)
[![License: WTFPL](https://img.shields.io/badge/License-WTFPL-brightgreen.svg)](http://www.wtfpl.net/about/)

CHIP-8 emulator and assembler written in C++.

[Try it in your browser](https://kryksyh.github.io/chipate/)

## Building

Requires CMake and C++20 compatible compiler. Dependencies are fetched automatically.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/chipate [rom_file]
```

### WebAssembly

```bash
emcmake cmake -B build -DCMAKE_BUILD_TYPE=Release -DPLATFORM=Web
cmake --build build
python3 -m http.server -d build 8000
```

## Testing

```bash
ctest --test-dir build --output-on-failure
```

## License

See LICENSE file.

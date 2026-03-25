# Razumov Vocal Chain

VST3 / AU / Standalone: вокальный цепной плагин с графом узлов (дорожная карта в `docs/ROADMAP.md`).

## Требования

- CMake **3.22+**
- Компилятор с **C++17** (Xcode 14+ на macOS, VS 2022 на Windows)
- **macOS 11+** (deployment target задан в `CMakeLists.txt`)

## Сборка (macOS)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Тесты (консоль)

```bash
cmake --build build --target RazumovVocalChainTests
./build/RazumovVocalChainTests
```

Артефакты JUCE обычно копируются после сборки в системные папки плагинов (см. вывод CMake `Installing:`).

- **VST3:** `~/Library/Audio/Plug-Ins/VST3/Razumov Vocal Chain.vst3`
- **AU:** `~/Library/Audio/Plug-Ins/Components/Razumov Vocal Chain.component`
- **Standalone:** `build/RazumovVocalChain_artefacts/Release/Standalone/Razumov Vocal Chain.app` (путь может отличаться в зависимости от генератора)

## Сборка (Windows)

```text
cmake -S . -B build
cmake --build build --config Release
```

Используйте **x64** Native Tools / Visual Studio 2022.

## JUCE

Версия фиксируется в `CMakeLists.txt` через `FetchContent` (тег `8.0.12`). Первый `cmake` скачает JUCE в кэш сборки.

## Документация для агентов

См. `AGENTS.md` и `.cursor/rules/`.

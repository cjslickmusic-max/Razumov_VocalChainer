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

**Где лежит плагин после сборки (один канонический путь в репозитории):** каталог  
`build/RazumovVocalChain_artefacts/Release/` — отсюда можно указать путь к VST3 в хосте (REAPER и т.д.), без копирования.

- **VST3:** `build/RazumovVocalChain_artefacts/Release/VST3/Razumov Vocal Chain.vst3`
- **AU:** `build/RazumovVocalChain_artefacts/Release/AU/Razumov Vocal Chain.component`
- **Standalone:** `build/RazumovVocalChain_artefacts/Release/Standalone/Razumov Vocal Chain.app`

Если **`lib..._SharedCode.a` свежее, а `Razumov Vocal Chain.vst3` со старой датой** — инкрементальная сборка не перелинковала бандл. Сделайте полную пересборку цели:  
`cmake --build build --parallel --target RazumovVocalChain` (или удалите каталог `build/RazumovVocalChain_artefacts` и соберите снова).

Дополнительно JUCE при `COPY_PLUGIN_AFTER_BUILD` дублирует в пользовательские Plug-Ins (удобно для хостов, которые смотрят только туда):

- `~/Library/Audio/Plug-Ins/VST3/Razumov Vocal Chain.vst3`
- `~/Library/Audio/Plug-Ins/Components/Razumov Vocal Chain.component`

## Сборка (Windows)

```text
cmake -S . -B build
cmake --build build --config Release
```

Используйте **x64** Native Tools / Visual Studio 2022. Подробности и генераторы: `docs/BUILD_WINDOWS.md`.

## JUCE

Версия фиксируется в `CMakeLists.txt` через `FetchContent` (тег `8.0.12`). Первый `cmake` скачает JUCE в кэш сборки.

## Документация для агентов

См. `AGENTS.md` и `.cursor/rules/`.

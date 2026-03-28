# Razumov Vocal Chain

VST3 / AU / Standalone: вокальный цепной плагин с графом узлов (дорожная карта в `docs/ROADMAP.md`).

## Требования

- CMake **3.22+**
- Компилятор с **C++17** (Xcode 14+ на macOS, VS 2022 на Windows)
- **macOS 11+** (deployment target задан в `CMakeLists.txt`)

## Сборка (macOS)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel --target RazumovVocalChain_VST3
```

Сборка только `RazumovVocalChain` (shared code) **не** обновляет VST3 в `Artifacts/` — для проверки в REAPER используй цель **`RazumovVocalChain_VST3`** (см. выше).

### Тесты (консоль)

```bash
cmake --build build --target RazumovVocalChainTests
./build/RazumovVocalChainTests
```

**VST3 для REAPER (один фиксированный путь в проекте):** после `cmake --build` бандл **автоматически** копируется в:

- **`Artifacts/Razumov Vocal Chain.vst3`**

Укажи в REAPER папку **`Artifacts`** как дополнительный каталог VST3 и не меняй путь — после каждой сборки файл там обновляется (ручное копирование не нужно).

Исходный вывод CMake (то же содержимое):

- **VST3:** `build/RazumovVocalChain_artefacts/Release/VST3/Razumov Vocal Chain.vst3`
- **AU:** `build/RazumovVocalChain_artefacts/Release/AU/Razumov Vocal Chain.component`
- **Standalone:** `build/RazumovVocalChain_artefacts/Release/Standalone/Razumov Vocal Chain.app`

Если **`lib..._SharedCode.a` свежее, а `Razumov Vocal Chain.vst3` со старой датой** — инкрементальная сборка не перелинковала бандл. Сделайте полную пересборку цели:  
`cmake --build build --parallel --target RazumovVocalChain` или явно `RazumovVocalChain_VST3` (или удалите каталог `build/RazumovVocalChain_artefacts` и соберите снова).

**Системные Plug-Ins (macOS):** в проекте **`COPY_PLUGIN_AFTER_BUILD` выключен** — сборка **не** копирует VST3/AU в `~/Library/Audio/Plug-Ins/...`, чтобы в REAPER не появлялись **две копии** одного плагина (отдельно укажи только папку **`Artifacts`** в хосте). Если раньше JUCE уже копировал плагин в `~/Library/...`, удалите оттуда `Razumov Vocal Chain.vst3` / `.component`, чтобы не дублировать с `Artifacts/`.

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

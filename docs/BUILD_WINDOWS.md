# Сборка на Windows

Проект: **CMake 3.22+**, **JUCE 8** через `FetchContent` (как на macOS). Нужны компилятор **MSVC** и **Git** в `PATH`.

## Visual Studio 2022 (рекомендуется)

В **x64 Native Tools** или из PowerShell в корне репозитория:

```bat
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

Артефакты: подкаталог `RazumovVocalChain_artefacts\Release\` с VST3 / Standalone (форматы из `CMakeLists.txt`).

## Ninja (опционально)

```bat
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Замечания

- Минимальная версия Windows задаётся политикой JUCE / toolchain; при ошибках SDK установите **Desktop development with C++** в Visual Studio Installer.
- AU — только macOS; на Windows собираются цели, объявленные в `juce_add_plugin` (например VST3, Standalone).

# VST3 (canonical path)

После **любой** успешной сборки цели `RazumovVocalChain` CMake **автоматически** обновляет бандл по фиксированному пути:

**`<корень репозитория>/Artifacts/Razumov Vocal Chain.vst3`**

В REAPER укажи **один раз** папку **`Artifacts`** как дополнительный каталог VST3 и больше её не меняй: после каждой пересборки плагин там же (имя файла не меняется).

Исходная сборка по-прежнему в каталоге CMake:

`build/RazumovVocalChain_artefacts/Release/VST3/Razumov Vocal Chain.vst3`

Ручное `cp` не нужно (копирование делает `POST_BUILD` в `CMakeLists.txt`).

Папка `Artifacts/*.vst3` в `.gitignore` — бинарники в git не коммитим.

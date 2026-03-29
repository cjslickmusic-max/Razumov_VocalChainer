# VST3 (canonical path)

После **любой** успешной сборки цели `RazumovVocalChain` CMake **автоматически** обновляет бандл по фиксированному пути:

**`<корень репозитория>/Artifacts/Razumov VocalChainer.vst3`**

В REAPER укажи **один раз** папку **`Artifacts`** как дополнительный каталог VST3 и больше её не меняй: после каждой пересборки плагин там же (имя файла не меняется).

Исходная сборка по-прежнему в каталоге CMake:

`build/RazumovVocalChain_artefacts/Release/VST3/Razumov VocalChainer.vst3`

Ручное `cp` не нужно (копирование делает `POST_BUILD` в `CMakeLists.txt`).

Если после переименования продукта остался старый бандл **`Razumov Vocal Chain.vst3`** в этой папке — удали его вручную, чтобы в хосте не было двух имён.

Сборка **не** кладёт плагин в `~/Library/Audio/Plug-Ins/` (`COPY_PLUGIN_AFTER_BUILD FALSE`), чтобы в REAPER не было двух копий. Старый файл в Library, если был от прошлых сборок, удали вручную один раз.

Папка `Artifacts/*.vst3` в `.gitignore` — бинарники в git не коммитим.

# Сборка VST3 для проверки

Сюда копируется готовый **VST3** из каталога сборки CMake (после `cmake --build build --config Release`):

`build/RazumovVocalChain_artefacts/Release/VST3/Razumov Vocal Chain.vst3`

Папка `Artifacts/` в `.gitignore` — бинарники в git не кладём; обновляйте копию локально после сборки.

**Пример обновления:**

```bash
cp -R "build/RazumovVocalChain_artefacts/Release/VST3/Razumov Vocal Chain.vst3" Artifacts/
```

Установка в DAW: укажите путь к `Artifacts/` как дополнительную папку плагинов или скопируйте `.vst3` в `~/Library/Audio/Plug-Ins/VST3/` (macOS).

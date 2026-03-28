# Тесты Razumov Vocal Chain

## Сборка и запуск

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target RazumovVocalChainTests
./build/RazumovVocalChainTests
```

`ctest` (если включён `enable_testing`): тест `GraphEngine` вызывает тот же бинарник.

## Структура

| Файл | Назначение |
|------|------------|
| `Tests/TestMain.cpp` | `main`: `runGraphEngineTests()`, `runMergePdcTests()`, `runDspDeterminismTests()`. |
| `Tests/GraphTests.cpp` | Граф, merge, PDC, импульсы, вложенные split. |
| `Tests/MergePdcTests.cpp` | 2/3-way merge + вложенный split: импульс, синус = чистая задержка после PDC, DC. |
| `Tests/DspDeterminismTests.cpp` | Детерминизм узлов (в т.ч. Spectral), тишина на компрессорах, задержка и фаза синуса. |
| `Tests/DspTestHelpers.h` | Общие хелперы: `nearAbs`, `fillSine`, `assertBuffersNearEqual`, `copyBuffer`. |

## Политика проверок

1. **Детерминизм:** один и тот же буфер, `reset()`, повторный `process` — выход совпадает с первым прогоном (с допуском `float`).
2. **Тишина / ноль:** нулевой вход в компрессоры — выход около нуля (нет взвода шума).
3. **Задержка и фаза:** для `LatencyNode` проверяется соответствие `out[i] == in[i-D]` на синусоиде (сохранение фазовой связи как сдвиг времени).
4. **DC через Gain:** постоянное смещение масштабируется предсказуемо (линейный gain).

## Расширение

- Новые сьюты: отдельный `.cpp` + функция `runFooTests()` + вызов из `TestMain.cpp`.
- Спектральный узел, полный плагин через `AudioProcessor`: отдельные цели/хелперы по мере необходимости (сейчас покрыты узлы из консольного таргета).

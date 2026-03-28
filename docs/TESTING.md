# Тесты Razumov Vocal Chain

## Сборка и запуск

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target RazumovVocalChainTests
./build/RazumovVocalChainTests
```

Долгий цикл (30–40 мин): `scripts/regression_loop_until_duration.sh` (переменная `DURATION_SEC`, по умолчанию 2400 с) — повторяет **unit + stress** до истечения времени.

**Долгий stress-таргет** (миллионы сэмплов, типично **~10-20 с** wall на современном CPU; константы в `Tests/GraphStressTests.cpp`):

```bash
cmake --build build --target RazumovVocalChainStressTests
./build/RazumovVocalChainStressTests
```

`ctest`: быстрый `GraphEngine` (`RazumovVocalChainTests`) и помеченный `long`/`stress` тест `GraphStress` (`RazumovVocalChainStressTests`). Только быстрые тесты: `ctest -LE stress` или исключите по метке.

## Структура

| Файл | Назначение |
|------|------------|
| `Tests/TestMain.cpp` | `main`: `runGraphEngineTests()`, `runMergePdcTests()`, `runFlexGraphSerializationTests()`, `runPhaseAlignTests()`, `runDspDeterminismTests()`. |
| `Tests/GraphTests.cpp` | Граф, merge, PDC, импульсы, вложенные split. |
| `Tests/MergePdcTests.cpp` | 2/3-way merge + вложенный split: импульс, синус = чистая задержка после PDC, DC. |
| `Tests/PhaseAlignTests.cpp` | Этап G: `branchPhaseAlignSamples` на split — импульс, latency, синус = чистая задержка. |
| `Tests/FlexGraphSerializationTests.cpp` | ValueTree round-trip, `assignUniqueSlotIds`, пустой сегмент. |
| `Tests/DspDeterminismTests.cpp` | Детерминизм узлов (в т.ч. Spectral), тишина на компрессорах, задержка и фаза синуса. |
| `Tests/GraphStressTests.cpp` + `StressMain.cpp` | Отдельный бинарник: длинные прогоны merge/PDC, смена планов, Spectral, цепочка phase3, NaN/пик. |
| `Tests/DspTestHelpers.h` | Общие хелперы: `nearAbs`, `fillSine`, `assertBuffersNearEqual`, `copyBuffer`, `buffersExactlyEqual`. |

## Политика проверок

1. **Детерминизм:** один и тот же буфер, `reset()`, повторный `process` — выход совпадает с первым прогоном (с допуском `float`).
2. **Тишина / ноль:** нулевой вход в компрессоры — выход около нуля (нет взвода шума).
3. **Задержка и фаза:** для `LatencyNode` проверяется соответствие `out[i] == in[i-D]` на синусоиде (сохранение фазовой связи как сдвиг времени).
4. **DC через Gain:** постоянное смещение масштабируется предсказуемо (линейный gain).

## Расширение

- Новые сьюты: отдельный `.cpp` + функция `runFooTests()` + вызов из `TestMain.cpp`.
- Спектральный узел, полный плагин через `AudioProcessor`: отдельные цели/хелперы по мере необходимости (сейчас покрыты узлы из консольного таргета).

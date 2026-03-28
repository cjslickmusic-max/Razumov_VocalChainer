# Refactor / suspicious log

| Дата | Версия | Файл | Место | Суть | P | Verified |
|------|--------|------|-------|------|---|----------|
| 2026-03-28 | 0.9.1 | `GraphEngine.cpp` | `swapAndPreparePendingPlan`, mutex | `std::mutex` на каждом аудиоблоке при swap плана; риск задержек при конкуренции с UI — см. `docs/CODE_REVIEW_CRITIQUE.md` | M | |
| 2026-03-28 | 0.9.1 | `SpectralCompressorNode.cpp` | `process`, `dryScratch_.setSize` | При смене `numSamples` без повторного `prepare` возможна реаллокация scratch | M | |
| 2026-03-28 | 0.9.1 | `ModuleParamsRuntime.cpp` | `findSlotIndex` | Линейный поиск слота при `fillSlot`; при очень большом числе слотов — map/индекс | L | |
| 2026-03-28 | 0.9.1 | `GraphEngine.cpp` | `walkApplyPhase3*` | Полный обход дерева узлов каждый блок для apply параметров — O(N) | L | |

(P — приоритет H/M/L; Verified — кем проверено / дата.)

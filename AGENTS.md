# Razumov Vocal Chain — гид для агентов Cursor

**Читать в начале нетривиальной задачи** (DSP, граф узлов, UI, сборка, пресеты).

## Язык

- Ответы пользователю — **только на русском** (см. `.cursor/rules/answer-in-russian.mdc`). Код и идентификаторы — на английском.

## Продукт (кратко)

- VST-плагин **macOS + Windows** (JUCE и кроссплатформенные практики).
- **Граф узлов**: перестановка, неограниченное число узлов (в разумных пределах), **параллельные ветки** с суммированием.
- **Фаза / время:** при merge параллельных путей — **компенсация задержки**; спектральные и lookahead-модули учитывать явно.
- **Разные типы компрессоров** — отдельные узлы/модели (opto, FET, VCA…), не один «универсальный» морфинг, если не решено иначе; общие DSP-примитивы — в библиотеке.
- **Приоритет разработки:** цепочка, изменение графа, макросы, качество компрессоров/динамики, параллели и **фаза между ветками**; **коррекция микрофона по АЧХ/замерам и коррекция комнаты — в самом конце** (см. `docs/PRODUCT_ARCHITECTURE_ROADMAP.md`).

## Правила в репозитории (`.cursor/rules/`)

| Файл | Суть |
|------|------|
| `answer-in-russian.mdc` | Ответы на русском |
| `cross-platform-only.mdc` | Без Win-only / Mac-only в общем коде |
| `dsp-audio-safety.mdc` | Нет аллокаций/блокировок в audio thread; границы буферов |
| `vocal-chain-graph-dsp.mdc` | Граф, latency, merge, фаза, отдельные компрессоры |
| `node-module-conventions.mdc` | Узлы: prepare/process/latency, сериализация |
| `preserve-working-code.mdc` | Точечные правки, не ломать bypass/merge |
| `document-knowledge.mdc` | Куда писать `docs/` |
| `multi-message-task-continuity.mdc` | Не сбрасывать план из-за нового сообщения |
| `code-health-check.mdc` | Scoped/Full аудит → `docs/CODE_HEALTH_AUDIT.md` |
| `performance-checkpoints.mdc` | Замеры → `docs/PERF_CHECKPOINTS.md` |
| `git-auto-checkpoint.mdc` | Локальные WIP-коммиты, если есть `.git` |
| `agent-build-after-changes.mdc` | После правок кода — агент сам запускает сборку (`cmake --build`), пользователь не обязан компилировать вручную |
| `ui-strings-ascii.mdc` | Строки UI — ASCII-first |
| `refactor-suspicious-log.mdc` | Сомнения → `docs/REFACTOR_SUSPICIOUS_LOG.md` |

## Документы (по мере появления кода)

- **`docs/ROADMAP.md`** — пошаговая дорожная карта; **следовать порядку фаз**.
- `docs/ARCHITECTURE.md` — граф, поток аудио, пресеты (после появления кода).
- `docs/CODE_HEALTH_AUDIT.md` — журнал аудитов.
- `docs/PERF_CHECKPOINTS.md` — замеры CPU.
- `docs/REFACTOR_SUSPICIOUS_LOG.md` — отложенные риски.

## Отличия от Razumov Ultimate Sampler

Исходный пак правил: `CursorRules_RazumovSampler_SourcePack`. Оттуда перенесены **переносимые** идеи; **выкинуты** привязки к сэмплеру (9 каналов микшера, SawOsc, скины, Home VFX, Serum и т.д.). Для Vocal Chain добавлены правила **графа, latency и типов компрессоров**.

## Сборка и артефакты

**macOS (Release):**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

JUCE **8.0.12** подтягивается через `FetchContent` при первом `cmake`. Подробности и пути к VST3/AU — в **`README.md`**.

**Цель CMake:** `RazumovVocalChain` (объединённый таргет плагина + Standalone).

**Версия:** `project(... VERSION x.y.z)` и `VERSION` внутри `juce_add_plugin` — держать синхронно в `CMakeLists.txt` (сейчас **0.9.3**: цепочка в UI по дереву графа + fork/merge; phase align на split-ветках + PDC; макросы x8; per-slot модули + chain/mic profile).

**Тесты:** `cmake --build build --target RazumovVocalChainTests` затем `./build/RazumovVocalChainTests`; долгий прогон: таргет `RazumovVocalChainStressTests` или `ctest` (см. **`docs/TESTING.md`**). **Сборка после правок кода** — см. `.cursor/rules/agent-build-after-changes.mdc` (агент запускает сам).

# Razumov VocalChainer — design system

Документ для людей и агентов: **принципы**, **токены**, **связь веб/HTML и JUCE**, **правила изменений**.

## 1. Зачем токены

**Design tokens** — именованные значения (цвет, радиус, отступ, шрифт, тень), а не «магические» hex в коде. Так:

- один источник правды для лица продукта;
- проще согласовать HTML-макет и нативный UI;
- другой агент видит семантику (`accent.signal` = провода цепочки), а не сырой `#6c9fd2`.

Формат JSON взят с ориентиром на [Design Tokens Community Group](https://design-tokens.github.io/community-group/format/) (поля `$type`, `$value`, `$description`).

## 2. Файлы (канон и производные)

| Файл | Роль |
|------|------|
| `resources/design/tokens/tokens.json` | **Канон**: описание токенов, комментарии, типы. Менять **сначала** здесь. |
| `resources/design/tokens/tokens.css` | CSS-переменные для прототипов (`:root`). Держать в **соответствии** с JSON. |
| `Source/ui/DesignTokens.h` | **JUCE**: `0xAARRGGBB` в `tokens::argb` (фон, текст, бордеры, ротори), `tokens::macro` (восемь макросов + секция), `tokens::knob` (акценты групп модулей). Синхронизировать с JSON при смене палитры. |
| `resources/design/chain-graph-prototype.html` | Пример использования `tokens.css` (цепочка, glass, адаптив). |

Прозрачности и градиенты, которые нужны только в вебе, живут в `tokens.css` и в `tokens.json` (где уместно); в `DesignTokens.h` — только **opaque** цвета для `juce::Colour(uint32_t)`.

## 3. Принципы продукта (UX + лицо)

1. **Тёмная студийная база** — низкая усталость глаз, контраст текста и контролов как в DAW (см. `.cursor/rules/ui-strings-ascii.mdc` для строк).
2. **Один холодный акцент** (`accent.signal`) — провода, обводка узлов, «технический» слой. **Тёплый акцент** (`accent.cta`) — акценты продукта, параллель, важные бейджи; не смешивать с десятком других цветов без причины.
3. **Выделение слота** — `accent.selection` + мягкое свечение (см. `drawRoundedRectGlow` в `EditorVisualAssets`), не резкие кислотные обводки.
4. **Parallel / merge** — отдельный семантический акцент `accent.merge`, чтобы маршрут читался с первого взгляда (см. легенду в `chain-graph-prototype.html`).
5. **Liquid glass (направление)** — в вебе: `backdrop-filter`, полупрозрачность, шум; в JUCE: слоистые заливки и лёгкий blur **не на audio thread** (см. `.cursor/skills/liquid-glass-design/FOR_NATIVE_PLUGIN_UI.md`).
6. **Адаптив** — в прототипах: горизонтальный скролл цепочки, на узких экранах — колонка; в плагине: `setResizeLimits` и разумные минимальные размеры окна.

## 4. Семантика слоёв (имена токенов)

| Токен (логическое имя) | Назначение |
|------------------------|------------|
| `color.background.editor` | Фон всего окна редактора. |
| `color.background.chainStrip` | Подложка полосы «Signal path». |
| `color.background.node` | Карточка узла по умолчанию. |
| `color.background.mergeNode` | Узел слияния параллельных веток. |
| `color.text.primary` / `secondary` / `label` | Иерархия текста. |
| `color.accent.signal` | Провода и дефолтная обводка карточки. |
| `color.accent.selection` | Выбранный слот. |
| `color.accent.cta` | Продуктовый акцент (оранжевый). |
| `color.accent.bypass` | Состояние BYP. |
| `color.surface.micPreviewInner` / `moduleBackdrop` | Превью микрофона, подложка панели модуля. |
| `color.border.micPreview` / `modulePanel` | Обводка превью и секции модуля. |
| `color.text.tertiary` / `caption` / `title` | Строка сборки, подпись под превью, заголовок окна. |
| `color.control.rotaryOutline` | Дефолтный контур ротори (`styleRotary`). |
| `color.macro.*` | Секция «Macros» и заливка каждого из восьми макросов (Glue, Air, …). |
| `color.knob.*` | Акцент заливки ротори по типу модуля (micAmount, deess, opto, …). |

Новые экраны и компоненты должны **брать цвета по семантике**, а не вводить новый hex без записи в `tokens.json`.

**Нативный редактор:** основной файл `PluginEditor.cpp` использует токены (псевдоним `tkn`); литералов `0xff…` в нём для цветов быть не должно.

## 5. Как править палитру (чеклист)

1. Отредактировать `tokens.json` (и при необходимости описание в этом файле).
2. Обновить `tokens.css` (те же значения).
3. Обновить `DesignTokens.h` для всех непрозрачных цветов, которые используются в C++.
4. Заменить литералы в коде на `juce::Colour(tkn::argb::...)` / `tkn::macro::...` / `tkn::knob::...` (или полное имя `razumov::ui::tokens::`) по мере касания файлов.
5. Сборка Release и быстрая проверка в хосте (см. `.cursor/rules/agent-build-after-changes.mdc`).

Опционально в будущем: скрипт генерации `tokens.css` / заголовка из JSON (сейчас синхронизация вручную, явно зафиксирована).

## 6. Типографика

- **Веб-прототипы**: `font.family.display` (Outfit), `font.family.body` (Source Sans 3) — подключение в HTML, не в бинарнике плагина.
- **JUCE**: пока системные шрифты через LookAndFeel; кастомные шрифты — отдельная задача (встраивание в BinaryData, `FontOptions`).

## 7. Связанные документы

- `docs/design/UI_ASSETS.md` — PNG/SVG, углы, glow.
- `docs/UI_GRAPHICS_ROADMAP.md` — дорожная карта графики.
- `.cursor/rules/ui-strings-ascii.mdc` — строки UI.

## 8. Версия

Документ и токены привязаны к продукту; при крупной смене темы добавьте запись в `docs/CODE_HEALTH_AUDIT.md` (Quick check) или отдельный раздел в этом файле.

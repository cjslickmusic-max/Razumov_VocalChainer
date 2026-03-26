# Синтетический датасет АЧХ микрофонов

**Назначение:** заготовка для будущей коррекции в `MicCorrectionNode` (сейчас DSP не подключён). Реальные измерения можно подставить позже, сохранив те же имена файлов и формат.

## Формат CSV

- Первая строка: `frequency_hz,amplitude_db`
- Далее строки: частота в Гц, амплитуда в дБ (синтетическая кривая относительно выбранного референса).
- Один файл на модель — имя файла = идентификатор профиля (см. таблицу соответствия в `docs/MIC_PROFILES_LIST.md`).

## Соответствие файлов и моделей

| Файл | Модель |
|------|--------|
| `Behringer_XM8500.csv` | Behringer XM8500 |
| `Fifine_K669.csv` | Fifine K669 |
| `ATR2100x.csv` | Audio-Technica ATR2100x |
| `Samson_Q2U.csv` | Samson Q2U |
| `Superlux_PRA_D1.csv` | Superlux PRA-D1 |
| `BM800.csv` | BM-800 / клоны |
| `Shure_SM58.csv` | Shure SM58 |
| `Shure_SM57.csv` | Shure SM57 |
| `AKG_P120.csv` | AKG P120 |
| `Rode_NT1A.csv` | Rode NT1-A |
| `AT2020.csv` | Audio-Technica AT2020 |
| `MXL_990.csv` | MXL 990 |
| `Shure_SM7B.csv` | Shure SM7B |
| `EV_RE20.csv` | Electro-Voice RE20 |
| `AKG_C214.csv` | AKG C214 |
| `Rode_NT2A.csv` | Rode NT2-A |
| `Lewitt_440.csv` | Lewitt LCT 440 PURE |
| `Aston_Origin.csv` | Aston Origin |
| `Neumann_TLM102.csv` | Neumann TLM 102 |
| `Neumann_TLM103.csv` | Neumann TLM 103 |
| `AKG_C414.csv` | AKG C414 XLII |
| `Shure_KSM44.csv` | Shure KSM44 |
| `Sennheiser_MK4.csv` | Sennheiser MK4 |
| `Neumann_U87.csv` | Neumann U87 Ai |
| `Sony_C800G.csv` | Sony C-800G |
| `Telefunken_U47.csv` | Telefunken U47 |
| `Neumann_U67.csv` | Neumann U67 |
| `AKG_C12.csv` | AKG C12 |
| `Royer_R121.csv` | Royer R-121 |
| `Coles_4038.csv` | Coles 4038 |

## Важно

Данные **синтетические** — для разработки пайплайна загрузки и UI, не для субъективной оценки «как звучит микрофон».

# Cursor skills in this repo

Installed under **`.cursor/skills/`** (team-shared). Restart Cursor after pull so skills reload.

| Skill | Source |
|-------|--------|
| `fft-spectrum-engineering` | Проектный: `.cursor/skills/fft-spectrum-engineering/SKILL.md` — FFT/спектр, тапы по графу, SIMD-задел; см. `docs/FFT_SPECTRUM_ARCHITECTURE.md` |
| `ui-ux-pro-max` | `uipro init --ai cursor` ([ui-ux-pro-max-skill](https://github.com/nextlevelbuilder/ui-ux-pro-max-skill)) |
| `frontend-design` | [anthropics/skills](https://github.com/anthropics/skills/tree/main/skills/frontend-design) (SKILL.md + LICENSE) |
| `kpi-dashboard-design` | [wshobson/agents](https://github.com/wshobson/agents) (`npx skills add ... --skill kpi-dashboard-design --yes`) |
| `responsive-design` | wshobson/agents |
| `interaction-design` | wshobson/agents |
| `liquid-glass-design` | Based on [dagba/ios-mcp swiftui-liquid-glass](https://github.com/dagba/ios-mcp) + `FOR_NATIVE_PLUGIN_UI.md` for JUCE notes |

Update wshobson-derived skills (CLI may install under `.agents/`; copy into `.cursor/skills/` or add `.agents/` to `.gitignore`):

```bash
npx --yes skills add https://github.com/wshobson/agents --skill <name> --yes
cp -R .agents/skills/<name> .cursor/skills/
```

**Lock file:** `skills-lock.json` tracks hashes for skills installed via `npx skills`.

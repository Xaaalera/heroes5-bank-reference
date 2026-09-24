# Heroes V Bank Reference

## RU

Справочник **возможных армий хранилищ** с портретами, диапазонами и альтернативами по тирам для Heroes V: Повелители Орды с **[Universe / Heroes V Lobby](https://h5lobby.com/)**. [Сообщество Universe](https://vk.com/h5universe) · [разработка Universe](https://boosty.to/verydobro).

Мод не читает выбранный тир или скрытую охрану объекта. Показывает справочные варианты; число под парой портретов относится к группе, а не обещает столько существ каждого вида. [Каталог и расчёт диапазонов](https://xaaalera.github.io/heroes5-knowledge/reference/banks/) · [установка из исходников](#установка-из-исходников).

### Из чего состоит

`mod.json` содержит рецепт окон и собственный каталог `object_reference`. Другой мод с таким именем для сборки не нужен. Сборщик берёт UI-шаблоны, названия и портреты из установленной игры; игровые ресурсы не лежат в Git. Каталог DefaultStats закреплён SHA-256, нативное подключение — [четырьмя бинарными хешами](https://xaaalera.github.io/heroes5-knowledge/reference/universe-build/).

Проверенная среда: [Heroes V Mod Devkit](https://github.com/Xaaalera/heroes5-mod-devkit), ревизия [1b8934a](https://github.com/Xaaalera/heroes5-mod-devkit/tree/1b8934ac084491da460ce8bb145819e41e2cf888), закреплена submodule `devkit/`.

**Текущий способ подключения — H5U плюс нативный диагностический запуск devkit.** Одно копирование H5U не доказывает выбор нового окна. Отдельного пользовательского DLL-установщика для этого прототипа пока нет.

### Установка из исходников

Нужны Windows, Python 3.10+ x64, Git и поддерживаемая установленная Universe. Закрой игру и редактор. Команды PowerShell из нового каталога:

```powershell
git clone --recursive https://github.com/Xaaalera/heroes5-bank-reference.git
cd heroes5-bank-reference
python -m venv .venv
.venv/Scripts/python -m pip install -r devkit/requirements.txt
$env:H5_WORKSPACE = [IO.Path]::GetFullPath('../bank-reference-workspace')
$env:H5_GAME_DIR = (Resolve-Path '../HeroesV-Universe').Path
.venv/Scripts/python devkit/scripts/mod-dev.py prepare --sandbox
.venv/Scripts/python devkit/scripts/mod-dev.py build --sandbox --mod army-reference --source .
.venv/Scripts/python devkit/scripts/mod-dev.py deploy --sandbox --mod army-reference
.venv/Scripts/python devkit/scripts/native-probe.py launch --army-layout --control
```

Замени `../HeroesV-Universe` своей установкой, выбери отдельную рабочую папку. Переменные задаются снова в новом PowerShell. `prepare` создаёт копию игры и откажется перезаписывать существующую; после первого раза повторять его не нужно. После изменения рецепта сначала выполняется build: deploy устанавливает готовый H5U.

Игра открывает меню. Открой карту и наведи на поддерживаемое хранилище. У тайника бесов в контроле отображались T1–T4 и диапазоны 90–135 / 120–165 / 150–195 / 180–225; другие объекты и последние правки оформления не получили такого же полного живого подтверждения. При работе через полигон его можно создать командой `test-map.py` из devkit и добавить `--map WorkshopPolygon` при запуске.

### Отключение и удаление

Закрой тестовую игру через меню либо `game_control.py quit` из devkit, проверь завершение процесса. Затем:

```powershell
.venv/Scripts/python devkit/scripts/mod-dev.py rollback --sandbox --mod army-reference
```

Удаляется только собственный неизменённый `workshop-army-reference.h5u` из копии игры. Новый обычный запуск без `--army-layout` не устанавливает селектор окон. Откат не удаляет всю тестовую игру или чужие моды.

### Границы и проверки

- 19 публичных названий связаны с 12 подтверждёнными типами; каталог содержит 13 семейств, но привязка OrcDeposit не установлена. Переименованные объекты не поддерживаются этим выбором по имени.
- Это справочные варианты, не подсказка фактической охраны и не оценщик победы. Совместная установка с предиктором отдельно не проверена.
- Последние положения текста/масштаб и все хранилища требуют дальнейшего визуального контроля. История опытов — [в дневнике](https://xaaalera.github.io/heroes5-knowledge/reference/research-diary/#army-tooltip-probe).
- При переносе пакет собран отдельно от мастерской и побайтно совпал с предыдущей сборкой: SHA-256 `824a14b48fbdadce9ea0475b49bc4d1f1f6d2ef112ef8294b63cc1ea4ebf7f1e`. Это проверка сборки, не новый запуск игры.

Для четырёх изолированных проверок рецепта/нативного выбора без игры:

```powershell
.venv/Scripts/python -m pip install -r requirements-dev.txt
.venv/Scripts/python -X utf8 scripts/check.py
```

Отчёт JSON перечисляет результаты и пропуски; зависимости должны быть установлены. Полная изоляция записей профиля тестовой копией не доказана. Не публикуй свою игру, профили или build-журналы вместе с исходниками.

## EN

Reference **possible bank armies**, portraits, ranges and alternatives by tier for Heroes V: Tribes of the East with the linked Universe project. It does not read a bank's hidden selected tier/guards or predict victory. Counts below paired portraits describe the group, not a guaranteed quantity of both species. See the wiki and range catalog.

### Installation and removal

The self-contained mod.json embeds its own catalog; no separate object-reference mod is needed. Templates, portraits and names come from the installed game. The pinned devkit revision above records the tested environment; DefaultStats and native binaries have hash checks.

The current prototype needs **H5U plus the devkit native diagnostic launch**; there is no standalone player DLL installer. Use the shared PowerShell sequence with Windows, Git, Python 3.10+ x64 and your supported Universe installation. Close game/editor, replace the example game directory and choose a separate workspace. Reapply variables in new shells; prepare only creates a new sandbox. Rebuild after recipe changes before deploying.

Launch with --army-layout --control, open a map and hover a supported bank. The observed imp-cache window showed T1–T4 with the shared ranges. Other families and final presentation tweaks do not have identical live coverage. Optionally generate the devkit polygon and pass --map WorkshopPolygon.

Exit the game normally, confirm process termination, then run the shared rollback command. It removes only the owned unchanged H5U, not the sandbox or unrelated mods. An ordinary subsequent launch without --army-layout does not install the window selector.

### Limits and verification

Nineteen public titles cover twelve confirmed types; thirteen catalog families include an unresolved OrcDeposit binding. Renamed objects are unsupported. Combined use with the deployment predictor is unverified. Layout refinements/all-bank coverage still need visual checks. The extracted standalone package was byte-identical to the prior build at the SHA-256 above; no fresh live launch followed extraction.

Run the shared requirements-dev/check commands for four isolated recipe/native-selector checks without a game. JSON reports results/skips. Complete profile-write isolation is unproven. Do not publish game files, profiles or build logs.

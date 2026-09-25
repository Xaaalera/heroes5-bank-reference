# Heroes V Bank Reference

## RU

Справочник **возможных армий хранилищ** для [Heroes V Universe](https://h5lobby.com/): портреты, тиры, диапазоны и альтернативы. Фактическую скрытую охрану объекта не читает. [Описание и чтение карточек](https://xaaalera.github.io/heroes5-knowledge/players/bank-reference/) · [каталог хранилищ](https://xaaalera.github.io/heroes5-knowledge/reference/banks/).

### Для игрока

Готовый пакет готовится к предварительному выпуску в [Releases](https://github.com/Xaaalera/heroes5-bank-reference/releases). Code → Download ZIP скачивает исходники и не подходит для установки.

1. Закрой игру и редактор. Распакуй папку `Heroes5BankReference` рядом с игровой папкой `bin`.
2. Двойным щелчком открой `workshop_bank_reference.exe`. Если игра не найдена рядом, выбери её `bin/H5_Game.exe` в окне выбора файла.
3. Загрузчик проверяет версию, копирует свой `workshop-army-reference.h5u` в `UserMODs` и запускает игру со справочником. Отличающийся файл с тем же именем не перезаписывается.
4. На карте наведи на поддерживаемое хранилище. Карточка показывает возможные армии, а не разведанный состав.

Python, Git и devkit игроку не нужны. Загрузчик и H5U должны лежать рядом. Проверяется [конкретная сборка Universe](https://xaaalera.github.io/heroes5-knowledge/reference/universe-build/); игровые EXE/DLL на диске не патчатся.

**Удаление:** закрой игру. Удали `UserMODs/workshop-army-reference.h5u` и папку `Heroes5BankReference`. Для запуска без селектора используй обычный игровой EXE. Совместная работа с предиктором не подтверждена.

Нативный загрузчик новый: сборка и изолированные проверки не заменяют живую приёмку. Пакет пока не объявлен стабильным. Исторически в игре подтверждён тайник бесов, но не все хранилища и последние изменения оформления.

### Разработчику: собрать пакет

Среда закреплена submodule [devkit 587e09c](https://github.com/Xaaalera/heroes5-mod-devkit/tree/587e09cc75014ca2cf443c486b46ad9bb9dc4b0e). Нужны Windows, Git, Python 3.10+ x64, CMake 3.21+ и Visual Studio 2022 C++ x86 tools. Игровые ресурсы в Git не входят.

PowerShell из новой папки; путь `../HeroesV-Universe` замени своей установленной игрой:

```powershell
git clone --recursive https://github.com/Xaaalera/heroes5-bank-reference.git
cd heroes5-bank-reference
python -m venv .venv
.venv/Scripts/python -m pip install -r requirements-dev.txt
$env:H5_WORKSPACE = [IO.Path]::GetFullPath('../bank-reference-workspace')
$env:H5_GAME_DIR = (Resolve-Path '../HeroesV-Universe').Path
.venv/Scripts/python devkit/scripts/mod-dev.py prepare --sandbox
.venv/Scripts/python devkit/scripts/mod-dev.py build --sandbox --mod army-reference --source .
.venv/Scripts/python scripts/build-player.py "$env:H5_WORKSPACE/.local/test-state/army-reference.h5u"
cmake -S . -B .local/player-build -A Win32
cmake --build .local/player-build --config Release
ctest --test-dir .local/player-build -C Release --output-on-failure
Copy-Item "$env:H5_WORKSPACE/.local/test-state/army-reference.h5u" .local/player-build/Release/workshop-army-reference.h5u
```

`prepare` выполняется один раз и не перезаписывает существующую тестовую копию. Команды выше ничего не запускают. Для проверки версии и пары EXE/H5U без записи/запуска:

```powershell
.local/player-build/Release/workshop_bank_reference.exe --check --game "$env:H5_WORKSPACE/.local/test-game/bin/H5_Game.exe"
```

Для упаковки нужны только `workshop_bank_reference.exe`, соответствующий `workshop-army-reference.h5u` и инструкция игроку. После изменения рецепта заново собираются H5U, generated header и EXE: в EXE закреплён SHA-256 H5U.

### Проверки и механизм

```powershell
.venv/Scripts/python scripts/check.py
```

Шесть проверок: прежний счётчик/селектор в x86-эмуляторе, рецепт и подписи, отказ от неверных маршрутов, сборка нативного установщика на синтетических данных. CTest проверяет отказ на неверных/уже заменённых байтах и защиты памяти в собственном тестовом процессе. Дополнительно проверяются копирование H5U в пустую временную папку, повторная установка без перезаписи и отказ при отличающемся существующем файле. Игру эти тесты не запускают. Для них нужны CMake и x86-компилятор; отсутствие инструмента — ошибка, не PASS.

Генератор на этапе сборки берёт `layout_trampoline` и `layout_data` закреплённого devkit, определяет relocations через Capstone и сравнивает результат на трёх адресах с исходным генератором. EXE использует готовые байты: запускает собственный процесс suspended, проверяет исходные шесть байт `0x5f8800`, устанавливает тот же селектор, восстанавливает защиту и продолжает запуск. Неудачная установка завершает только собственный новый процесс. При ошибке запуска уже установленный H5U может остаться; его удаление описано выше.

19 публичных названий сопоставлены с 12 подтверждёнными типами; каталог включает 13 семейств, привязка OrcDeposit не найдена. Переименованные объекты не поддерживаются. Исходный H5U: SHA-256 `824a14b48fbdadce9ea0475b49bc4d1f1f6d2ef112ef8294b63cc1ea4ebf7f1e`. Новое подключение не расширяет доказанную область игровых проверок.

## EN

Reference possible bank armies for the linked Universe build: portraits, tiers, ranges and alternatives. It does not read actual hidden guards. See the wiki description and bank catalog linked above.

### Player package

A preview package is being prepared in Releases; the source ZIP is not an installer. Exit game/editor, extract `Heroes5BankReference` beside the game's `bin` folder and double-click `workshop_bank_reference.exe`. If needed, select the installed `bin/H5_Game.exe` in the file picker. Keep the EXE and its matching H5U together. No Python, Git or devkit installation is required for players.

The launcher validates the supported game, copies its `workshop-army-reference.h5u` into `UserMODs` without overwriting a different existing file, then starts the game with the selector. Hover a supported bank to see possible armies. Game EXE/DLL files remain untouched.

To remove: exit the game, delete `UserMODs/workshop-army-reference.h5u` and the mod folder. An ordinary game EXE launch omits the selector. Combined use with the predictor is unverified. This new native launch path has not yet received live acceptance and is not a stable release. Historical visual evidence covers the imp cache, not every bank or later presentation change.

### Developer build and checks

The pinned devkit revision and shared PowerShell commands above are canonical. Requires Windows, Git, Python 3.10+ x64, CMake 3.21+ and Visual Studio 2022 x86 tools. Replace the example game path. Prepare creates a separate sandbox once; it never overwrites an existing one. Build commands do not start a game. `--check --game <H5_Game.exe>` validates binaries and the EXE/H5U pairing without writing or launching.

Distribute only the built EXE, matching H5U and player instructions. Recipe edits require rebuilding the H5U, generated header and EXE because the package hash is compiled into the launcher. Game resources are not committed to Git.

Six checks cover the existing x86 counter/selector, recipe/labels, invalid routes, and a native build using synthetic routing data. CTest checks wrong/already-hooked entry rejection and memory protections inside its own disposable process. The same fixture also checks H5U copying into a fresh temporary directory, unchanged repeated installation and rejection of differing existing content. No game runs; absent CMake/compiler is a failure, not a pass.

The build generator consumes pinned devkit layout code/data, derives relocations through Capstone and compares rebasing at three addresses against the original generator. The native EXE uses those bytes without Python: creates its own suspended game, checks six original bytes at `0x5f8800`, installs the selector, restores protection and resumes. Setup failure terminates only that new child. If launch fails after resource installation, the owned H5U may remain; remove it as described above.

The public catalog has 19 titles, 12 confirmed types and 13 families, including an unresolved OrcDeposit binding. Renamed objects are unsupported. The shared H5U hash above identifies the unchanged resource build. New launch code does not extend prior live-game evidence.

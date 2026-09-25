# Heroes V Bank Reference

## RU

Справочник **возможных армий хранилищ** для [Heroes V Universe](https://h5lobby.com/): портреты, тиры, диапазоны и альтернативы. Фактическую скрытую охрану не читает. [Описание и пример](https://xaaalera.github.io/heroes5-knowledge/players/bank-reference/) · [каталог и расчёт диапазонов](https://xaaalera.github.io/heroes5-knowledge/reference/banks/).

### Игроку: обычный запуск

Поставка — DLL и H5U, без отдельного EXE. Версия 0.1.0-preview.2 экспериментальная; Наличие архива проверяй в [Releases](https://github.com/Xaaalera/heroes5-bank-reference/releases): пустой список означает, что пользовательский пакет ещё не опубликован. Старый EXE-кандидат отменён. Code → Download ZIP скачивает исходники.

1. Закрой игру и редактор. Файлы пакета размещаются в установленной игре:
   - bin/dinput8.dll — общий файл для обоих наших модов;
   - bin/Heroes5Mods/WorkshopBankReference.dll;
   - UserMODs/workshop-army-reference.h5u.
2. Не перезаписывай dinput8.dll другого мода: совместимость не проверена. Штатные d3d9.dll, uni.dll и um.dll не заменяются.
3. Если сохранился старый текстовый workshop-object-reference.h5u, убери его из UserMODs. Он добавляет длинное описание над портретами; новая DLL отказывается работать при этом конфликте.
4. Запускай Heroes/Lobby как раньше. На карте наведи на поддерживаемое хранилище: должна появиться справка с возможными армиями.

Python, Git и devkit игроку не нужны. DLL и H5U должны быть из одного выпуска. Поддерживается [закреплённая сборка игры](https://xaaalera.github.io/heroes5-knowledge/reference/universe-build/); несовместимость или ошибка модуля прекращает запуск с сообщением.

Для удаления закрой игру и убери WorkshopBankReference.dll и workshop-army-reference.h5u из указанных папок. Другие UserMODs не трогай. Общий dinput8.dll удаляй только после всех наших DLL-модов и только если он установлен из нашего пакета.

### Что проверено

Обычный H5_Game.exe загрузил обе DLL. Предиктор прошёл пять инструментированных боёв с включённым справочником. После удаления старого текстового пакета получена чистая карточка склепа; A/B размещены на разных строках, окно помещается в кадре 1264×921. Все хранилища и разрешения экрана этим не проверены. Проверялся русский интерфейс; другие локализации не проверены. Сам интерфейс Heroes/Lobby отдельно не автоматизировался.

19 публичных названий сопоставлены с 12 подтверждёнными типами; каталог включает 13 семейств, привязка OrcDeposit не найдена. Переименованные объекты не поддерживаются. H5U SHA-256: 824a14b48fbdadce9ea0475b49bc4d1f1f6d2ef112ef8294b63cc1ea4ebf7f1e.

### Разработчику

Закреплён [devkit 71509e4](https://github.com/Xaaalera/heroes5-mod-devkit/tree/71509e43af0faf080d8a47ed5b3ff8c72da2a3e9). Нужны Windows, Git, Python 3.10+ x64, CMake 3.21+ и Visual Studio 2022 C++ x86 tools. Игровые ресурсы не входят в Git. PowerShell; пример пути ../HeroesV-Universe замени своей установленной игрой:

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
    cmake --install .local/player-build --config Release --prefix .local/dist
    cmake -S devkit/native -B .local/bootstrap -A Win32
    cmake --build .local/bootstrap --config Release
    cmake --install .local/bootstrap --config Release --prefix .local/dist
    New-Item -ItemType Directory -Path .local/dist/UserMODs -Force
    Copy-Item "$env:H5_WORKSPACE/.local/test-state/army-reference.h5u" .local/dist/UserMODs/workshop-army-reference.h5u
    .venv/Scripts/python scripts/check.py

Prepare создаёт новую тестовую копию один раз и отказывается перезаписывать существующую. Команды выше не запускают игру. После изменения рецепта заново собираются H5U, generated header и DLL: в модуле закреплён хеш H5U.

По умолчанию CMake install ставит только DLL. Прежний EXE остаётся диагностикой и устанавливается лишь явным --component Diagnostics; игроку он не поставляется. Не совмещай его установку hook с автоматическим подключением DLL.

Генератор получает code/data из закреплённого native-probe, вычисляет релокации через Capstone и сравнивает результат на трёх адресах с исходным генератором. WorkshopBankReferenceInstall проверяет игру/H5U/отсутствие текстового прототипа, затем ставит селектор в текущий процесс после проверки шести исходных байт 0x5f8800. Bootstrap останавливает запуск при отказе любого модуля.

Шесть проверок включают x86-счётчик/селектор, рецепт, неверные маршруты и нативный fixture на синтетических данных. Проверяются защиты памяти, неверный/уже заменённый hook, копирование H5U в пустую временную папку, повтор без перезаписи, сохранение отличающегося файла при отказе и неизменность источника. Это проверки без игры; CMake/компилятор обязательны.

## EN

Reference possible bank armies for the linked Universe build: portraits, tiers, ranges and alternatives. It does not read actual hidden guards. The wiki explains the cards and data.

### Player installation

Check Releases for a published DLL/H5U archive; if none is listed, no player package is available yet. Do not use a separate EXE or source ZIP. Version 0.1.0-preview.2 is experimental; the old EXE draft was withdrawn. Exit game/editor and place the shared bin/dinput8.dll, bin/Heroes5Mods/WorkshopBankReference.dll and UserMODs/workshop-army-reference.h5u under the installed game directory.

Both mods share one bootstrap. Do not overwrite another mod's dinput8.dll without compatibility checks; original d3d9.dll, uni.dll and um.dll remain unchanged. Remove the old workshop-object-reference.h5u text prototype: it adds oversized descriptions, and the new DLL rejects that conflict.

Start through Heroes/Lobby as usual. Hover a supported bank for possible armies. No player Python, Git or devkit installation is required. DLL and H5U must belong to the same release. The linked four-hash game build is required; a mismatch or module failure cancels startup with a message.

To uninstall, exit and remove the bank DLL/H5U only. Leave unrelated UserMODs alone. Remove our shared dinput8.dll only after all our DLL mods are removed.

### Evidence and development

Ordinary H5_Game.exe startup loaded both DLLs; five instrumented predictor battles passed with bank reference enabled. After removing the old text package, a clean crypt card showed separate A/B rows within a 1264×921 frame. This does not cover every bank or display size. The tested interface was Russian; other localizations are unverified. The Heroes/Lobby UI was not separately automated.

There are 19 public titles, 12 confirmed types and 13 catalog families, including an unresolved OrcDeposit binding. Renamed objects are unsupported. The shared H5U hash above identifies the resource build.

Use the pinned SDK and shared PowerShell commands above with Windows, Git, Python 3.10+ x64, CMake 3.21+ and VS2022 C++ x86 tools. Replace the sample game path; prepare creates a fresh sandbox once. These commands do not launch the game. Default CMake install contains DLLs only. The EXE remains an explicit Diagnostics component, never a player artifact; do not combine its hook installation with automatic loading.

The build generator freezes the pinned devkit selector/data, derives relocations and compares three rebases. The DLL validates the game, paired H5U and absence of the legacy text package before checking and patching entry 0x5f8800 in its own process. Bootstrap aborts startup on failure.

Six game-free checks cover x86 behavior, recipe/routes, native memory protections and hook rejection, plus resource copying into a temporary directory, unchanged repeats and preservation of differing existing files/source. CMake/compiler availability is required. Recipe changes require rebuilding H5U, generated header and DLL together.

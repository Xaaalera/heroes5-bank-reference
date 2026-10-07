# Heroes V Bank Reference

## Текущая разработка / Current development

В нашей мастерской новая версия xkit поддерживает `xkit start army-reference --map WorkshopPolygon`, `xkit build army-reference` и `xkit release army-reference`. При разработке меняем исходники; сборку и применение поддерживаемых изменений выполняет SDK. При выпуске создаётся отдельный ZIP с DLL и H5U для обычного запуска игры.

Исходники адаптера и готовый SDK опубликованы в [xkit v0.1.1-preview.1](https://github.com/Xaaalera/heroes5-mod-devkit/releases/tag/v0.1.1-preview.1). В живой игре проверены применение DLL, автоматическая замена ядра, новая функция и откат несовместимого ядра с продолжением прежней функции. Карточка склепа и сохранение заполненного кеша при обновлении DLL и ядра SDK проверены. Опубликованный preview.3 использует прежний контракт запуска; его возможности не следует смешивать с проверками текущего адаптера.

EN: The workshop's xkit adapter is available in the sources and ready [SDK v0.1.1-preview.1](https://github.com/Xaaalera/heroes5-mod-devkit/releases/tag/v0.1.1-preview.1), with named start/build/release. Edit source files; SDK handles compilation and supported updates. Named native application, automatic core replacement, a newly available export and incompatible-core rollback are live verified. Crypt-card rendering and populated cache continuity across bank DLL replacement are verified; populated crypt-card cache continuity through core replacement is also live verified. Published preview.3 retains its legacy startup contract.

RU: проверенные зависимости: devkit `5f4af79`, Game API `2aa326d`. Мастерская использует один canonical checkout; самостоятельный clone получает закреплённые сабмодули.

EN: Verified dependency revisions: devkit `5f4af79`, Game API `2aa326d`. The workshop shares canonical checkouts; standalone clones use pinned submodules.

RU, 2026-10-07: [xkit 0.1.0](https://github.com/Xaaalera/heroes5-mod-devkit/releases/tag/v0.1.0) опубликован с готовым SDK. Новый SDK-плагин выпускается отдельной DLL; общие dinput8.dll и d3d9.dll обеспечивают подключение. Исходная библиотека игры сохраняется локально как d3d9.universe.dll и не распространяется. Проверены две независимые DLL и обычный запуск. Опубликованный preview.3 сохраняет прежний контракт запуска. Текущая разработка использует адаптер HMR, описанный выше.
EN: xkit 0.1.0 is published with a ready SDK bundle. New SDK plugins ship as separate DLLs with shared dinput8.dll/d3d9.dll infrastructure; the game original remains local as d3d9.universe.dll. Two independent player DLLs and ordinary startup are verified. Published preview.3 retains its legacy startup contract. Current development uses the HMR adapter described above.


[Game API](https://github.com/Xaaalera/heroes5-game-api) — shared C++ game bindings / общая библиотека привязок к игре.

Историческое состояние / Historical status, 2026-10-04: общий devkit разрабатывает ABI3 hot reload и выпуск новых SDK-плагинов в bin/Heroes5Mods/Plugins. Справочник сохраняет текущий legacy DLL-контракт и H5U; такая перезагрузка пока не поддержана. EN: Shared bootstrap remains the ordinary player entry; the SDK prototype does not automatically migrate this plugin. [SDK status/release notes](https://github.com/Xaaalera/heroes5-mod-devkit).


## Author and related projects / Автор и связанные проекты

Автор / Author: [Xaaalera](https://github.com/Xaaalera) · [email](mailto:dampirsimpl@gmail.com) · [личный Telegram / personal Telegram](https://t.me/Victima).

- [Deployment Preview / Предиктор](https://github.com/Xaaalera/heroes5-deployment-preview).
- [Bank Reference / Справочник армий](https://github.com/Xaaalera/heroes5-bank-reference).
- [Mod Devkit / Девкит](https://github.com/Xaaalera/heroes5-mod-devkit): shared development and test tools for both DLL mods.
- [Knowledge source / Исходники базы](https://github.com/Xaaalera/heroes5-knowledge) · [public knowledge / база знаний](https://xaaalera.github.io/heroes5-knowledge/).
- [Heroes V Universe / Heroes Lobby](https://h5lobby.com/).

RU: личные проекты автора; не официальные продукты Universe. EN: Personal projects by the author, not official Universe products.

## RU

Справочник **возможных армий хранилищ** для [Heroes V Universe](https://h5lobby.com/): портреты, тиры, диапазоны и альтернативы. Фактическую скрытую охрану не читает. [Описание и пример](https://xaaalera.github.io/heroes5-knowledge/players/bank-reference/) · [каталог и расчёт диапазонов](https://xaaalera.github.io/heroes5-knowledge/reference/banks/).

### Игроку: обычный запуск

Поставка — DLL и H5U, без отдельного EXE. Опубликован экспериментальный выпуск [0.1.0-preview.3](https://github.com/Xaaalera/heroes5-bank-reference/releases/tag/v0.1.0-preview.3). Выбирай готовый архив в Releases; Code → Download ZIP скачивает исходники. Проверки конкретного выпуска описаны ниже.

1. Закрой игру и редактор. Файлы пакета размещаются в установленной игре:
   - bin/dinput8.dll — общий файл для обоих наших модов;
   - bin/d3d9.dll — графический файл xkit, если он входит в пакет;
   - bin/Heroes5Mods/WorkshopBankReference.dll;
   - UserMODs/workshop-army-reference.h5u.
2. Для пакета с xkit сначала сохрани исходный bin/d3d9.dll как bin/d3d9.universe.dll, затем установи bin/d3d9.dll из того же пакета. Уже сохранённый оригинал не перезаписывай. Сохрани резервную копию прежнего dinput8.dll; файлы другого загрузчика без проверки совместимости не заменяй. uni.dll и um.dll не меняются.
3. Если сохранился старый текстовый workshop-object-reference.h5u, убери его из UserMODs. Он добавляет длинное описание над портретами; новая DLL отказывается работать при этом конфликте.
4. Запускай Heroes/Lobby как раньше. На карте наведи на поддерживаемое хранилище: должна появиться справка с возможными армиями.

Python, Git и devkit игроку не нужны. DLL и H5U должны быть из одного выпуска. Поддерживается [закреплённая сборка игры](https://xaaalera.github.io/heroes5-knowledge/reference/universe-build/); несовместимость или ошибка модуля прекращает запуск с сообщением.

Для удаления закрой игру и убери WorkshopBankReference.dll и workshop-army-reference.h5u из указанных папок. Другие UserMODs не трогай. Общий dinput8.dll удаляй только после всех наших DLL-модов и только если он установлен из нашего пакета.

После удаления всех модов, использующих xkit, восстанови сохранённый прежний dinput8.dll. Если установлен наш графический файл, убери его и переименуй сохранённый d3d9.universe.dll обратно в d3d9.dll. Пока другой мод использует xkit, общие файлы оставь на месте. Если происхождение файлов неизвестно, восстанови игру из своей резервной копии вместо удаления наугад.

### Что проверено

Новый пакет с xkit, 2026-10-07: проверены автоматическое подключение DLL при обычном запуске и штатное закрытие игры с кодом 0. Карточка именно нового игрового архива ещё не проверена. В режиме разработки проверены карточка склепа и сохранение её заполненного кеша при обновлении DLL и ядра SDK. Следующие результаты относятся к прежнему preview.2 и не подтверждают новый пакет.

Обычный H5_Game.exe загрузил обе DLL. Предиктор прошёл пять инструментированных боёв с включённым справочником. После удаления старого текстового пакета получена чистая карточка склепа; A/B размещены на разных строках, окно помещается в кадре 1264×921. Все хранилища и разрешения экрана этим не проверены. Проверялся русский интерфейс; другие локализации не проверены. Сам интерфейс Heroes/Lobby отдельно не автоматизировался.

19 публичных названий сопоставлены с 12 подтверждёнными типами; каталог включает 13 семейств, привязка OrcDeposit не найдена. Переименованные объекты не поддерживаются. H5U SHA-256: 824a14b48fbdadce9ea0475b49bc4d1f1f6d2ef112ef8294b63cc1ea4ebf7f1e.

### Разработчику

Опубликованный preview.2 проверен с [devkit 71509e4](https://github.com/Xaaalera/heroes5-mod-devkit/tree/71509e43af0faf080d8a47ed5b3ff8c72da2a3e9). Нужны Windows, Git, Python 3.10+ x64, CMake 3.21+ и Visual Studio 2022 C++ x86 tools. Игровые ресурсы не входят в Git. PowerShell; пример пути ../HeroesV-Universe замени своей установленной игрой:

В связанной мастерской `devkit/` этого мода — ссылка на единственный корневой SDK. Из корня мастерской выполнить `powershell -NoProfile -File scripts/sync-subrepos.ps1 -Check`, исправление — без `-Check`. Текущая версия задаётся HEAD общего SDK и gitlink мода; общие правки не копируются. Самостоятельный клон сохраняет обычный submodule. Перед выпуском закрепить SDK коммитом, синхронизировать gitlink и проверить потребителей.

    git clone --recursive https://github.com/Xaaalera/heroes5-bank-reference.git
    cd heroes5-bank-reference
    python -m venv .venv
    .venv/Scripts/python -m pip install -r requirements-dev.txt
    $env:H5_WORKSPACE = [IO.Path]::GetFullPath('../bank-reference-workspace')
    $env:H5_GAME_DIR = (Resolve-Path '../HeroesV-Universe').Path
    .venv/Scripts/python devkit/scripts/mod-dev.py prepare --sandbox
    .venv/Scripts/python devkit/scripts/mod-dev.py build --sandbox --mod army-reference --source .
    .venv/Scripts/python scripts/build-player.py "$env:H5_WORKSPACE/.local/test-state/army-reference.h5u"
    cmake -S devkit/native -B .local/bootstrap -A Win32
    cmake --build .local/bootstrap --config Release
    cmake --install .local/bootstrap --config Release --prefix .local/dist
    cmake -S . -B .local/player-build -A Win32 -DXKIT_GRAPHICS_FILE="$PWD/.local/bootstrap/Release/d3d9.dll"
    cmake --build .local/player-build --config Release
    cmake --install .local/player-build --config Release --prefix .local/dist
    Copy-Item .local/bootstrap/Release/d3d9.dll .local/dist/bin/d3d9.dll
    New-Item -ItemType Directory -Path .local/dist/UserMODs -Force
    Copy-Item "$env:H5_WORKSPACE/.local/test-state/army-reference.h5u" .local/dist/UserMODs/workshop-army-reference.h5u
    .venv/Scripts/python scripts/check.py

Prepare создаёт новую тестовую копию один раз и отказывается перезаписывать существующую. Команды выше не запускают игру. После изменения рецепта заново собираются H5U, generated header и DLL: в модуле закреплён хеш H5U.

Для выпуска с новым xkit сначала собери закреплённый SDK, затем добавь при настройке справочника `-DXKIT_GRAPHICS_FILE=<путь к собранному d3d9.dll SDK>`. В пакет включай именно эту библиотеку и соответствующий dinput8.dll. Сборка закрепляет её хеш в DLL справочника; файл из установленной игры для этой настройки не используй. Без параметра сохраняется прежняя поставка с исходной графической библиотекой. Это настройка игрового выпуска; режим разработки с HMR включается отдельно через xkit start.

Режим `-DBANK_MANAGED_SELECTOR=ON` передаёт callback и данные постоянному сервису xkit. Он включается при xkit start и выключен в игровом выпуске. В нашей тестовой игре проверены вызовы callback, карточка склепа и сохранение заполненного кеша при автоматическом обновлении DLL и ядра SDK. Изменение комментария проверяет цикл обновления; отдельный callback с меткой версии подтверждает выполнение изменённого кода. Несовместимая структура данных отклоняется, чтобы сохранить кеш окон.

По умолчанию CMake install ставит только DLL. Прежний EXE остаётся диагностикой и устанавливается лишь явным --component Diagnostics; игроку он не поставляется. Не совмещай его установку hook с автоматическим подключением DLL.

Генератор получает code/data из закреплённого native-probe, вычисляет релокации через Capstone и сравнивает три варианта размещения с исходным генератором. WorkshopBankReferenceInstall проверяет игру/H5U/отсутствие текстового прототипа, затем подключает выбор армии банка через BankLayout после проверки исходных инструкций. Bootstrap останавливает запуск при отказе любого модуля.

Шесть проверок включают x86-счётчик/селектор, рецепт, неверные маршруты и нативный fixture на синтетических данных. Проверяются защиты памяти, неверный/уже заменённый hook, копирование H5U в пустую временную папку, повтор без перезаписи, сохранение отличающегося файла при отказе и неизменность источника. Это проверки без игры; CMake/компилятор обязательны.

## EN

Reference possible bank armies for the linked Universe build: portraits, tiers, ranges and alternatives. It does not read actual hidden guards. The wiki explains the cards and data.

### Player installation

Check Releases for a published DLL/H5U archive; if none is listed, no player package is available yet. Do not use a separate EXE or source ZIP. Version 0.1.0-preview.3 is experimental; the old EXE draft was withdrawn. Exit game/editor and place the shared bin/dinput8.dll, bin/Heroes5Mods/WorkshopBankReference.dll and UserMODs/workshop-army-reference.h5u under the installed game directory.

Both mods share one bootstrap. For an xkit package, first retain the original bin/d3d9.dll as bin/d3d9.universe.dll, then install the package's bin/d3d9.dll. Preserve an already retained original and back up the previous dinput8.dll. Do not replace another loader without compatibility checks. uni.dll and um.dll remain unchanged. Remove the old workshop-object-reference.h5u text prototype: it adds oversized descriptions, and the new DLL rejects that conflict.

To uninstall, close the game and remove this mod's WorkshopBankReference.dll and workshop-army-reference.h5u. Keep shared files while another xkit mod uses them. After removing all xkit mods, restore the backed-up input DLL, remove our graphics facade and rename the retained d3d9.universe.dll back to d3d9.dll. If file ownership is unknown, use your game backup rather than deleting unknown files.

Developers building this delivery must first build the pinned SDK, then configure the bank with `-DXKIT_GRAPHICS_FILE=<built SDK d3d9.dll>`. Package that exact facade and its matching input bootstrap. The bank DLL embeds its digest; do not take this build input from an installed game. Omitting the option retains stock graphics verification. This changes delivery, not the plugin's algorithm or HMR support.

The local `-DBANK_MANAGED_SELECTOR=ON` prototype transfers callback bytes and data to the new xkit resident service. It defaults off and is absent from published preview.3. The owned test game verifies callback execution, crypt-card rendering and populated cache continuity through automatic bank and SDK core replacement. A comment-only bank source update checks lifecycle continuity; a separate marked callback test verifies changed behavior. A changed data-table layout is rejected to preserve cached-window compatibility.

Start through Heroes/Lobby as usual. Hover a supported bank for possible armies. No player Python, Git or devkit installation is required. DLL and H5U must belong to the same release. The linked four-hash game build is required; a mismatch or module failure cancels startup with a message.

To uninstall, exit and remove the bank DLL/H5U only. Leave unrelated UserMODs alone. Remove our shared dinput8.dll only after all our DLL mods are removed.

### Evidence and development

New xkit delivery, October 7, 2026: automatic DLL installation on ordinary startup and normal game exit with code 0 are verified. Card rendering from this exact player archive is not yet verified. Development mode verifies the crypt card and its populated cache through bank DLL and SDK core updates. The following results belong to the older preview.2 delivery and do not validate the new package.

Ordinary H5_Game.exe startup loaded both DLLs; five instrumented predictor battles passed with bank reference enabled. After removing the old text package, a clean crypt card showed separate A/B rows within a 1264×921 frame. This does not cover every bank or display size. The tested interface was Russian; other localizations are unverified. The Heroes/Lobby UI was not separately automated.

There are 19 public titles, 12 confirmed types and 13 catalog families, including an unresolved OrcDeposit binding. Renamed objects are unsupported. The shared H5U hash above identifies the resource build.

Use the pinned SDK and shared PowerShell commands above with Windows, Git, Python 3.10+ x64, CMake 3.21+ and VS2022 C++ x86 tools. Replace the sample game path; prepare creates a fresh sandbox once. These commands do not launch the game. Default CMake install contains DLLs only. The EXE remains an explicit Diagnostics component, never a player artifact; do not combine its hook installation with automatic loading.

The historical SDK link identifies published preview.2 verification. In a linked workshop, the mod's devkit directory is a junction to the single top-level SDK; current HEAD/gitlink define its version. Run `scripts/sync-subrepos.ps1 -Check` from the workshop, repairing without `-Check`. Never copy shared edits between repos. Standalone clones retain normal pinned submodules; commit SDK changes, synchronize pins and reverify consumers before releasing.

The build generator freezes the pinned devkit selector/data, derives relocations and compares three rebases. The DLL validates the game, paired H5U and absence of the legacy text package before checking original instructions and connecting bank army selection through BankLayout in its own process. Bootstrap aborts startup on failure.

Six game-free checks cover x86 behavior, recipe/routes, native memory protections and hook rejection, plus resource copying into a temporary directory, unchanged repeats and preservation of differing existing files/source. CMake/compiler availability is required. Recipe changes require rebuilding H5U, generated header and DLL together.

## Standalone use / Работа вне мастерской

RU: этот репозиторий можно использовать отдельно. Начни с его README и AGENTS.md; глобальная папка мастерской не обязательна. Если есть .gitmodules, выполни `git submodule update --init --recursive` после клонирования. В связанной мастерской используй её sync-subrepos вместо создания вторых checkout.
EN: This repository can be used independently. Start with its README and AGENTS.md; the global workshop is optional. If .gitmodules exists, initialize pinned dependencies with `git submodule update --init --recursive`. In a linked workshop use its canonical dependency synchronization.

- [Devkit commands / команды SDK](https://github.com/Xaaalera/heroes5-mod-devkit/blob/main/docs/commands.md).
- [Game API contracts / контракты библиотеки](https://github.com/Xaaalera/heroes5-game-api/blob/main/docs/mechanisms/game-bindings.md).
- [Research index / карта исследований](https://xaaalera.github.io/heroes5-knowledge/reference/research-index/).

[Code standards / стандарты кода](https://github.com/Xaaalera/claude-skills).

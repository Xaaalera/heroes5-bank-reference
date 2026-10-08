# Heroes V Bank Reference

## RU

Справочник возможных армий хранилищ: портреты, тиры, диапазоны и альтернативы. Фактическую скрытую охрану объекта не раскрывает.

## EN

Reference for possible bank armies: portraits, tiers, ranges and alternatives. It does not reveal the actual hidden guards.

## Документация / Documentation

- [Описание, установка и ограничения](https://xaaalera.github.io/heroes5-knowledge/players/bank-reference/) · [English](https://xaaalera.github.io/heroes5-knowledge/en/players/bank-reference/).
- [Готовые выпуски / Releases](https://github.com/Xaaalera/heroes5-bank-reference/releases).
- [SDK xkit](https://xaaalera.github.io/heroes5-knowledge/modding/devkit/).
- [Самостоятельный клон и зависимости / Standalone clone and dependencies](https://xaaalera.github.io/heroes5-knowledge/modding/native-projects/).
- [Команды SDK / SDK commands](https://xaaalera.github.io/heroes5-knowledge/reference/xkit-commands/).
- [Game API](https://xaaalera.github.io/heroes5-knowledge/reference/game-api/).
- [Карта исследований / Research index](https://xaaalera.github.io/heroes5-knowledge/reference/research-index/).

Карточка склепа проверена для preview.4; это не проверка всех хранилищ и разрешений. / The preview.4 crypt card was checked; other banks and resolutions are not thereby certified.

Текущие инструкции находятся только на сайте. Каждый мод устанавливается отдельно с указанными общими зависимостями; исходники не являются готовым игровым пакетом.

The website owns the living instructions. Each mod installs independently with its stated shared dependencies; source downloads are not player packages.

## Standalone use / Работа вне мастерской

После клонирования выполни `git submodule update --init`. Глобальная мастерская не обязательна. Если ветка объявляет корневую `game-api`, путь `devkit/game-api` должен ссылаться на неё, а не содержать вторую копию; версии должны соответствовать закреплённым коммитам. Различия старых веток описаны в инструкции самостоятельного клона на сайте. В общей мастерской зависимости проверяет её `sync-subrepos`. Рабочие правила — в [AGENTS.md](AGENTS.md).

After cloning, run `git submodule update --init`. The global workshop is optional. Where the branch declares a root `game-api`, `devkit/game-api` must reference that checkout with consistent pins. The site's standalone-clone guide explains older branches; shared workshop checkouts use its canonical dependency synchronization.

## Проекты и контакты / Projects and contacts

- [Предиктор / Deployment predictor](https://github.com/Xaaalera/heroes5-deployment-preview).
- [Справочник / Bank reference](https://github.com/Xaaalera/heroes5-bank-reference).
- [SDK xkit](https://github.com/Xaaalera/heroes5-mod-devkit).
- [Game API source](https://github.com/Xaaalera/heroes5-game-api).
- [Исходники базы / Knowledge source](https://github.com/Xaaalera/heroes5-knowledge) · [сайт / website](https://xaaalera.github.io/heroes5-knowledge/).
- [Стандарты / Code standards](https://github.com/Xaaalera/claude-skills).
- [Universe / Heroes Lobby](https://h5lobby.com/).

[Xaaalera](https://github.com/Xaaalera) · [email](mailto:dampirsimpl@gmail.com) · [Telegram](https://t.me/Victima). Наши проекты не являются официальными продуктами Universe. / Our projects are unofficial Universe tools.

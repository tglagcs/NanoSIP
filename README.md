# NanoSIP

🇬🇧 [English](#english) · 🇷🇺 [Русский](#русский)

---

## English

A custom fork of [MicroSIP](https://www.microsip.org/) 3.22.3 (Windows MFC/PJSIP softphone) —
a stripped-down, voice-only build with no update checks and no unnecessary internet access.

> This is a personal/niche fork built around one specific use case (outgoing calls only, no
> video, no contact management), not a general-purpose replacement for MicroSIP. If you need
> full functionality and support, use the original [MicroSIP](https://www.microsip.org/).

The source code lives in the [`NanoSIP/`](NanoSIP) subfolder, not in the repository root.

<details>
<summary>Screenshots</summary>
<p align="center">
  <img src="docs/nanosip.png" alt="NanoSIP main screen" width="260">
  <img src="docs/account.png" alt="NanoSIP account dialog" width="260">
  <img src="docs/settings.png" alt="NanoSIP settings dialog" width="420">
</p>
</details>

### What's different from the original MicroSIP

- **No update checks, no outbound telemetry-like network calls** — the auto-update checker
  (which contacted MicroSIP's server to check for new versions) and the network `POST` from
  the crash reporter (`crash-report2.microsip.org`) have been removed; local crash-dump
  writing to disk is left as-is.
- **Fully dark theme**, no light-theme toggle.
- **Voice only** — video codecs and all related UI are disabled (`opus.lib` is not used).
- **No Contacts tab** and no add/edit/import/export contact features (the tab itself is
  hidden, but internal caller-name resolution and the corporate directory are kept).
- **No outbound internet links** — the Visit Website / Help menu items and the Shortcuts
  dialog have been removed.
- **Simplified Account dialog** — most fields are tucked behind a collapsible "Advanced"
  section; Media Encryption (SRTP), TLS transport, Publish Presence and Voicemail Number have
  been removed; password display is simplified (a new account always shows the password in
  plain text, an existing account always shows it masked; the password is never stored in
  plain text in the config — same as upstream MicroSIP, it's encrypted before being written
  to the ini file).
- **Fewer buttons on the dialer screen** — DND, the auto-answer button and Conference have
  been removed; call recording (REC) is off by default (toggle it in settings).
- **Simplified Settings tab** — rarely-needed options were removed (ringtone file picker,
  Auto Answer, Random Popup Position, Default List Action selector, etc.); incoming calls,
  answer/hangup, call forwarding and the core audio/network settings all still work as before.

### Building

Requirements:
- Windows 10/11
- Visual Studio Build Tools 2022 with the **"Desktop development with C++"** workload
  (VCTools) and the **MFC/ATL** component (`Microsoft.VisualStudio.Component.VC.ATLMFC`)
- A Windows SDK (any reasonably recent 10.0.2xxxx.0 works — use whichever version you have
  installed in the commands below)

Build from the repository root (PowerShell); the path to `MSBuild.exe` depends on where you
installed Build Tools:

```powershell
Get-Process microsip,NanoSIP -ErrorAction SilentlyContinue | Stop-Process -Force
& "<path_to_Build_Tools>\MSBuild\Current\Bin\MSBuild.exe" `
  "NanoSIP\microsip.vcxproj" `
  /p:Configuration=Release /p:Platform=Win32 `
  /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0.26100.0 `
  /t:Build /m
```

For example, if Build Tools are installed under `C:\BuildTools`, the path would be
`C:\BuildTools\MSBuild\Current\Bin\MSBuild.exe`. If `MSBuild` is already on your `PATH`, you
can just call `msbuild ...` without the full path.

Close any running instance of the exe before building, otherwise the linker fails with
`LNK1104` (file in use).

The project hardcodes `PlatformToolset=v140` and `WindowsTargetPlatformVersion=8.1` — neither
exists in modern Build Tools, so you **must** override both via `/p:` (see the command above),
matching whatever toolset/SDK version you actually have installed.

Build output: `NanoSIP\Release\NanoSIP.exe`.

### Dependencies

The `pjproject` tree lives inside `NanoSIP/pjproject/`. A prebuilt static library is already
included in the repository: `pjproject/lib/libpjproject-i386-Win32-vc14-Release-Static.lib` —
you don't need to rebuild pjproject separately.

If you do need to rebuild pjproject (e.g. after editing
`pjproject/pjlib/include/pj/config_site.h`):

```powershell
& "<path_to_Build_Tools>\MSBuild\Current\Bin\MSBuild.exe" `
  "NanoSIP\pjproject\pjsip-apps\build\libpjproject.vcxproj" `
  /p:Configuration="Release-Static" /p:Platform=Win32 `
  /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0.26100.0
```

It's important to build the `Release-Static` configuration specifically (static CRT, `/MT`) —
it matches what NanoSIP/MicroSIP's own `Release` configuration uses. The plain `Release`
config in pjproject is built with the dynamic CRT (`/MD`) and won't link (`LNK2001`).

### License

[GPL-2.0-or-later](LICENSE), same as the original MicroSIP. The original author's copyright
notice is kept in `NanoSIP/res/main.rc2` (LegalCopyright). Licenses of bundled third-party
libraries (pjproject, Speex, libsrtp, WebRTC modules, JsonCpp, etc.) are listed in
[`THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md).

---

## Русский

Кастомный форк [MicroSIP](https://www.microsip.org/) 3.22.3 (Windows MFC/PJSIP софтфон) —
максимально лёгкая версия только для голосовых звонков, без проверки обновлений и без лишнего
выхода в интернет.

> Это личный/нишевый форк под конкретный сценарий использования (только исходящие звонки,
> без видео, без записи контактов), а не универсальная замена MicroSIP. Если нужны полная
> функциональность и поддержка — используйте оригинальный [MicroSIP](https://www.microsip.org/).

Исходники лежат в подпапке [`NanoSIP/`](NanoSIP), а не в корне репозитория.

<details>
<summary>Скриншоты</summary>
<p align="center">
  <img src="docs/nanosip.png" alt="Главный экран NanoSIP" width="260">
  <img src="docs/account.png" alt="Диалог Account" width="260">
  <img src="docs/settings.png" alt="Диалог Settings" width="420">
</p>
</details>

### Что изменено относительно оригинального MicroSIP

- **Без проверки обновлений и без отправки данных вовне** — вырезан auto-update checker
  (ходил на сервер MicroSIP проверять новую версию) и сетевой POST из crash-report
  (`crash-report2.microsip.org`); локальная запись crash-dump на диск оставлена как есть.
- **Полностью тёмная тема**, переключателя светлой темы нет.
- **Только голос** — видео-кодеки и весь связанный UI отключены (`opus.lib` не используется).
- **Без вкладки Contacts** и без функций добавления/редактирования/импорта/экспорта контактов
  (сама вкладка скрыта, но внутренний резолв имени звонящего и corporate directory оставлены).
- **Без выхода в интернет** — убраны пункты меню Visit Website / Help, диалог Shortcuts.
- **Упрощённый диалог Account** — большинство полей собрано под раскрывающуюся секцию
  «Дополнительно»; убраны Media Encryption (SRTP), TLS-транспорт, Publish Presence,
  Voicemail Number; показ/скрытие пароля упрощено (новый аккаунт — открытым текстом,
  редактирование существующего — только звёздочки; в конфиге пароль, как и в оригинальном
  MicroSIP, в открытом виде не хранится — шифруется перед записью в ini).
- **Меньше кнопок на экране звонилки** — убраны DND, кнопка авто-ответа и Conference;
  запись звонка (REC) по умолчанию выключена (включается в настройках).
- **Упрощённая вкладка настроек** — убраны редко нужные опции (выбор файла рингтона,
  Auto Answer, Random Popup Position, выбор Default List Action и т.п.); входящие звонки,
  ответ/отбой, переадресация и базовые настройки звука/сети работают как раньше.

### Сборка

Нужны:
- Windows 10/11
- Visual Studio Build Tools 2022 с workload **"Desktop development with C++"** (VCTools) и
  компонентом **MFC/ATL** (`Microsoft.VisualStudio.Component.VC.ATLMFC`)
- Windows SDK (подойдёт любой относительно новый 10.0.2xxxx.0 — в командах ниже укажите
  установленную у вас версию)

Сборка из корня репозитория (PowerShell), путь к `MSBuild.exe` зависит от того, куда у вас
установлены Build Tools:

```powershell
Get-Process microsip,NanoSIP -ErrorAction SilentlyContinue | Stop-Process -Force
& "<путь_к_Build_Tools>\MSBuild\Current\Bin\MSBuild.exe" `
  "NanoSIP\microsip.vcxproj" `
  /p:Configuration=Release /p:Platform=Win32 `
  /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0.26100.0 `
  /t:Build /m
```

Например, если Build Tools установлены в `C:\BuildTools`, путь будет
`C:\BuildTools\MSBuild\Current\Bin\MSBuild.exe`. Если переменная `MSBuild` есть в `PATH`,
можно просто вызвать `msbuild ...` без полного пути.

Перед сборкой нужно закрыть запущенный exe, иначе линковщик выдаст `LNK1104` (файл занят).

Проект изначально хардкодит `PlatformToolset=v140` и `WindowsTargetPlatformVersion=8.1` —
оба отсутствуют в современных Build Tools, поэтому при сборке обязательно переопределять их
через `/p:` (см. команду выше), указав установленную у вас версию toolset/SDK.

Результат сборки: `NanoSIP\Release\NanoSIP.exe`.

### Зависимости

Дерево `pjproject` лежит внутри `NanoSIP/pjproject/`. Готовая статическая либа уже собрана и
входит в репозиторий: `pjproject/lib/libpjproject-i386-Win32-vc14-Release-Static.lib` —
пересобирать pjproject отдельно не требуется.

Если всё же нужно пересобрать pjproject (например, после правок в `pjproject/pjlib/include/pj/config_site.h`):

```powershell
& "<путь_к_Build_Tools>\MSBuild\Current\Bin\MSBuild.exe" `
  "NanoSIP\pjproject\pjsip-apps\build\libpjproject.vcxproj" `
  /p:Configuration="Release-Static" /p:Platform=Win32 `
  /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0.26100.0
```

Важно собирать именно конфигурацию `Release-Static` (статический CRT `/MT`) — она совпадает
с тем, что использует `Release` у самого NanoSIP/MicroSIP. Обычная `Release` у pjproject
собирается с динамическим CRT (`/MD`) и не слинкуется (`LNK2001`).

### Лицензия

[GPL-2.0-or-later](LICENSE), как и оригинальный MicroSIP. Авторская атрибуция оригинала
сохранена в `NanoSIP/res/main.rc2` (LegalCopyright). Лицензии сторонних библиотек (pjproject,
Speex, libsrtp, WebRTC-модули, JsonCpp и т.д.) — в [`THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md).

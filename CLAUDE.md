# CLAUDE.md — project context

Read this first. It exists so a new session has full context without the
maintainer re-explaining the project.

## What this is

**Flameshot v2** — a personal fork of [Flameshot](https://github.com/flameshot-org/flameshot),
a screenshot tool. Upstream is cross-platform; **this fork targets Windows
only**. Repository: `MichaelNguyen5653/Flameshot-v2` (public).

The fork exists to add features upstream does not have, listed under
"Fork-specific features" below. Everything else is upstream code and should
be left alone unless a task requires otherwise.

## Stack and build

| | |
|---|---|
| Language | C++20 (`cxx_std_20`) |
| Framework | Qt 6 (Widgets, Gui, Network, Svg, LinguistTools) |
| Build | CMake ≥ 3.22, Visual Studio generator |
| Packaging | CPack → WiX (`.msi`) and ZIP |
| CI | GitHub Actions |

Fetched at configure time via `FetchContent`: KDSingleApplication (single
instance), QHotkey (global hotkeys), QtColorWidgets.

### Building on the maintainer's machine

Qt lives at `C:\Qt\6.9.3\msvc2022_64` (installed with `aqtinstall`; the Qt
GUI installer fails on this network with a redirect loop). CMake is the one
bundled with Visual Studio:

```
"C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
cmake .. -G "Visual Studio 18 2026" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.9.3\msvc2022_64"
cmake --build . --config RelWithDebInfo
```

Deploy for testing by copying the built exe over
`build\install\bin\flameshot.exe` — that folder already has the Qt runtime
beside it. A running `flameshot.exe` locks the file; stop the process first.

**Trap:** `cmake --install` re-runs `windeployqt` with `--debug` (it reads
`CMAKE_BUILD_TYPE`, always empty for multi-config generators) and deploys
debug Qt DLLs next to a release exe, so the app dies with "no Qt platform
plugin could be initialized". Copy the exe instead, or re-run
`windeployqt --release` afterwards.

## Architecture

Entry point `src/main.cpp`. With no arguments it starts the **daemon**
(tray icon, global hotkeys, hosts pinned screenshots); with arguments it
parses a CLI command. On Windows only daemon mode is really supported.

| Path | Role |
|---|---|
| `src/core/` | `Flameshot` singleton (capture entry points), `FlameshotDaemon` (tray, updates, IPC) |
| `src/widgets/capture/` | `CaptureWidget` — the fullscreen capture/annotation overlay |
| `src/tools/` | One directory per editor tool; `ToolFactory` maps `CaptureTool::Type` → class |
| `src/config/` | Settings UI (`GeneralConf`, `ShortcutsWidget`) |
| `src/utils/` | `ConfigHandler` (settings), `ScreenGrabber` (screen capture) |
| `data/img/app/` | Application artwork; `data/graphics.qrc` embeds it |

### Things that bite

- **`CaptureWidget` is effectively a singleton.** `OverlayMessage` is a
  static instance parented to it, its destructor exports the capture, and
  `Flameshot::m_captureWindow` is a single pointer. Do not instantiate two.
- **A window cannot span mixed-DPI monitors.** Qt gives a window one scale
  factor while the desktop has one per screen, so a spanning overlay draws
  at the wrong size. This is why multi-monitor capture uses one window per
  screen — see `snip-across-all-monitors` below.
- **Adding a tool type**: append to the `CaptureTool::Type` enum (bottom
  only — existing configs store the numbers), then update `ToolFactory`,
  `CaptureToolButton::iterableButtonTypes` and `buttonTypeOrder`.
- **Adding a config key**: add an `OPTION(...)` row in `confighandler.cpp`
  and a `CONFIG_GETTER_SETTER` in `confighandler.h`. Unrecognized keys in a
  user's config raise an error, so never remove one that shipped.
- **Formatting**: CI pins **clang-format 11**, which disagrees with newer
  versions. Use `pip install clang-format==11.1.0` rather than the one
  bundled with Visual Studio, or the lint job goes red.

## Fork-specific features

- **Snip across all monitors** — a third multi-monitor mode
  (`captureRegionMode=2`). The capture widget binds to the screen under the
  cursor and follows it between screens; other screens are covered by
  per-screen dimming overlays. First mouse press commits to one screen.
- **OCR tool** (`TYPE_OCR`, shortcut `O`) — extracts text from a selection
  using Windows.Media.Ocr via C++/WinRT. `src/tools/ocr/` has an abstract
  `OcrEngine` so other platforms can implement one. Runs on a worker thread;
  does a second pass at higher resolution when the recognized text is small,
  and inverts dark backgrounds.
- **Highlighter rectangles** — the rectangle tool has a fill mode (solid or
  highlighter). Highlight compositing lives in `src/tools/highlightstyle.h`
  and is shared with the marker tool.
- **Restart** entry in the tray menu.
- **First-run welcome dialog** offering to free the Print Screen key.
- Updates point at this fork's releases, not upstream's.

## Releasing

The version comes from the **git tag**; `Windows-release.yml` passes it to
CMake as `RELEASE_VERSION`. Tag `vX.Y.Z` (numeric only) and push:

```
git tag v14.1.1 && git push origin v14.1.1
```

That builds the MSI and publishes a GitHub Release with a checksum. Also
bump `FLAMESHOT_VERSION` in `CMakeLists.txt` so `Windows-pack` artifacts
agree. The installer is unsigned, so users see a SmartScreen warning.

**Trap:** PowerShell does not expand a variable inside a bare token starting
with a dash. `-DFOO=$bar` passes the literal text; it must be `"-DFOO=$bar"`.
This silently broke the release build three times.

`Linux-pack`, `MacOS-pack`, `build_cmake` and `deploy-dev-docs` are
`workflow_dispatch` only — this fork does not ship those platforms.

## Session log

Append a short entry per session. Newest last.

### 2026-07-31 — multi-monitor capture, OCR
- Added `captureRegionMode` (select monitor / active monitor / all monitors)
  with migration from the old `captureActiveMonitor` bool.
- First attempt used one overlay spanning the virtual desktop; abandoned
  after it rendered wrong on mixed DPI. Replaced with the per-screen design.
- Added the OCR tool, engine abstraction, preprocessing, results window.
- Set up the toolchain (VS 2026, Qt 6.9.3 via aqtinstall).

### 2026-08-02 — release pipeline, branding, highlighter
- Renamed the installer to `Flameshot-v2` with its own WiX upgrade GUID, so
  it no longer replaces an upstream Flameshot install.
- Made the git tag the source of truth for the version; added a
  configure-time guard that rejects a malformed version early.
- Published the first release, `v14.1.0`. Getting there took four attempts;
  the cause was the unquoted-`-D` PowerShell trap noted above. Installing
  the GitHub CLI so logs could actually be read is what solved it.
- Replaced all application artwork with the new logo, then cropped its
  `viewBox` because the artwork only filled 38% of its own canvas.
- Pointed the update checker at this fork; it now links the `.msi` directly.
- Added the tray Restart entry and the highlighter fill mode.
- Trimmed CI to Windows only.
- New installer artwork, ICO small sizes re-encoded as BMP (Windows search
  renders PNG-in-ICO badly), first-run welcome dialog, CLAUDE.md.
- Implemented the update flow from `docs/update-notification-spec.md`:
  amber tray badge, "Update to X and restart" menu entry, download →
  SHA256 verify → `msiexec /passive` via a temp cmd script that relaunches
  the app afterwards. Non-admin decision: per-machine stays (a per-user
  MSI cannot upgrade existing per-machine installs); UAC collects admin
  credentials.

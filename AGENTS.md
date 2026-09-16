# AGENTS.md — caQtDM

Guidance for AI coding agents working in this repository.

## What this project is

caQtDM: Qt-based display manager for EPICS control systems.
Upstream: https://github.com/caqtdm/caqtdm with three branches
`Development` / `Release` / `master` (roles: see Development Workflow
section). Agent worktrees live under `.claude/worktrees/`.

**Always check first which branch is checked out here**
(`git rev-parse --abbrev-ref HEAD`) — worktrees branch off `Development`
and do NOT contain feature-branch states.

## Structure (top level)

| Directory | Contents |
|---|---|
| `caQtDM_Lib/` | Core library: `caqtdm_lib.cpp` (central dispatcher, very large), HMI event bus (`hmisharedeventbus.*`, `hmisharedconfiglistmanager.*`), softPV/calc logic |
| `caQtDM_QtControls/` | All widgets (ca*/E*/S* classes, e.g. ENumeric/SNumeric for caNumeric/caSpinbox) + Qt Designer plugins |
| `caQtDM_Viewer/` | Application, `qtdefs.pri` (version/feature switches), `package/` with packaging for redhat/debian/windows(+wix7)/brew/appimage/flatpak/… |
| `caQtDM_Plugins/` | Control-system plugins: epics3, epics4, archive, bsread, modbus, opcua, gps, environment, internal, demo. Shared interface: `controlsinterface.h` — changes there affect ALL plugins, avoid if possible |
| `caQtDM_Parsers/` | adl (MEDM) and edl (EDM) parsers, each as static + shared lib (edl on Unix only); see MEDM/EDM section |
| `caQtDM_UnitTests/` | qmake test projects: `tst_qtcontrols` (incl. oracle-based `tst_numeric_suite.h`), `tst_lib`, `tst_plugins`, `tst_viewer` |
| `caQtDM_Tests/` | Test/demo UI panels (.ui) for manual testing |
| `caQtDM_Web/`, `caQtDM_docs/`, `ci/` | Browser variant (see caQtDM_Web section), Sphinx docs (see Documentation section), CI helper files |

## MEDM / EDM (legacy predecessors and format reference)

caQtDM can replaces the older EPICS display managers and still reads their
panel formats:

- **MEDM** (Motif Editor and Display Manager, `.adl` panels) — upstream:
  https://github.com/epics-extensions/medm.git
- **EDM** (Extensible Display Manager, `.edl` panels) — upstream:
  https://github.com/gnartohl/edm.git

Relevant for work here:

- The parsers in `caQtDM_Parsers/` are partly derived from the original
  sources (`adlParserSrc/parser.c` from MEDM; `edlParserSrc/expString.cc`,
  `tag_pkg.cc`, `parserClass.cc` from EDM). For questions like "what does
  this adl/edl attribute mean?", the behavior of MEDM/EDM in the upstream
  repos is the reference.
- Each parser is built as static + shared lib (`caQtDM_Parsers.pro`); the
  **edl side only on Unix**, Windows builds only the adl parser. There are
  also standalone converters: `adl2ui` (`caQtDM_Viewer/parser/`) and
  `edl2ui` (`caQtDM_Viewer/parserEDM/`); shared qmake scopes
  `caQtDM_xdl2ui`/`caQtDM_xdl2ui_Lib` in `caQtDM.pri`.
- At runtime the viewer converts .adl/.edl on the fly
  (`CONFIG ADL_EDL_FILES` in `qtdefs.pri`) — disabled on MOBILE, where an
  .ui file of the same name is assumed for .adl references.
- The static variants (`libadlParser.a`/`libedlParser.a`) are expected in
  the collect dir at link time — if missing there, the link fails (see
  Build section).

### Parser linking (INTERIM state — to be resolved)

Two coexisting linking patterns, per parser:

- **prc (new pattern):** `libprcParser.a` is linked **statically into
  libqtcontrols** on Linux (`caQtDM.pri`, QtControls scope; the static
  lib builds with `-fPIC` for this). libqtcontrols.so is self-contained —
  no DT_NEEDED, works with `CAQTDM_NORPATH` packaging, and the
  UiConverterFactory works everywhere qtcontrols is loaded (viewer,
  caInclude, Designer). macOS links the dylib with full path, Windows the
  import lib (prcParser.dll ships in the MSI).
- **adl/edl (legacy pattern):** their symbols (via `parseotherfile.cpp`
  in QtControls) stay **unresolved** in libqtcontrols.so; the viewer
  links `-ladlParser/-ledlParser` on top (`caQtDM.pri` viewer scope).
  Consequence: every executable must link them itself, and the Designer
  cannot resolve these paths.

This mixed state is transitional and should be resolved by migrating
adl/edl to the static pattern (then drop the viewer link lines and the
macOS dylib special cases; caInclude could then load .adl/.edl in the
Designer too). Never add a `-l<parser>` line to the QtControls scope
while both `.a` and `.so` exist in the collect dir — ld picks the `.so`
and NORPATH package builds break (RPM link failure 2026-08).

Runtime converter selection (`caQtDM_QtControls/src/uiconverter.{h,cpp}`,
`UiConverterFactory`): dispatch by file suffix — `prc` always, `adl`/`edl`
only under `ADL_EDL_FILES`. For prc, env `CAQTDM_CONVERTER_PRC=new|old`
(compat alias `CAQTDM_PRC_CONVERTER=1` = new) picks `ParsePrcFile` (new)
vs `ParsePepFile` (old); PEP panels use the `.prc` extension. Callers:
caInclude and `caqtdm_lib.cpp`. Note caInclude itself only loads `.ui`
and `.prc` — any other extension in its file property is replaced by
`.ui` (`cainclude.cpp`, format switch).

## QtControls: widgets & Designer plugins

- **Four Designer plugin groups** (`caQtDM_QtControls/plugins/`, one .pro
  each): `qtcontrols_controllers` (caNumeric, caSlider, caMenu,
  caTextEntry, …), `qtcontrols_graphics` (caFrame, caLabel, caGraphics,
  caInclude, …), `qtcontrols_monitors` (caLineEdit, caThermo,
  caCartesianPlot, caCamera, caCalc, genSoftPV, …),
  `qtcontrols_utilities` (replaceMacro, wmSignalPropagator,
  caShellCommand, caScriptButton, …). All include `plugins/plugins.pri`
  (`TEMPLATE = lib` + `CONFIG += plugin`, DESTDIR
  `$(CAQTDM_COLLECT)/designer`). New widget = widget class +
  `d_plugins.append(...)` in the matching `*_plugin.cpp` + pixmap +
  entry in `designerPluginTexts.h`.
- **designer vs uiplugin:** desktop builds use `QT += designer`,
  ios/android (MOBILE) `uiplugin` (Qt6 android: neither — plugins are
  linked statically). The conditions differ slightly between
  `caQtDM_QtControls.pro` and `plugins/plugins.pri`.
- **`DESIGNER_TOOLTIP_DESCRIPTIONS`** (qtdefs.pri): enables tooltip
  descriptions in the widgets' domXml — historically for PSI's patched
  Qt 4.8.2 designer, also set for Qt 5 (> 5.5), NOT set for Qt 6.
  Caution (qtdefs.pri): set for a designer that doesn't understand it,
  the widgets disappear from the Designer entirely.
- **`caWidgetInterface`** (`src/caWidgetInterface.h`): opt-in abstract
  interface (caDataUpdate, caActivate, getWidgetInfo, createContextMenu,
  getDragText) meant to move per-widget special cases out of
  `caqtdm_lib.cpp` — the dispatcher probes it via `dynamic_cast` and
  falls back to the classic type checks. So far only `caLineDraw`
  implements it; it is NOT a repo-wide widget interface.
- **`wmSignalPropagator`** (utilities group): Designer bridge from panel
  widgets to window operations of the surrounding main window (close,
  reload, show*, print, resize; `caqtdm_lib.cpp` collects instances and
  connects the `wm*` signals). Despite the name it has NO relation to
  the HMI event bus.
- `qtdefinitions.h`: small global defines used everywhere, e.g.
  `qasc(x)` (= `x.toLatin1().constData()`) and `MAX_STRING_LENGTH`.

## caQtDM_Lib: core objects & data flow

- **`knobData`** (`src/knobData.h`, plain C struct, extern "C"): one
  record per widget-channel connection — pv name, widget pointer
  (`dispW`), plugin pointer/name, and the embedded **`epicsData edata`**
  (connection state, fieldtype, status/severity, limits, precision,
  units, `rvalue`/`ivalue`, vector data `dataB`, `monitorCount` vs
  `displayCount`, per-channel `repRate`, default 5 Hz).
- **`MutexKnobData`** (`src/mutexKnobData.{h,cpp}`): thread-safe store
  of all knobData records + the softPV lists. Control-system plugins
  deliver updates from their own threads via
  `SetMutexKnobDataReceived()`; a timer (starts at 10 Hz, adapts to the
  fastest requested repRate) compares monitorCount/displayCount and
  triggers the widget repaint in `caqtdm_lib.cpp` — widgets are never
  updated directly from plugin threads.
- **C wrappers** for EPICS C callbacks: `mutexKnobDataWrapper.h`
  (`C_SetMutexKnobDataReceived`, `C_DataLock`, …) and
  `messageWindowWrapper.h` (`C_postMsgEvent`, see Logging).
- **`loadPlugins`** (`src/loadPlugins.{h,cpp}`): fills a
  `QMap<QString, ControlsInterface*>`; `caqtdm_lib.cpp` routes each PV
  to its plugin by prefix (`epics3://`, `internal://`, …).
- **`CaQtDM_Lib`** (`src/caqtdm_lib.cpp`): central dispatcher — loads
  the .ui, replaces macros, creates the monitors per widget type and
  handles display/write logic (huge file, grep instead of reading).
  The three key functions:
  - `scanWidgets(list, macro)`: after loading a panel, walks all child
    widgets in three passes (primary softPVs working on their own value
    first, then other softPVs, then everything else) and calls
    HandleWidget for each.
  - `HandleWidget(w, macro, firstPass, treatPrimary)`: per-widget setup —
    reads the channel/property definitions, applies macros, creates the
    knobData monitors and wires the widget signals.
  - `Callback_UpdateWidget(indx, w, units, fec, String, data)`: slot for
    `MutexKnobData::Signal_UpdateWidget` (GUI thread) — the giant
    per-widget-type switch that pushes new data into the widget.
- **caInclude special case** (inside `HandleWidget`, caqtdm_lib.cpp
  ~2639–3100): the included panel is instantiated by the DISPATCHER, not
  by the widget itself — QUiLoader for `.ui`, UiConverterFactory for
  `.prc`, placed into a frame/grid layout built at load time. One copy
  per macro-list entry / item count (`qMax(macroList.count(),
  getItemCount())`), stacked Row/Column/RowColumn/ColumnRow or free
  `Positions`; each copy gets its own macro set (`treatMacro`,
  `savedMacro[level]`). Nested includes recurse depth-first via
  `scanWidgets()` with a `level`/`cainclude_path` stack, so relative
  paths resolve against the including file. With `Positions` stacking,
  `Callback_UpdateWidget` moves/resizes the copies at runtime via PVs.

## Build

- qmake project (`all.pro`); entry via `caQtDM_Env`/`caQtDM_BuildAll`
  (Unix) or the `caQtDM_*.bat` scripts (Windows). Feature detection and
  switches live centrally in `caQtDM_Viewer/qtdefs.pri`, per-OS
  link/include logic in `caQtDM.pri`.
- **Never build inside the source tree** (standing rule): builds go
  exclusively into temp directories; the user often builds/tests himself.
- GitHub CI (`.github/workflows/`) is the reference for reproducible
  per-platform builds, especially `build-rpm-package.yml` (RHEL/Alma).

### Environment variables

Mandatory — `all.pro` aborts with `error()` otherwise:
`QTHOME`, `QWTHOME`, `EPICS_BASE`, `EPICS_HOST_ARCH`.

Paths/libs (defaults are set by `caQtDM_Env`, only if not already set):

- `QWTINCLUDE`, `QWTLIB`, `QWTVERSION`, `QWTLIBNAME` — `QWTLIBNAME` is
  `qwt`, `qwt-qt5` or `qwt-qt6` depending on the distro (a common Linux
  pitfall).
- `EPICSINCLUDE`, `EPICSLIB` (= `$EPICS_BASE/lib/$EPICS_HOST_ARCH`),
  `EPICSEXTENSIONS`, `EPICS4LOCATION` (only for the old epics4 path).
- `CAQTDM_COLLECT` and `QTCONTROLS_LIBS` — collect directory for all built
  libs/plugins (default `caQtDM_Binaries/`); `QTBASE` is pointed at it by
  `caQtDM_Env`. Linking happens against this directory — if something is
  missing there (e.g. `libadlParser.*`/`libedlParser.*`), the link fails.
- `QTDM_RPATH` (rpath entries), `QTDM_LIBINSTALL`/`QTDM_BININSTALL`
  (install targets under `$EPICSEXTENSIONS`).
- `ZMQ`/`ZMQINC`/`ZMQLIB` — the existence of `$ZMQINC/zmq.h` automatically
  enables the bsread plugin.
- `PYTHONVERSION`/`PYTHONINCLUDE`/`PYTHONLIB` — for PYTHONCALC (Python in
  caCalc/visibility).

Feature switches (`qtdefs.pri`; they act via set/not-set — the value does
not matter, convention is `1`; empty = off):

- `CAQTDM_WEB` — web build (qnovnc, see below), Qt >= 5 only.
- `CAQTDM_MODBUS`, `CAQTDM_GPS`, `CAQTDM_OPCUA` — build the respective
  plugin; OPC UA additionally requires the Qt module `opcua` (encryption
  only with QOpcUaX509 headers).
- `CAQTDM_NORPATH` — build WITHOUT rpath; set in package builds (RPM).
- `HOMEBREW_MAKE_JOBS` set -> unit tests are NOT built (Homebrew special
  case).

Auto-detection without switches: the epics7/epics4 plugin via
`exists($EPICSINCLUDE/pv/pvAccess.h)`; archive/archiveSF/archiveHTTP are
always on.

### OS differences

- **Linux:** links with `-L$CAQTDM_COLLECT` + rpath from `QTDM_RPATH`
  (unless `CAQTDM_NORPATH` is set — the default in RPM builds). Watch the
  distro names of the Qwt lib (`QWTLIBNAME=qwt-qt6` on RHEL10). EPICS
  includes come from `os/Linux`.
- **macOS:** dylibs are linked directly with full paths from
  `$CAQTDM_COLLECT` (no -L/-l); Qwt as a framework
  (`QWTINCLUDE=$QWTHOME/lib/qwt.framework/Headers`). Details and pitfalls
  in the macOS recipe below.
- **Windows:** entry via `caQtDM_Env.bat`/`caQtDM_BuildAll.bat` (MSVC);
  EPICS includes from `os/win32`; packaging with WiX
  (`caQtDM_Viewer/package/windows*`). The GitHub CI
  (`build-windows-package.yml`) builds x64 only. Windows-on-ARM (ARM64)
  is NOT a supported target of this repo so far — it exists only as an
  internal cross-build experiment outside this repository.
- **iOS/Android** (`MOBILE`): without unit tests and without adl/edl
  conversion; plugins are imported statically (DEFINES
  BSREAD/EPICS4/ARCHIVE* in `qtdefs.pri`).

### macOS recipe 

- Qt `/usr/local/Qt-6.10.1/bin` in PATH (Homebrew Qt 6.11 lacks
  Qt5Compat/QTextCodec); Qwt 6.3.0 as a framework build:
  `QWTHOME=<qwt-6.3.0-dir>`,
  `QWTINCLUDE=$QWTHOME/lib/qwt.framework/Headers`, `QWTLIB=$QWTHOME/lib`,
  `QWTLIBNAME=qwt`.
- **No shadow build** — some sources (castripplot, cacartesianplot,
  caclock, …) contain `#include "moc_*.cpp"`; a shadow build produces
  duplicate symbols. Instead rsync the needed subtrees to /tmp and build
  in-tree there.
- Point `CAQTDM_COLLECT`/`QTCONTROLS_LIBS` at a temp collect dir; copy
  `libadlParser.*`/`libedlParser.*` from `caQtDM_Binaries/` there.
- Pitfalls: rsync -a preserves mtimes (after changing sources in the copy,
  `touch` them, otherwise make considers old objects current); never run
  `make` without the env (auto-qmake bakes empty QWT paths into the
  Makefile).
- Tests run headless: `QT_QPA_PLATFORM=offscreen
  DYLD_LIBRARY_PATH=<collect> DYLD_FRAMEWORK_PATH=$QWTLIB
  ./tst_qtcontrols`.

## Logging (important for new functionality)

Two separate levels — do not mix them up:

1. **Developer/operations logging: Qt logging categories.** New code logs
   with `qCDebug/qCInfo/qCWarning(<category>)`, NOT with bare `qDebug()`.
   There is NO `caQtDM_Log` macro — logging goes exclusively through the
   `qC*` macros on categories. Categories are declared
   (`Q_DECLARE_LOGGING_CATEGORY`) in the module's global header and
   defined in the respective module .cpp:
   `Q_LOGGING_CATEGORY(name, "caqtdm.<area>.<module>")`.
   Declaration headers per area:
   - `caQtDM_Lib/src/caQtDM_Lib_global.h` — lib and web categories
     (web ones under `#ifdef WEB`)
   - `caQtDM_QtControls/src/qtcontrols_global.h` — widget categories
     (largest group, ~67)
   - `caQtDM_Plugins/caQtDM_Plugins_global.h` — plugin categories
   - `caQtDM_Viewer/src/loggingcategories.h` — viewer categories
   - `caQtDM_Parsers/prcParserSrc/prcparserdefs.h` — prc parser
   Naming scheme: `caqtdm.lib.*`, `caqtdm.widgets.*`, `caqtdm.viewer.*`,
   `caqtdm.web.*`, `caqtdm.plugins.*`, `caqtdm.parsers.*`,
   `caqtdm.logging.*` (the log handlers themselves), `caqtdm.extern.c`.
   For a new module, create its own category following this pattern.
   Caveat: widget categories used inside `caqtdm_lib.cpp` are defined
   there a second time (`#ifndef MOBILE` block, separate translation
   unit) — keep both strings identical when touching one of them.
   Category strings are user-visible via `QT_LOGGING_RULES` — renaming
   existing ones breaks users' logging rules (ask first).
2. **User-facing messages: the MessageWindow**
   (`caQtDM_Lib/src/MessageWindow.*`, dock "caQtDM Messages", max. 400
   lines). Filled thread-safely via `postMsgEvent()` (custom event);
   C code (EPICS callbacks) uses `C_postMsgEvent` from
   `messageWindowWrapper.h`. Messages the operator should see (channel
   errors, plugin loaded) belong here — developer diagnostics go to the
   categories.

Central infrastructure (`caQtDM_Viewer/src/logging/`):
`GeneralLogHandler::initialize()` is called in `caQtDM.cpp` (main),
installs the `qInstallMessageHandler` and dispatches every message as a
structured log record (UTC timestamp in ms, level, message,
file:function:line, category, PID) to the configured handlers (own
thread). The `CAQTDM_LOGGING_*` env vars below are RUNTIME configuration
(read at viewer startup) — they are not qtdefs.pri build switches:

- Selection via env `CAQTDM_LOGGING_HANDLERS` (comma-separated):
  `console` (default), `file`, `syslog` (Unix only), `logstash` (HTTP).
- Fine-tuning via `CAQTDM_LOGGING_FILE_*` (COUNT/SIZE/BUFFER_*) and
  `CAQTDM_LOGGING_LOGSTASH_*` (URL/BUFFER_*); `logging/example.conf` is an
  example Logstash server configuration.
- `CAQTDM_LOGGING_INCLUDE_MESSAGEWINDOW` additionally feeds the
  MessageWindow entries into the log system.

**Level behavior (the most important rule):** `initialize()` sets
`*.debug=false` as the default — `qCDebug` is invisible in normal
operation and costs nothing; it only becomes visible via
`QT_LOGGING_RULES` / `QT_LOGGING_CONF`, e.g.
`QT_LOGGING_RULES="caqtdm.lib.cahmi.debug=true"`.
Info/Warning/Critical/Fatal always remain visible. So be generous with
debug output in new code, but always on categories — never stderr printf
for diagnostics.

Embedding special case: the extern-C entry point `caQtDM_Create()`
(Python/library use, `caqtdm.py`) installs the simple stderr handler
`myMessageOutput` instead — the last installed handler wins; structured
logging only applies to the normal viewer start.
Branch divergence note: the category/handler system is too new for the
`Release` branch — it does NOT exist there yet; Release logs with plain
`qDebug()` + prefix. When backporting Development code to Release, no
`qCDebug(<category>)` calls may travel along.

## Development workflow (the three branches)

- **`Development`** — development happens here, pull requests target this
  branch. If changes are too extensive for a direct PR, a feature branch
  is forked off Development and later merged back into it.
- **`Release`** — new versions are prepared here: merge from Development
  into Release, then packaging work usually begins. Problems on the
  individual OS platforms are fixed here; the tags that get set show what
  is already moving toward production internally. The big runtime tests
  usually start in January and run for at least half a year. Bugs found
  are fixed directly on Release; depending on severity, bugfixes travel
  via **cherry-pick in both directions** between Release and Development.
- **`master`** — the final version. Only when the runtime tests pass
  without errors/problems is Release merged into master and the final
  version tag assigned. NO development happens on master.

Consequence for work here: feature work belongs on Development (or a
branch off it); fixes that surface during the test phase land on Release
first and may need to be carried back to Development.

## Ongoing work / branches 

- **`feature/internalChannel`**: internal plugin with caSTRING support;
  intended to replace softPVs in the long run. Core is already on
  Development (`caQtDM_Plugins/internal/internal_channel.{h,cpp}`):
  simulated channels `internal://NAME[.FIELD]`, configured via JSON with
  EPICS field names (assembled by the genSoftPV widget). Each channel
  stores its value in a `NativeValue` struct in the channel's native
  EPICS type — int16/int32/float/double/enum/char, selected by JSON
  `type` (double/float/int/long/enum/string/char) into `fieldtype`; only
  the member matching `fieldtype` is used, and writes get the native
  type's truncation/wraparound (like the epics3 plugin). caSTRING
  channels generate strings from a regex pattern; `doubleValue` then
  holds the enumeration index.
- **Paused:** caCalc `%TimeStamp(A)%` -> string into an internalPV (fully
  planned, not yet implemented). Key decisions: no controlsinterface.h
  change, the calc expression is the routing switch, genSoftPV (dataType
  String) defines the channel, format local ISO 8601.
- **HMI event bus** (`caQtDM_Lib/src/hmisharedeventbus.*`): hardened
  2026-07 on the Release branch (locked copy-then-emit, 10-minute cleanup
  with lowest-PID election, slot ownership check). Cleanup mechanics
  (`performCleanupCheck`): every `CLEANUP_INTERVAL_MS` (10 min) the
  lowest registered pid is elected as cleaner (no OS liveness check); a
  cleanup announces itself via `CleanupStarted` and waits
  `CLEANUP_GRACE_MS` (150 ms) so peers can poll it before the ring is
  wiped. Safety net for a dead cleaner: peers take over once no cleanup
  was observed for `2*CLEANUP_INTERVAL_MS +
  slotIndex*4*CLEANUP_GRACE_MS` — staggered per slot so concurrent
  takeovers stay rare; an observed `CleanupStarted` resets the clock.
  SHM key: `caQtDM_HmiSharedEventBus_SharedMem_<uid>`. Locking uses
  exclusively
  `QSharedMemory::lock()` (never mix in QSystemSemaphores!). **No
  layout/key changes without asking** — the user explicitly rejected key
  versioning. When adding event types, update `LAST_EVENT_TYPE`; never
  hold both SHM locks (bus + config list) at the same time.

## Packaging / CI

- `caQtDM_Viewer/package/` contains all variants: `redhat`/`rhel`/`sl6`
  (RPM), `debian` (DEB), `appimage`, `archlinux`, `flatpak`, `brew`
  (Homebrew), `windows` + `windows_wix7` (WiX MSI).
- RPM specs/patches (`package/redhat/`: caqtdm.spec, epics-base.spec +
  patches) and `package/debian/` feed the GitHub CI and external
  packaging pipelines alike — changes take effect everywhere.
  Current versions: caQtDM 4.6.1, EPICS 7.0.10, Qwt 6.3.0.

### Homebrew / macOS bundle

- `package/brew/`: own formulas `caqtdm.rb` + `epicsbase.rb`, generated
  via `create-brew.sh`, cleanup with `clean(brew).sh`.
- The Homebrew build sets `HOMEBREW_MAKE_JOBS` — as a result the unit
  tests are NOT built (switch in `qtdefs.pri`).
- Pitfall: current Homebrew Qt (6.11) no longer has Qt5Compat/QTextCodec —
  use a self-built Qt for local builds (see macOS recipe).
- App bundle: `caQtDM_BuildMacBundle` collects `caQtDM.app` from the
  collect dir, copies Qt plugins (platforms/imageformats/printsupport) and
  calls `macdeployqt` with `-libpath` for EPICS/Qwt/ZMQ. Caution: the
  script contains absolute local paths (EPICS, zmq build) — check/adjust
  before use. Windows counterparts for signing/packaging:
  `caQtDM_Build_Package.bat` / `caQtDM_Build_Sign.bat`.

### Windows packaging (TWO variants: WiX 3 and WiX 7)

- **`package/windows/` — WiX 3 (candle/light), established.** Separate
  sources per target: `caQtDM.wxs` (historic Qt4/32bit), `caQtDM_x64.wxs`
  (Qt5/Qt6 x64), `caQtDM_x86.wxs`; includes `caQtDM_Qt5_x64.wxi` /
  `caQtDM_Qt6_x64.wxi` / `caQtDM_Version.wxi`. Invoked via the root
  script `caQtDM_Build_Package.bat`: target selection via `SELCTION` from
  `caQtDM_Env.bat` (1 = Qt4/32, 2 = Qt6/x64, 3 = Qt5/x64), `WIXHOME` must
  be in PATH; the MSI is copied to `%CAQTDM_COLLECT%` and signed with
  signtool (`CAQTDM_SIGNER`/`TIMESTAPER`).
  **The GitHub CI (`build-windows-package.yml`) uses THIS variant**
  (candle/light on `caQtDM_x64.wxs`).
- **`package/windows_wix7/` — WiX 7 (.NET tool `wix`), newer.** ONE
  parameterized source `caQtDM.wxs` + `caQtDM_Qt6.wxi`;
  `build_package.bat` loads the matching `caQtDM_Env.bat` profile itself,
  checks mandatory variables (QTHOME, EPICS_BASE, EPICS_HOST_ARCH,
  CAQTDM_COLLECT, ZMQBIN/ZMQLIBRARY, QWTHOME, SSLBIN/LIBSSL/LIBCRYPTO)
  and aborts if any are missing. The MSI version is read from the
  FILEVERSION of caQtDM.exe (which gets it via caQtDM.rc from
  qtdefs.pri — it can never diverge from the binary). Build in
  `project_<arch>/`, result `caQtDM_<maj>_<min>_<patch>_<arch>.msi` in
  `%CAQTDM_COLLECT%`; signing optional (same variables as WiX 3).
  The script also accepts an `arm64` parameter — that path belongs to an
  internal experiment and is not a supported target yet (see below).
- For both variants: the .wxs files list ALL files explicitly (no
  if-exists) — new binaries/plugins must be added explicitly, otherwise
  they are missing from the MSI or the build breaks.

### Windows on ARM (ARM64) — status

Not a supported target of this repository so far. An ARM64 cross build
(EPICS, Qt/Qwt, caQtDM, MSI) has only been done as an internal
experiment in an external CI pipeline; nothing of it is part of the
upstream GitHub CI. Only traces in this repo: the `arm64` parameter in
`windows_wix7/build_package.bat`. Do not present ARM64 as a supported
platform.

### Android / iOS

- `MOBILE` mode (qtdefs.pri, `ios | android`): panels are downloaded via
  HTTP into a local directory instead of read from the file system (start
  option `-httpconfig` shows the configuration page;
  `caQtDM_IOS_Config.xml` as example config). No unit tests, no adl/edl
  conversion (only .ui), control-system plugins imported statically
  (DEFINES BSREAD/EPICS4/ARCHIVE* in fileopenwindow).
- **Android:** CI is `.github/workflows/build-android.yml` (daily + PR):
  Qt 6.11.1 is built from source (host `gcc_64` + `android_arm64_v8a`),
  OpenSSL 3.5.5 (own SSL API level 28), Qwt 6.3, EPICS R7.0.10, NDK 27.2,
  Android API 36, ABI arm64-v8a; switches CAQTDM_NORPATH/GPS/MODBUS/OPCUA
  = 1. Manifest/Gradle/resources live under `caQtDM_Viewer/src/Android/`.
- **iOS:** no CI workflow — built locally (Qt for iOS + Xcode).
  Assets/Info.plist under `caQtDM_Viewer/src/IOS/`; parser libs for iOS in
  the top-level folder `caQtDM_Binaries_IOS/`; qjpeg/qgif are linked in
  statically as QTPLUGIN. Note at the top of `all.pro`: for older Qt
  versions `mkspecs/features/toolchain.prf` had to be extended with
  `cxx_flags += -arch $$QMAKE_APPLE_DEVICE_ARCHS`, otherwise even parsing
  the .pro files fails.

## caQtDM_Web & qnovnc platform plugin

- caQtDM_Web is the browser variant: caQtDM runs headless in a container
  and renders via the QPA plugin **qnovnc** directly over WebSocket to a
  noVNC client in the browser (`caQtDM_Web/user/noVNC`, git submodule;
  fronted by nginx or Caddy as reverse proxy).
- The plugin is a **separate repo**:
  https://github.com/caqtdm/qnovnc-platform-plugin.git — CMake build
  (`cmake -B build -S . && cmake --build build --target install`),
  produces `libqnovnc.so` in `<qt>/plugins/platforms/`. Changes to the
  web rendering may therefore be needed there, not in the caQtDM tree.
- caQtDM is built with `CAQTDM_WEB=1` for this; container start:
  `caQtDM -server -novnc -host 0.0.0.0 <panel.ui>` (selects the qnovnc
  platform plugin).
- Two Docker images (`caQtDM_Web/docker/`): `caqtdm` = Alpine multistage
  (EPICS + caQtDM + qnovnc plugin, build context repo root), `nginx` =
  web content (context `caQtDM_Web/`, needs the user/admin submodules).
  Reference CI: `.github/workflows/build-caqtdm_web-docker-image.yml`.

## Documentation (caQtDM_docs)

- Sphinx project (`caQtDM_docs/`, theme `pydata_sphinx_theme`); the
  content is essentially ONE large file: `source/caQtDM_Manual.rst`
  (~4200 lines, the complete manual — widgets, attributes, start
  options). New features/widgets are documented there, not in separate
  files.
- Build locally: `make html` in `caQtDM_docs/` (needs `sphinx` +
  `pydata-sphinx-theme`; TeX Live/dvipng for math rendering), output in
  `caQtDM_docs/build/html`.
- Publishing: `.github/workflows/docs-deployment.yml` deploys manually
  (workflow_dispatch) to GitHub Pages.
- Open documentation state: internalPVs are not yet described in the
  manual (docs are being written separately); the paused caCalc
  %TimeStamp% plan only foresees a small snippet for the calc list.

## Working rules (repo-specific)

- Git read-only for agents; the user frequently builds/tests himself.
- `caqtdm_lib.cpp` is huge — work with grep/partial reads instead of
  reading it whole.
- Keep code comments short; longer explanations belong in the plan, not
  in the code.

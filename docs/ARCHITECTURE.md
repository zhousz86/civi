# CIVI front-end architecture

## 1. Where the front-end sits

```
 Snapdragon cockpit SoC (SA8295P / SA8775P class)                       zonal Ethernet
 ┌───────────────────────────────────────────────────────────────────┐        │
 │ Hypervisor (QNX Hypervisor, or BSP-provided)            CIVI-SW-001│        │
 │ ┌──────────────── Cluster partition ────────────────┐ ┌─ Infotainment partition ─┐
 │ │ QNX OS for Safety 8.0                             │ │ QNX SDP 8.0 (demo)        │
 │ │  civi-telltaled  (C, CPU-rendered, top z-order)   │ │  civi-center              │
 │ │  civi-cluster    (Qt Quick)                       │ │  civi-passenger           │
 │ │  civi-hud        (Qt Quick)                       │ │  → or Linux IVI / AAOS    │
 │ └───────────────┬───────────────────────────────────┘ └───────────┬──────────────┘
 │                 │  PPS objects /pps/civi/... (read state, write requests)          │
 │ ┌───────────────┴──────── Vehicle-services partition ─────────────┴──────────────┐ │
 │ │ vehicle-services server (authorises every write)  ── CAN/SOME-IP/DDS gateways  │─┘
 │ └────────────────────────────────────────────────────────────────────────────────┘
 │ ┌──────────── Connectivity / TCU partition (hardened) ───────────────────────────┐
 │ │ multi-orbit terminal: LEO · MEO(GNSS) · GEO ; policy engine ; NG911 ; Wi-Fi AP │
 │ └────────────────────────────────────────────────────────────────────────────────┘
 └────────────────────────────────────────────────────────────────────────────────────┘
```

This repository is everything **above** the PPS line: four display processes, the
safety telltale renderer, the shared platform-service library, and the tooling
(simulator, release gates). Everything below the line is represented by its contract
(`docs/VEHICLE_SERVICES_API.md`) and simulated by `tools/vsim`.

## 2. Design decisions and why

| Decision | Rationale | Requirements |
|---|---|---|
| **One process per display**, each its own Screen window | A crash, hang or update of one display cannot blank another; processes restart independently and resynchronise from PPS | PLT-001, PLT-002, SW-008 |
| **Telltales in a separate C process**, CPU-rendered into a Screen buffer at the highest z-order | No Qt, no GPU, no shader compiler in the telltale path; survives infotainment GPU faults; small enough to carry into a safety argument (MISRA-friendly subset) | PLT-001, PLT-002, PLT-003, HMI-008 |
| **Heartbeat supervision** in the telltale renderer | A frozen source lights the CHECK lamp rather than showing stale-but-plausible telltales | PLT-002 |
| **PPS as the vehicle-services transport** | Native to QNX, file-permission access control (read state / write requests only), no extra middleware in the boot path; the same text format is simulated by plain files on Linux | SW-002, SW-004 |
| **Platform services in Qt Core only** (`platform/`) | Reusable behind a Kanzi, Linux-IVI or Android Automotive front-end through a thin binding; keeps the ecosystem decision open | SW-008, §12.1 "Infotainment ecosystem" |
| **Single-source bilingual catalogue** (`i18n/catalog.json`) served by a custom `QTranslator` | French and English cannot drift; one id per string; a missing translation is visible and caught by the CI gate; language switches cockpit-wide in one frame | HMI-001, SW-005, VAL-002, SEC-011 |
| **Platform-enforced distraction policy** (`Policy` singleton) | Apps bind to `Policy.textEntryAllowed` etc.; nothing decides on its own; invalid/stale motion ⇒ locked (fail-safe) | HMI-006, SW-003 |
| **One alert arbiter, identical in every process** | Deterministic priority: takeover > safety warning > emergency > public alert > connectivity > info; takeover and warnings are not dismissible from the HMI | HMI-008, AVR-001, EMG-007 |
| **Takeover latency measured on the presented frame** | Armed at scene sync, stamped at swap of the same frame, compared with the publisher's CLOCK_MONOTONIC stamp; published to `hmi/metrics_<role>` for test rigs | AVR-001 (TBD-11) |
| **Every TBD is configuration** (`config/civi-config.json`, `config/policy/`) | OEM closes values without a software change | DAT-009, §12.2 |
| **Offline-first UI state** | Every model holds cached state and detects staleness; loss of link produces a bilingual "what still works" banner, never a frozen screen | CON-016, CON-019, P4 |

## 3. Code map

```
platform/            Qt-Core services (no UI)
  PpsChannel           PPS transport (QNX ?delta + QSocketNotifier / host inotify)
  VehicleServices      motion, state, telltales mirror, warnings, AV/takeover, requests
  DistractionPolicy    lockout with hysteresis, fail-safe on invalid motion
  HmiSettings          session language, units, accessibility; Quebec ⇒ French default
  CatalogTranslator    single-source FR/EN with singular forms
  AlertArbiter         priority arbitration, media pause, TTS requests, takeover latency
  ConnectivityModel    LEO/MEO/GEO/terrestrial status, routing, emergency bearer, Wi-Fi
  EmergencyService     SOS / automatic crash notification state and requests
  ConsentManager       per-purpose, per-profile consent from policy file; factory reset
  OtaService           update state, release gates, install preconditions
  NavigationModel      guidance, offline readiness, predicted sky-view gaps
  MediaModel           AM/FM tuner, blend, stations
hmi/runtime/         process bootstrap, QML singletons (import CIVI.Platform), display placement
hmi/controls/        CIVI.Controls: Theme, Fmt, glove-sized controls, overlays
hmi/{cluster,center,passenger,hud}/   one Qt Quick app per display
safety/civi-telltaled/                C telltale renderer (QNX Screen / host console)
tools/vsim/          vehicle + TCU simulator with demo scenarios
tools/i18n/          French-parity release gate
tools/sbom/          CycloneDX SBOM + US BIS provenance gate
deploy/qnx/          start-up order, IFS fragment, security-policy sketch
```

## 4. Process start order on target

1. BSP: `screen`, `pps`, early rear-view camera (Qualcomm AIS) — not in this repo.
2. `civi-telltaled` from the IFS — telltales lit (CHECK until first heartbeat).
3. `civi-cluster`, `civi-hud` — cluster partition, higher priority.
4. `civi-center`, `civi-passenger` — infotainment.

See `deploy/qnx/civi-startup.sh`.

## 5. Porting the centre/passenger displays to another ecosystem

* **Linux IVI guest**: build the same tree with a Linux toolchain; replace the PPS
  transport in `PpsChannel` with the guest's IPC (virtio-vsock or SOME/IP to the
  vehicle-services partition). QML and services are unchanged.
* **Android Automotive**: the contract maps to VHAL properties and the consent/alert
  logic to system services; the catalogue converts to `strings.xml` (fr-CA, en-CA)
  from the same JSON source. Cluster, HUD and telltales stay on QNX either way.

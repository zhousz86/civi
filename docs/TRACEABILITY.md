# Requirement traceability — CIVI front-end draft v0.1

Source: *CIVI High-Level System Requirements v1.1* (Nov 10, 2026). This table states
honestly what the front-end code base covers today.

**Legend**
✅ implemented in this repo and covered by a test or gate ·
🟡 implemented on the HMI side; completion depends on another component or on hardware measurement ·
🔌 owned by another domain; this repo defines/consumes the interface ·
⬜ outside front-end scope, or not yet started (gap)

## Platform and compute
| ID | Status | Where / evidence |
|---|---|---|
| PLT-001 | 🟡 | Separate processes per display; telltales on a CPU-rendered Screen window (`safety/civi-telltaled`). CPU/memory-bandwidth/GPU partitioning is hypervisor + BSP configuration |
| PLT-002 | ✅ | Telltale renderer independent of Qt/GPU; heartbeat fail-safe (`tst_telltale`); display restart re-shows active takeover/warnings (`tst_platform::takeover_visible_after_restart`) |
| PLT-003 | 🟡 | Start order + IFS fragment (`deploy/qnx`); CMVSS timing to be measured on hardware at −40 °C |
| PLT-004 | ⬜ | Hardware selection |
| PLT-005 | 🟡 | HMI first frame is ready as soon as Screen is; panel heating/OLED is hardware |
| PLT-006 | 🔌 | Wake-policy engine is power management; HMI explains the budget (climate page) |
| PLT-007 | ⬜ | Analysis — footprint to be measured on target |
| PLT-008 | ✅ | AV status + takeover on cluster and HUD from `vs/av` — no hardware change |

## Software platform and vehicle-services API
| ID | Status | Where / evidence |
|---|---|---|
| SW-001 | 🔌 | Partition mapping in `docs/ARCHITECTURE.md`; hypervisor config is BSP/OEM |
| SW-002 | ✅ | `VehicleServices`: apps read state, write only request objects; PPS permissions + `secpol` sketch |
| SW-003 | ✅ | Motion state + AV mode/takeover signals in `VehicleServices` |
| SW-004 | 🟡 | Contract table `docs/VEHICLE_SERVICES_API.md`; staleness handling implemented for motion, TCU status and telltales |
| SW-005 | ✅ | `HmiSettings` + `Fmt` (units, fr-CA/en-CA formats, voice locale); cockpit-wide switch |
| SW-006 | ⬜ | **Gap**: CarPlay / Android Auto need licensed stacks (Apple/Google certified); slot planned on centre display |
| SW-007 | ✅ | `tools/sbom/generate_sbom.py` (CycloneDX with supplier + origin) — ctest `sbom_provenance` |
| SW-008 | 🟡 | Infotainment processes replaceable; services Qt-Core only for ecosystem portability |

## HMI, bilingual voice and distraction
| ID | Status | Where / evidence |
|---|---|---|
| HMI-001 | ✅ | `i18n/catalog.json` single source (French authored first); ctest `french_parity` |
| HMI-002 | 🟡 | Wrapping titles/rows; `tools/screenshots.sh` renders every screen in both languages; 62 strings exceed +30 % and are flagged for layout review |
| HMI-003 | ✅ | `HmiSettings::defaultLanguage` — `tst_platform::quebec_defaults_to_french` |
| HMI-004 | 🔌 | Speech engine (vendor TBD); voice locale published with every TTS request |
| HMI-005 | 🔌 | TTS requests via `audio/tts_request`; on-device ASR/TTS engine TBD |
| HMI-006 | ✅ | `DistractionPolicy` (threshold TBD-7 in config, hysteresis, fail-safe) — `tst_platform::distraction_policy`; text entry/lists/video bound to it |
| HMI-007 | ⬜ | Driver studies (VAL-004) |
| HMI-008 | ✅ | `AlertArbiter` priority + opaque, non-dismissible warning/takeover overlays below a telltale strip that HMI cannot draw into — `tst_platform::alert_priority` |
| HMI-009 | 🟡 | Video only on the passenger process; optical privacy is hardware |
| HMI-010 | 🟡 | Large text, high contrast, haptics flag, voice rate implemented in both languages; screen reader needs a TTS engine |
| HMI-011 | 🟡 | 88 px minimum targets (`Theme.touchMin`); glove test on hardware |

## Broadcast, emergency, public alerting
| ID | Status | Where / evidence |
|---|---|---|
| EMG-001 | ✅ | AM/FM tuner UI (`MediaPage`) |
| EMG-002 | 🟡 | Blend indicator; blending is in the tuner |
| EMG-003 | ⬜ | Certification |
| EMG-004 | ✅ | ACN flow and notices (`EmergencyService`, `EmergencyPage`) — detection is in the airbag ECU/TCU |
| EMG-005 | 🔌 | NG911 call is TCU; HMI shows voice/location/crash-data status |
| EMG-006 | 🟡 | GEO bearer shown; carriage is TCU |
| EMG-007 | ✅ | Public alerts from `alerts/public` shown and spoken in session language, media paused — `tst_platform::alert_priority` |
| EMG-008 | ✅ | `ConnectivityModel::selectEmergencyBearer` mirrors GEO→LEO→terrestrial — `tst_platform::emergency_bearer`; "move to open sky" guidance when none |
| EMG-009 | ⬜ | Onboard cell (planned) |

## Connectivity plane
| ID | Status | Where / evidence |
|---|---|---|
| CON-001 | 🟡 | LEO/MEO/GEO status UI; terminal management is TCU |
| CON-002…006 | 🔌 | TCU/terminal; HMI displays throughput, GNSS fix/dead reckoning, GEO elevation |
| CON-007 | 🟡 | Terrestrial shown as optional / "not fitted" |
| CON-008 | ✅ | No front-end function depends on terrestrial or projection (offline scenario `tunnel`) |
| CON-009, -010 | 🔌 | AP and isolation are TCU/network |
| CON-011 | ✅ | Owner Wi-Fi on/off and data cap, bilingual (`ConnectivityPage`) |
| CON-012 | 🔌 | Routing shows Wi-Fi bearer for bulk |
| CON-013, -014 | ⬜ | Onboard cell |
| CON-015 | 🟡 | Per-class routing displayed; policy engine is TCU |
| CON-016 | ✅ | Navigation, voice, emergency, settings independent of link; offline banner |
| CON-017 | 🟡 | Queue size displayed; queueing is TCU |
| CON-018 | 🟡 | Predicted sky-view gaps + prefetch status displayed |
| CON-019 | ✅ | Stale-status detection, bilingual degraded state, no blocking calls |
| CON-020 | ⬜ | V2X |

## Data, privacy, sovereignty
| ID | Status | Where / evidence |
|---|---|---|
| DAT-001 | ✅ | `ConsentManager`: per purpose, per profile, immediate in-vehicle revocation |
| DAT-002 | ✅ | All purposes default off (`consent-policy.json`) |
| DAT-003 | 🟡 | Residency from configuration and shown to the owner; routing is backend |
| DAT-004, -005, -008 | ⬜ | Process / backend |
| DAT-006 | ✅ | Factory-reset request with confirmation, locked while moving |
| DAT-007 | ✅ | Consents stored per profile |
| DAT-009 | ✅ | Policy file + config — `tst_platform::config_tbd_values` |

## Security, OTA, provenance, compliance
| ID | Status | Where / evidence |
|---|---|---|
| SEC-001, -003, -004, -006, -009 | ⬜ | Platform/backend (QNX secure boot, Qualcomm HW RoT, IDS, PKI, rollout) |
| SEC-002, -005 | 🟡 | `deploy/qnx/secpol/civi.secpol` sketch; PPS permission model |
| SEC-007 | 🟡 | Hardening flags in the QNX toolchain file; TARA not started |
| SEC-008 | ✅ | `OtaService::evaluate` — `tst_platform::ota_preconditions` |
| SEC-010 | ✅ | SBOM per build in CI |
| SEC-011 | ✅ | French-parity gate in CI + `parity_ok` install gate |
| SEC-012 | ✅ | `generate_sbom.py --market US` restricted-origin check; `--release` fails on TBD suppliers |

## AV readiness
| ID | Status | Where / evidence |
|---|---|---|
| AVR-001 | ✅ | Bilingual, pre-emptive takeover on cluster + HUD; on-frame latency measurement vs `takeover.max_latency_ms` (TBD-11) |
| AVR-002, -005 | ⬜ | Connectivity / pipeline |
| AVR-003 | 🔌 | Spoken takeover via TTS request |
| AVR-004 | 🟡 | `av_data_loop` consent purpose |

## Verification and validation
| ID | Status | Where / evidence |
|---|---|---|
| VAL-001, -004 | ⬜ | Climatic chamber / driver studies |
| VAL-002 | ✅ | Automatic untranslated-string scan in CI |
| VAL-003 | 🟡 | `tools/vsim` scenarios (tunnel, GEO→LEO fallback, offline) for lab runs on target |
| VAL-005 | 🟡 | Tests named by requirement; CI uploads screenshots + SBOM as evidence |

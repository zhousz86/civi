# Vehicle-services contract (civi-vs/1.0)

Every interface defines content, timing, validity, degradation behaviour, versioning
and locale (CIVI-SW-004). PPS root: `/pps/civi` on target, `$CIVI_PPS_ROOT` on host.
Encoding per attribute: `name:enc:value`, enc = `` (string) / `n` / `b` / `json`.

**Access rule (CIVI-SW-002):** HMI processes have read access to state objects and
write access only to `*request*` objects and HMI-owned objects (`hmi/*`). The owning
server validates and authorises every request; the HMI never assumes a request took
effect until the state object changes.

| Object | Owner | Content | Timing | Validity / degradation | Locale |
|---|---|---|---|---|---|
| `vs/motion` | vehicle services | `speed_kph:n`, `gear` (P/R/N/D), `valid:b`, `ts_mono_ns:n` | 10–50 Hz | stale after `distraction.motion_signal_timeout_ms` (500) ⇒ treated as moving (lockout) | n/a (SI) |
| `vs/state` | vehicle services | `soc_pct`, `range_km`, `ambient_c`, `cabin_c`, `odometer_km`, `climate_setpoint_c`, `seat_heat`, `defrost:b` | on change, ≤1 Hz | last value shown | SI; HMI converts |
| `vs/telltales` | vehicle services (safety path) | flat `<lamp>:b`, `seq:n` heartbeat | 10 Hz heartbeat | renderer lights CHECK if `seq` stalls > 300 ms | symbols only |
| `vs/warnings` | vehicle services | `active:json [{id, code, severity}]` | on change | clears only from source | `code` → catalogue `civi.warning.<code>` |
| `vs/av` | AV stack via vehicle services | `mode` (manual/l2/l3/takeover), `takeover_id`, `takeover_reason`, `takeover_deadline_ms`, `ts_mono_ns` | event | new id = new request; empty id = cleared | `reason` → `civi.takeover.reason.<r>` |
| `vs/request` | **HMI writes** | `climate_setpoint_c`, `seat_heat`, `defrost`, `api_version`, `ts_ms` | user action | server may reject | — |
| `tcu/links` | TCU | `leo`, `meo`, `geo`, `terr` (json: `state` up/degraded/blocked/down/off, metrics), `routing:json`, `queue_mb`, `sky_view:b`, `hb` | 1 Hz | stale after 3 s ⇒ "status unknown" banner | — |
| `tcu/emergency` | TCU | `state`, `kind` (sos/acn), `bearer`, `location_sent:b`, `lat`, `lon`, `cancel_until_ms` | event | — | — |
| `tcu/emergency_request` | **HMI writes** | `action` (sos/cancel) | user action | cancel honoured only while connecting | — |
| `alerts/public` | TCU (NPAS/Alert Ready) | `active:json [{id, kind, severity, area, headline_fr, headline_en, body_fr, body_en}]` | event | removed from list ⇒ cleared | **both languages required** |
| `wifi/status`, `wifi/request` | TCU / **HMI writes** | `enabled`, `clients`, `used_mb`, `cap_mb` | 1 Hz / user | — | — |
| `nav/guidance`, `nav/request` | nav engine / **HMI writes** | guidance, `offline_ready`, `cached_region`, `sky_gaps:json` | 1 Hz | offline-first | place names as provided |
| `media/state`, `media/request` | audio manager / **HMI writes** | tuner + stream state, `stations:json` | on change | — | — |
| `ota/status`, `ota/request` | OTA client / **HMI writes** | state, version, progress, `notes_fr`, `notes_en`, gates `signature_ok`, `sbom_ok`, `parity_ok` | on change | install refused unless all gates + preconditions pass | both languages required |
| `privacy/consent`, `privacy/request` | privacy service / **HMI writes** | `profile_<id>:json`, actions set/factory_reset | user action | revocation effective in vehicle immediately | — |
| `audio/tts_request`, `audio/focus_request` | **HMI writes** (centre only) | text, locale (fr-CA/en-CA), rate; `pause_media` | event | — | session language |
| `hmi/settings` | **HMI** | `language`, `units`, accessibility flags | user action | shared by all displays | — |
| `hmi/metrics_<role>` | **HMI** | `takeover_latency_ms`, `takeover_budget_ms` | per takeover | for test rigs | — |

Versioning: `api_version` is stamped on every request; servers reject unknown major
versions. Adding attributes is minor; renaming/removing is major.

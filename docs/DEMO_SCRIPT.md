# Customer demo script (≈ 12 minutes)

Run `tools/run-desktop.sh full-demo` (laptop) or the `full-demo` scenario on the
bench. The simulator prints each step; talk track below.

| t (s) | On screen | Talking point | Requirement |
|---|---|---|---|
| 0 | Parked in Montréal, −18 °C, French by default | "Configured for Quebec, it boots in French. One tap switches every display, voice and alert to English." Tap **EN**, then **FR** | HMI-001, HMI-003, SW-005 |
| 0 | Updates page | "An update installs only if signature, provenance and *French parity* gates passed, and the car is parked with enough battery." | SEC-008, SEC-011 |
| 6 | Winter drive, 100 km/h | Search field becomes voice-only; "Available when parked" | HMI-006 |
| 12 | Black ice | ESC telltale + slippery-road warning pre-empts guidance on the cluster | HMI-008 |
| 24 | Tunnel | All three orbits blocked; navigation continues on dead reckoning; telemetry queued; "Offline — navigation, voice, emergency still work" | CON-016, CON-017, CON-019 |
| 30 | Tunnel exit | LEO returns first: emergency path GEO → **LEO** fallback; then GEO | EMG-008 |
| 38 | Automated driving | AV status on cluster | PLT-008 |
| 43 | Takeover request | Bilingual, pre-emptive, on cluster *and* HUD; latency measured on the presented frame (Settings → About) | AVR-001 |
| 55 | Wildfire alert | Alert Ready message in session language, spoken, media paused | EMG-007 |
| 68 | Crash | Automatic crash notification over GEO with location; NG911-ready | EMG-004…006 |
| any | Kill the centre display process | Cluster and telltales keep running; restarting it shows any active alert at once | PLT-002 |

#!/usr/bin/env python3
"""CIVI vehicle / TCU simulator — drives every display through the same PPS contract
the real vehicle-services server and TCU publish on target.

Host:   writes PPS-format files under $CIVI_PPS_ROOT (default /tmp/civi-pps).
Target: point CIVI_PPS_ROOT at /pps/civi on a QNX bench (python3 from the QNX
        package set) to run the same scenarios on Snapdragon hardware without a car.

  civi_vsim.py --scenario full-demo          # narrated customer demo, loops
  civi_vsim.py --scenario takeover --until 6 # play to t=6 s, then hold that state
  civi_vsim.py --list
"""
import argparse, copy, json, os, sys, tempfile, time

ROOT = os.environ.get("CIVI_PPS_ROOT", "/tmp/civi-pps")


# ---------------------------------------------------------------- PPS I/O
def enc(v):
    if isinstance(v, bool):
        return "b", "true" if v else "false"
    if isinstance(v, (int, float)):
        return "n", repr(float(v)) if isinstance(v, float) else str(v)
    if isinstance(v, (dict, list)):
        return "json", json.dumps(v, separators=(",", ":"), ensure_ascii=False)
    return "", str(v).replace("\n", " ")


def write_obj(obj, attrs):
    path = os.path.join(ROOT, obj)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    lines = ["@" + os.path.basename(obj)] + [f"{k}:{enc(v)[0]}:{enc(v)[1]}" for k, v in attrs.items()]
    fd, tmp = tempfile.mkstemp(dir=os.path.dirname(path))
    with os.fdopen(fd, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    os.chmod(tmp, 0o644)
    os.replace(tmp, path)


def read_obj(obj):
    out = {}
    try:
        with open(os.path.join(ROOT, obj), encoding="utf-8") as f:
            for line in f:
                line = line.rstrip("\n")
                if not line or line.startswith("@"):
                    continue
                k, e, v = line.split(":", 2)
                out[k] = float(v) if e == "n" else (v == "true") if e == "b" else json.loads(v) if e == "json" else v
    except (FileNotFoundError, ValueError):
        pass
    return out


# ---------------------------------------------------------------- world state
STATIONS = [
    {"name": "ICI Radio-Canada Première", "freq": "95,1", "band": "fm"},
    {"name": "CBC Radio One", "freq": "88,5", "band": "fm"},
    {"name": "CKOI", "freq": "96,9", "band": "fm"},
    {"name": "Rouge FM", "freq": "107,3", "band": "fm"},
    {"name": "CHOM", "freq": "97,7", "band": "fm"},
    {"name": "ICI Musique", "freq": "100,7", "band": "fm"},
    {"name": "98,5 Montréal", "freq": "98,5", "band": "fm"},
    {"name": "CJAD", "freq": "800", "band": "am"},
    {"name": "CKAC", "freq": "730", "band": "am"},
]

BASE = {
    "vs/motion": {"speed_kph": 0.0, "gear": "P", "valid": True},
    "vs/state": {"soc_pct": 82.0, "range_km": 412.0, "ambient_c": -18.0, "cabin_c": 19.5, "odometer_km": 12480.0,
                 "climate_setpoint_c": 21.5, "seat_heat": 2, "defrost": False},
    "vs/telltales": {"turn_left": False, "seatbelt": False, "brake": False, "abs": False, "airbag": False,
                     "esc": False, "tpms": False, "check": False, "high_beam": False, "turn_right": False},
    "vs/warnings": {"active": []},
    "vs/av": {"mode": "manual", "takeover_id": "", "takeover_reason": "", "takeover_deadline_ms": 0},
    "tcu/links": {
        "leo": {"state": "up", "quality": 88, "dl_mbps": 142, "ul_mbps": 18},
        "meo": {"state": "up", "fix": "3d", "sats": 21, "lat": 45.50884, "lon": -73.58781},
        "geo": {"state": "up", "elevation_deg": 31},
        "terr": {"fitted": False, "state": "off"},
        "routing": {"emergency": "geo", "safety": "geo", "telemetry": "leo", "ota": "leo",
                    "media": "leo", "passenger": "leo", "bulk": "wifi"},
        "queue_mb": 0.0, "sky_view": True},
    "tcu/emergency": {"state": "idle", "kind": "", "bearer": "", "location_sent": False, "lat": 0.0, "lon": 0.0, "cancel_until_ms": 0},
    "wifi/status": {"enabled": True, "clients": 3, "used_mb": 1840.0, "cap_mb": 5120.0},
    "nav/guidance": {"active": False, "destination": "", "maneuver": "straight", "maneuver_dist_m": 0, "street": "",
                     "eta_min": 0, "remaining_km": 0, "offline_ready": True,
                     "cached_region": "Grand Montréal · A-20 → Québec", "sky_gaps": []},
    "media/state": {"source": "fm", "station": "ICI Radio-Canada Première", "freq": "95,1",
                    "title": "Le 15-18", "artist": "", "blend": True, "playing": True, "stations": STATIONS},
    "alerts/public": {"active": []},
    "ota/status": {"state": "ready", "version": "2026.11.2", "size_mb": 640.0, "progress": 100.0,
                   "notes_fr": "Amélioration du démarrage par grand froid, nouvelles voix québécoises et correctifs de sécurité.",
                   "notes_en": "Faster cold-weather start-up, new Quebec voices and security fixes.",
                   "signature_ok": True, "sbom_ok": True, "parity_ok": True},
}

ROUTE = {"active": True, "destination": "Château Frontenac, Québec", "maneuver": "right", "maneuver_dist_m": 850,
         "street": "Autoroute 20 Est — sortie 312", "eta_min": 148, "remaining_km": 241,
         "sky_gaps": [{"kind": "tunnel", "name": "Tunnel Louis-H.-La Fontaine", "dist_km": 3.2, "prefetch": "done"},
                      {"kind": "canopy", "name": "Parc de la Jacques-Cartier", "dist_km": 188.0, "prefetch": "running"}]}

WILDFIRE = {"id": "npas-2026-1104", "kind": "wildfire", "severity": "extreme", "area": "Saguenay–Lac-Saint-Jean",
            "headline_fr": "Feu de forêt à proximité — évacuez vers le sud par la route 175",
            "headline_en": "Wildfire nearby — evacuate south via Route 175",
            "body_fr": "SOPFEU : le feu progresse vers l'autoroute 70. Suivez les consignes des autorités.",
            "body_en": "SOPFEU: the fire is moving toward Highway 70. Follow instructions from authorities."}


class World:
    def __init__(self):
        self.s = copy.deepcopy(BASE)
        self.seq = 0
        self.req_mtime = {}
        self.emg_t0 = None

    def set(self, obj, **kv):
        self.s[obj].update(kv)

    def link(self, name, **kv):
        self.s["tcu/links"][name].update(kv)

    def publish_all(self):
        for obj, attrs in self.s.items():
            if obj not in ("vs/motion", "vs/telltales"):
                write_obj(obj, attrs)

    def tick(self):
        """10 Hz: motion + telltale heartbeat (stale => cluster fail-safe)."""
        self.seq += 1
        mono = time.monotonic_ns()
        write_obj("vs/motion", {**self.s["vs/motion"], "ts_mono_ns": mono})
        write_obj("vs/telltales", {**self.s["vs/telltales"], "seq": self.seq})
        if self.seq % 10 == 0:   # TCU status heartbeat, 1 Hz (stale after link_status_timeout_ms)
            write_obj("tcu/links", {**self.s["tcu/links"], "hb": self.seq // 10})
        self.handle_requests()
        self.advance_emergency()

    # Requests are what the real servers authorise; the simulator just accepts them.
    def handle_requests(self):
        for obj in ("vs/request", "media/request", "wifi/request", "tcu/emergency_request", "ota/request", "nav/request", "privacy/request"):
            p = os.path.join(ROOT, obj)
            try:
                m = os.stat(p).st_mtime_ns
            except FileNotFoundError:
                continue
            if self.req_mtime.get(obj) == m:
                continue
            first = obj not in self.req_mtime
            self.req_mtime[obj] = m
            if first:
                continue
            r = read_obj(obj)
            print(f"  ← request {obj}: {r}")
            if obj == "vs/request":
                for k in ("climate_setpoint_c", "seat_heat", "defrost"):
                    if k in r:
                        self.s["vs/state"][k] = r[k]
                write_obj("vs/state", self.s["vs/state"])
            elif obj == "media/request":
                a = r.get("action")
                if a == "tune":
                    st = next((x for x in STATIONS if x["freq"] == r.get("freq")), None)
                    if st:
                        self.set("media/state", station=st["name"], freq=st["freq"], source=st["band"], title="", playing=True)
                elif a in ("play", "pause"):
                    self.set("media/state", playing=(a == "play"))
                elif a == "source":
                    self.set("media/state", source=r.get("source", "fm"))
                write_obj("media/state", self.s["media/state"])
            elif obj == "wifi/request":
                if "enabled" in r:
                    self.set("wifi/status", enabled=r["enabled"], clients=3 if r["enabled"] else 0)
                if "cap_mb" in r:
                    self.set("wifi/status", cap_mb=r["cap_mb"])
                write_obj("wifi/status", self.s["wifi/status"])
            elif obj == "tcu/emergency_request":
                if r.get("action") == "sos":
                    self.start_emergency("sos")
                elif r.get("action") == "cancel":
                    self.set("tcu/emergency", state="ended")
                    write_obj("tcu/emergency", self.s["tcu/emergency"])
                    self.emg_t0 = None
            elif obj == "ota/request" and r.get("action") == "install":
                self.set("ota/status", state="installing", progress=0.0)
                write_obj("ota/status", self.s["ota/status"])

    def bearer(self):
        L = self.s["tcu/links"]
        ok = lambda l: l.get("state") in ("up", "degraded")
        if ok(L["geo"]):
            return "geo"
        if ok(L["leo"]):
            return "leo"
        if L["terr"].get("fitted") and ok(L["terr"]):
            return "terrestrial"
        return "none"

    def start_emergency(self, kind):
        b = self.bearer()
        meo = self.s["tcu/links"]["meo"]
        self.set("tcu/emergency", state="connecting" if b != "none" else "queued", kind=kind, bearer=b,
                 location_sent=False, lat=meo["lat"], lon=meo["lon"],
                 cancel_until_ms=int(time.time() * 1000) + (10000 if kind == "acn" else 0))
        write_obj("tcu/emergency", self.s["tcu/emergency"])
        self.emg_t0 = time.monotonic()

    def advance_emergency(self):
        if self.emg_t0 is None:
            return
        e = self.s["tcu/emergency"]
        if e["state"] in ("connecting", "queued") and time.monotonic() - self.emg_t0 > 2.5 and self.bearer() != "none":
            self.set("tcu/emergency", state="active", location_sent=True, bearer=self.bearer())
            write_obj("tcu/emergency", e)
        if self.s["ota/status"]["state"] == "installing":
            pass


# ---------------------------------------------------------------- scenarios
def sc_parked(w):
    return [(0, "Parked in Montréal, −18 °C, all orbits up, OTA ready", lambda: None)]


def drive(w, kph=100):
    w.set("vs/motion", speed_kph=float(kph), gear="D")
    w.set("vs/state", ambient_c=-24.0)
    w.set("nav/guidance", **ROUTE)
    w.set("media/state", title="Le 15-18", playing=True)


def sc_winter_drive(w):
    def slippery():
        w.set("vs/warnings", active=[{"id": "lt1", "code": "low_traction", "severity": "high"}])
        w.set("vs/telltales", esc=True)
    def clear():
        w.set("vs/warnings", active=[])
        w.set("vs/telltales", esc=False)
    return [(0, "A-20 East at 100 km/h, −24 °C — driver lockout active", lambda: drive(w)),
            (6, "Black ice: ESC intervenes, low-traction warning pre-empts cluster", slippery),
            (10, "Traction recovered", clear)]


def sc_tunnel(w):
    def enter():
        w.link("leo", state="blocked", dl_mbps=0, ul_mbps=0)
        w.link("geo", state="blocked")
        w.link("meo", state="degraded", fix="dr", sats=0)
        w.s["tcu/links"].update(sky_view=False, queue_mb=3.5,
                                routing={k: ("queued" if k != "bulk" else "wifi") for k in BASE["tcu/links"]["routing"]})
        w.set("media/state", blend=False)
    def leo_back():
        w.link("leo", state="up", dl_mbps=96, ul_mbps=12)
        w.link("meo", state="up", fix="3d", sats=17)
        w.s["tcu/links"].update(sky_view=True, routing={**BASE["tcu/links"]["routing"], "emergency": "leo", "safety": "leo"})
    def geo_back():
        w.link("geo", state="up")
        w.s["tcu/links"].update(queue_mb=0.0, routing=copy.deepcopy(BASE["tcu/links"]["routing"]))
        w.set("media/state", blend=True)
    return [(0, "Approaching Louis-H.-La Fontaine tunnel — route data already prefetched", lambda: drive(w, 70)),
            (4, "In tunnel: no sky view. Offline nav continues on dead reckoning; telemetry queued", enter),
            (10, "Exit: LEO back first — emergency path falls back GEO→LEO", leo_back),
            (14, "GEO back — queue flushed, emergency path returns to GEO", geo_back)]


def sc_takeover(w):
    def l3():
        w.set("vs/av", mode="l3")
    def request():
        w.set("vs/av", mode="takeover", takeover_id="tor-%d" % int(time.time()), takeover_reason="weather",
              takeover_deadline_ms=10000, ts_mono_ns=time.monotonic_ns())
    def done():
        w.set("vs/av", mode="manual", takeover_id="", takeover_reason="")
    return [(0, "Highway, automated driving (L3) engaged", lambda: (drive(w, 95), l3())),
            (5, "Snow squall: sensors limited → bilingual takeover request on cluster + HUD", request),
            (11, "Driver has taken over", done)]


def sc_wildfire(w):
    return [(0, "Driving north of Québec", lambda: drive(w, 90)),
            (3, "NPAS / Alert Ready wildfire alert → shown + spoken in session language, media paused",
             lambda: w.set("alerts/public", active=[WILDFIRE]))]


def sc_crash(w):
    def crash():
        w.set("vs/motion", speed_kph=0.0, gear="P")
        w.set("vs/telltales", airbag=True, check=True)
        w.start_emergency("acn")
    return [(0, "Rural road, no terrestrial coverage", lambda: drive(w, 80)),
            (4, "Crash detected → automatic crash notification over GEO (NG911-ready)", crash)]


def sc_full_demo(w):
    t, steps = 0, []
    for name, fn, dur in (("parked", sc_parked, 6), ("winter-drive", sc_winter_drive, 14), ("tunnel", sc_tunnel, 18),
                          ("takeover", sc_takeover, 14), ("wildfire", sc_wildfire, 12), ("crash", sc_crash, 10)):
        steps.append((t, f"── {name} ──", lambda: w.__init__()))
        for (dt, desc, f) in fn(w):
            steps.append((t + dt + 0.01, desc, f))
        t += dur
    return steps


SCENARIOS = {"parked": sc_parked, "winter-drive": sc_winter_drive, "tunnel": sc_tunnel, "takeover": sc_takeover,
             "wildfire": sc_wildfire, "crash": sc_crash, "full-demo": sc_full_demo}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--scenario", default="parked", choices=sorted(SCENARIOS))
    ap.add_argument("--until", type=float, help="stop advancing the timeline after T seconds (hold state)")
    ap.add_argument("--duration", type=float, help="exit after T seconds")
    ap.add_argument("--loop", action="store_true", help="restart the scenario when it ends")
    ap.add_argument("--list", action="store_true")
    a = ap.parse_args()
    if a.list:
        for k in sorted(SCENARIOS):
            print(k)
        return 0

    w = World()
    os.makedirs(ROOT, exist_ok=True)
    print(f"civi-vsim → {ROOT}  scenario={a.scenario}")
    steps = sorted(SCENARIOS[a.scenario](w), key=lambda s: s[0])
    end_t = steps[-1][0] + 6
    w.publish_all()
    t0, i = time.monotonic(), 0
    while True:
        t = time.monotonic() - t0
        while i < len(steps) and steps[i][0] <= t and (a.until is None or steps[i][0] <= a.until):
            print(f"[{steps[i][0]:5.1f}s] {steps[i][1]}")
            steps[i][2]()
            w.publish_all()
            i += 1
        w.tick()
        if a.duration and t >= a.duration:
            return 0
        if a.loop and i >= len(steps) and t > end_t and a.until is None:
            w.__init__()
            w.publish_all()
            t0, i = time.monotonic(), 0
        time.sleep(0.1)


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        pass

#!/usr/bin/env python3
"""Per-build SBOM with supplier and origin (CIVI-SW-007, -SEC-010) and the US BIS
connected-vehicle provenance gate (CIVI-SEC-012).

Emits CycloneDX 1.5 JSON. Exit codes: 0 pass, 1 restricted origin in a market's
covered scope, 2 unknown provenance with --release.

  generate_sbom.py --out civi-sbom.cdx.json [--market US] [--release]
"""
import argparse, datetime, json, os, sys, uuid


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=os.path.join(os.path.dirname(__file__), "..", ".."))
    ap.add_argument("--out", required=True)
    ap.add_argument("--market", action="append", default=[])
    ap.add_argument("--release", action="store_true", help="treat TBD provenance as a failure")
    a = ap.parse_args()
    root = os.path.abspath(a.root)
    comps = json.load(open(os.path.join(root, "third_party", "manifest.json")))["components"]
    rules = json.load(open(os.path.join(root, "config", "compliance", "restricted-origins.json")))["markets"]

    bom = {
        "bomFormat": "CycloneDX", "specVersion": "1.5", "serialNumber": f"urn:uuid:{uuid.uuid4()}", "version": 1,
        "metadata": {"timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="seconds"),
                     "component": {"type": "application", "name": "civi-frontend", "version": "0.1.0"}},
        "components": [],
    }
    violations, unknown = [], []
    for c in comps:
        bom["components"].append({
            "type": "application" if c["supplier"].startswith("InnoStar") else "library",
            "name": c["name"], "version": c["version"],
            "supplier": {"name": c["supplier"]},
            "licenses": [{"license": {"name": c["license"]}}],
            **({"purl": c["purl"]} if "purl" in c else {}),
            "properties": [
                {"name": "civi:origin", "value": c["origin"]},
                {"name": "civi:partition", "value": c["partition"]},
                {"name": "civi:connectivity_stack", "value": str(c["connectivity_stack"]).lower()},
            ],
        })
        if c["origin"] == "TBD":
            unknown.append(c["name"])
        for m in a.market:
            r = rules.get(m)
            if r and c["connectivity_stack"] and c["origin"] in r["restricted_origins"]:
                violations.append(f"{m}: {c['name']} ({c['supplier']}, origin {c['origin']})")

    with open(a.out, "w") as f:
        json.dump(bom, f, indent=2)
    print(f"SBOM: {len(comps)} components -> {a.out}")
    for u in unknown:
        print(f"{'ERROR' if a.release else 'WARN '} provenance unknown: {u}")
    for v in violations:
        print(f"ERROR restricted origin: {v}")
    if violations:
        return 1
    if unknown and a.release:
        return 2
    print("Provenance gate: PASS" + (" (with open TBD suppliers)" if unknown else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""French-parity release gate (CIVI-VAL-002, CIVI-SEC-011, CIVI-HMI-001/-002).

Fails (exit 1) if:
  * any catalog entry lacks a non-empty French or English string;
  * a QML/C++ source references a qsTrId id that is not in the catalog;
  * a dynamic id prefix (qsTrId("civi.x." + v)) has no catalog entries;
  * a user-visible QML property is assigned a raw string literal instead of qsTrId()
    (suppress a deliberate case with a trailing  // civi-i18n-ignore  comment);
  * %n placeholders differ between French and English.
Warns when French exceeds English by more than the configured expansion allowance
(TBD-5) so layout tests can target those strings.

Usage: check_parity.py [--root DIR] [--json report.json]
"""
import argparse, json, os, re, sys

ID_RE = re.compile(r'qsTrId\(\s*"([^"]+)"\s*\)')
DYN_RE = re.compile(r'qsTrId\(\s*"([^"]+\.)"\s*\+')
CPP_ID_RE = re.compile(r'"(civi\.[a-z0-9_]+\.[a-z0-9_.]*[a-z0-9_])"')   # >= 2 dots: skips logging categories
LITERAL_RE = re.compile(r'(?:^|[{;])\s*(text|title|label|detail|placeholderText)\s*:\s*"([^"]*[A-Za-zÀ-ÿ]{2,}[^"]*)"')
PH_RE = re.compile(r'%(?:\d|n)')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=os.path.join(os.path.dirname(__file__), "..", ".."))
    ap.add_argument("--json")
    ap.add_argument("--verbose", action="store_true")
    a = ap.parse_args()
    root = os.path.abspath(a.root)

    cat = json.load(open(os.path.join(root, "i18n", "catalog.json"), encoding="utf-8"))["messages"]
    cfg = json.load(open(os.path.join(root, "config", "civi-config.json"), encoding="utf-8"))
    allowance = cfg.get("i18n", {}).get("max_french_expansion_pct", 30)

    errors, warnings = [], []
    for mid, e in cat.items():
        fr, en = e.get("fr", "").strip(), e.get("en", "").strip()
        if not fr:
            errors.append(f"{mid}: missing French")
        if not en:
            errors.append(f"{mid}: missing English")
        for one in ("fr_one", "en_one"):
            if one in e and sorted(PH_RE.findall(e[one])) != sorted(PH_RE.findall(e[one[:2]])):
                errors.append(f"{mid}: placeholder mismatch in {one}")
        if ("fr_one" in e) != ("en_one" in e):
            errors.append(f"{mid}: singular form present in one language only")
        if sorted(PH_RE.findall(fr)) != sorted(PH_RE.findall(en)):
            errors.append(f"{mid}: placeholder mismatch fr={PH_RE.findall(fr)} en={PH_RE.findall(en)}")
        if fr and en and len(fr) > len(en) * (1 + allowance / 100) and len(en) > 6:
            warnings.append(f"{mid}: French {len(fr)} chars vs English {len(en)} (> {allowance}% expansion) — check layout")

    used, prefixes = set(), set()
    for d in ("hmi", "platform"):
        for dp, _, files in os.walk(os.path.join(root, d)):
            for fn in files:
                p = os.path.join(dp, fn)
                rel = os.path.relpath(p, root)
                if fn.endswith(".qml"):
                    for n, line in enumerate(open(p, encoding="utf-8"), 1):
                        used.update(ID_RE.findall(line))
                        prefixes.update(DYN_RE.findall(line))
                        if "civi-i18n-ignore" in line:
                            continue
                        for prop, lit in LITERAL_RE.findall(line):
                            errors.append(f"{rel}:{n}: hard-coded user-visible string in '{prop}': \"{lit}\"")
                elif fn.endswith((".cpp", ".h")):
                    src = open(p, encoding="utf-8").read()
                    for m in CPP_ID_RE.findall(src):
                        if not m.endswith("."):
                            used.add(m)
                    prefixes.update(re.findall(r'"(civi\.[a-z0-9_]+\.[a-z0-9_.]*\.)"', src))

    for mid in sorted(used):
        if mid not in cat:
            errors.append(f"id used but not in catalog: {mid}")
    for p in sorted(prefixes):
        if not any(k.startswith(p) for k in cat):
            errors.append(f"dynamic id prefix has no catalog entries: {p}*")

    report = {"messages": len(cat), "ids_referenced": len(used), "dynamic_prefixes": len(prefixes),
              "errors": errors, "warnings": warnings, "pass": not errors}
    if a.json:
        json.dump(report, open(a.json, "w"), indent=2, ensure_ascii=False)
    if a.verbose:
        for w in warnings:
            print("INFO ", w)
    elif warnings:
        print(f"INFO  {len(warnings)} strings exceed the {allowance}% French expansion allowance (TBD-5); "
              "their layouts are covered by the bilingual screenshot run (--verbose to list)")
    for e in errors:
        print("ERROR", e)
    print(f"French parity: {len(cat)} messages, {len(used)} static ids, {len(prefixes)} dynamic prefixes — "
          + ("PASS" if not errors else f"FAIL ({len(errors)} errors)"))
    return 0 if not errors else 1


if __name__ == "__main__":
    sys.exit(main())

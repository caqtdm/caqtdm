#!/usr/bin/env bash
set -euo pipefail

epics_source="$1"

# The POSIX fallback calls ClockTime_Init, which EPICS excludes on Apple targets.
cp "${epics_source}/modules/libcom/src/osi/os/Darwin/osdTime.cpp" \
  "${epics_source}/modules/libcom/src/osi/os/iOS/osdTime.cpp"

python3 - "${epics_source}" <<'PY'
from pathlib import Path
import re
import sys

root = Path(sys.argv[1]) / "modules/pva2pva"
matches = []
for makefile in root.rglob("Makefile"):
    lines = makefile.read_text().splitlines(keepends=True)
    for index, line in enumerate(lines):
        definition = line.split("#", 1)[0].strip()
        if "softIocPVA" in definition and re.match(r"^(?:TEST)?PROD(?:_IOC)?\s*\+?=", definition):
            matches.append((makefile, lines, index))

if len(matches) != 1:
    raise SystemExit(f"Expected one softIocPVA product declaration, found {len(matches)}")

makefile, lines, index = matches[0]
lines[index] = "# Omitted for iOS: softIocPVA is an executable, not a library.\n"
makefile.write_text("".join(lines))
print(f"Disabled iOS softIocPVA product in {makefile}")
PY

#!/usr/bin/env bash
set -euo pipefail

epics_source="$1"

# The POSIX fallback calls ClockTime_Init, which EPICS excludes on Apple targets.
cp "${epics_source}/modules/libcom/src/osi/os/Darwin/osdTime.cpp" \
  "${epics_source}/modules/libcom/src/osi/os/iOS/osdTime.cpp"

# AGENTS.md

## Layout And Clean Contract

- Existing src/scripts/docs/build layout is already conforming; do not move working application/firmware modules for cosmetics.
- Normal clean uses scripts/clean_intermediates.ps1: preflight all paths/reparse points, delete only allowlisted compiler files, never recursively purge folders by name. Preserve EXE/INI/DLL/firmware/data/log/evidence/unknown content even in obj/tests/verify/x64/CMakeFiles.
- Codex automatically verifies actual clean plus nested-delivery/idempotence/junction fixtures; no manual setup is assigned to the user.

## Project Purpose

This project builds a native Windows MFC end-user PMBus dashboard tool.

- PC tool connects to an M032 EVB USB HID bridge.
- The bridge uses the existing 64-byte HID protocol with `0xA5` magic and a 6-byte header.
- Pressing `Start` initializes the selected M032 I2C master bus and automatically polls target PMBus read-only values.
- The user-facing scope is dashboard display, M-CRPS read-only detail fields, connection controls, and logs.

## Local Rules

- Keep the project layout aligned with sibling simple tools:
  - `src\`
  - `scripts\`
  - `docs\`
  - `demo_code\`
  - root `README.md`, `AGENTS.md`, `SELF_CHECK.md`, `HANDOFF.md`
- Reuse the PMBus/SMBus bridge contract from `docs\PMBUS_COMMAND_CONTRACT.csv`.
- Do not expose engineer-oriented raw hex command controls in the main workflow.
- V1 must stay read-only except PMBus `PAGE` writes for non-destructive rail selection.
- Do not add calibration, EEP, target firmware update, firmware upload, or destructive MFR flows without an explicit product safety policy.
- Keep polling work off the UI thread and aggregate repeated polling errors into log/status instead of repeated modal dialogs.

## Build

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts\build_mfc.ps1 -Configuration Release -Platform x64
```

Output:

- `build\PmbusGuiTool.exe`

## Verification Expectations

For code changes, report:

1. changed files
2. what changed
3. why it changed
4. how to verify
5. actual verification result, or `NOT RUN` with reason

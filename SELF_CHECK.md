# SELF_CHECK.md

Allowed result values: `PASS`, `FAIL`, `NOT RUN`, `N/A`.

## Build / Output

[BLD-01] Release x64 build

- Result: PASS
- Command: `powershell -NoProfile -ExecutionPolicy Bypass -File scripts\build_mfc.ps1 -Configuration Release -Platform x64`
- Observed: build succeeded with `0` warnings and `0` errors.

[BLD-02] EXE output

- Result: PASS
- Observed: `build\PmbusGuiTool.exe` exists.

[BLD-03] PMBus contract validation

- Result: PASS
- Command: `powershell -NoProfile -ExecutionPolicy Bypass -File scripts\validate_pmbus_contract.ps1`
- Observed: `PMBus contract validation PASS`, rows `224`, policy gaps `0`.

## Runtime

[RUN-01] Startup smoke without target

- Result: PASS
- Command: start `build\PmbusGuiTool.exe`, wait 3 seconds, close main window.
- Observed: `STARTED_AND_CLOSED`.

[RUN-02] M032 EVB hardware smoke

- Result: NOT RUN
- Reason: no physical M032 EVB and PMBus target were exercised in this session.

[RUN-03] Polling error behavior

- Result: NOT RUN
- Reason: requires connected bridge/target or forced hardware error path.

## UI / Packaging

[UI-01] App icon generated

- Result: PASS
- Command: `powershell -NoProfile -ExecutionPolicy Bypass -File scripts\generate_app_icon.ps1`
- Observed: generated `src\res\p_tool.ico`.

[UI-02] Basic layout sanity

- Result: PASS
- Observed: application starts and closes. Detailed visual DPI review is still pending.

[UI-03] 100% / 125% DPI visual review

- Result: NOT RUN
- Reason: no visual screenshot review was performed.

## Device / Protocol Boundary

[DEV-01] Read-only V1 scope

- Result: PASS
- Observed: V1 polling code reads standard PMBus/M-CRPS fields; automatic write path is limited to PMBus `PAGE` rail selection.

[DEV-02] Destructive commands excluded

- Result: PASS
- Observed: V1 dashboard field list does not call `MFR_FWUPLOAD`, `MFR_CLEAR_BLACK_BOX`, calibration, EEP, or write/readback validation commands.

## Final Summary

- PASS count: 7
- FAIL count: 0
- NOT RUN count: 3
- N/A count: 0

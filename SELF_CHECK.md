# SELF_CHECK.md

Allowed result values: `PASS`, `FAIL`, `NOT RUN`, `N/A`.

## Verified x64 EXE slimming 2026-10-25

- PASS comparable Release baseline 2850816 bytes (original 2850816);
  formal 729088 bytes, SHA 7322BB0AF031F3728C9D3257E0B6A3D53F5C8166F26EE5BC1121CF06A8E1ACDB.
- PASS scripts/build_mfc.ps1 -Configuration Release -Platform x64 -CloseRunningTool $false; isolated Debug with version prebuild disabled.
  No compiler/linker warnings/errors reported.
- PASS actual formal hash-matched fresh fixtures: controls/UI dispatch (no file-dialog workflow),
  identical child-control class counts, assertion-free Debug and normal exit 0.
  No personal INI, Connect, protocol command, flash or clipboard action.
- Initial dialog-test IDs 1022/13161 were invalid for this dashboard UI; those
  failures were test configuration errors. The active dashboard has no corresponding
  disconnected file-load control. Final startup/control/Debug checks omit that test.
- PASS /O2 /GL /MT /GS and PE 0x8160 retained; VisualManager RTTI absent,
  no new DLL runtime; x64 macro only. Application/protocol/firmware sources unchanged.
- PASS performance: two warmups, baseline calibration before Release candidate,
  5 alternating AB/BA pairs without concurrent compilation. Predeclared delta
  max(floor, 10% baseline median, 3*MAD); memory 5%; floors startup25/UI2/CPU16ms,
  memory1MiB. Results (baseline -> candidate; acceptance limit):
  - StartupMs: 467.0350 -> 409.6745; 1092.0350.
  - PeakBytes: 19410944.0000 -> 18931712.0000; 20459520.0000.
  - CpuMs: 312.5000 -> 328.1250; 406.5000.
  - WorkflowMs: 1.0603 -> 1.8588; 3.0603.
- Raw samples/limits and earlier failed timing where applicable:
  docs/verification/exe-slimming-20261025.json.
- PASS actual clean_mfc.bat removed 34 compiler files, all 2 protected actual
  files SHA unchanged; 17 nested fixture files/source, help/idempotence PASS.
  Prior unchanged junction-rejection evidence reused; root INI hashes preserved.
- NOT RUN: physical-device/protocol throughput, unrelated firmware, visual/DPI,
  clean-PC, Win32. Ignored EXE is not force-added to Git.


## Layout / Clean Audit 2026-10-20

- Existing source layout already conforms; no application/protocol/firmware restructuring or dependencies were introduced.
- Replaced broad directory deletion with standalone `scripts/clean_intermediates.ps1`; batch compatibility remains.
- Build: PASS: scripts/build_mfc.ps1 -Configuration Release -Platform x64 -CloseRunningTool $false; 0 warnings/errors; normal generated version 1.0.0.266.
- Runtime: PASS: isolated captioned main frame/startup/normal close, exit 0; no connection/write.
- PASS: shared verifier `-CleanBatch scripts/clean_mfc.bat -CleanActualProject -CheckNestedDelivery -CheckReparseRejection`; 17 protected fixture build files plus source retained, object fixture removed, repeat-clean and pre-deletion junction rejection PASS. 2 actual protected build files unchanged (SHA-256).
- The initial expanded audit classified MSBuild `obj/*.recipe` compiler metadata as protected and reported FAIL; the classification was corrected against Microsoft.CppBuild.targets (DesktopDeployRecipeFile), then the audit passed. No delivery EXE was deleted.
- NOT RUN: physical hardware/firmware workflows, complete GUI/DPI, new Debug acceptance. Publication only updates already tracked deliverables; private INIs/data/logs and newly built test EXEs are excluded.
- Final commit/push state: workspace `#READ_ME_FIRST#/PROJECT_LAYOUT_MIGRATION.md`.

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

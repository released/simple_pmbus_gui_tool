# HANDOFF.md

## Verified x64 EXE slimming 2026-10-25

- EXE 2850816 -> 729088 bytes; formal SHA
  7322BB0AF031F3728C9D3257E0B6A3D53F5C8166F26EE5BC1121CF06A8E1ACDB.
- Release x64 canonical build and isolated Debug PASS; controls/UI dispatch (no file-dialog workflow),
  matching child-control counts and normal exit 0 PASS. 5 AB/BA pairs PASS.
- Initial dialog-test IDs 1022/13161 were invalid for this dashboard UI; those
  failures were test configuration errors. The active dashboard has no corresponding
  disconnected file-load control. Final startup/control/Debug checks omit that test.
- Actual clean removed 34 compiler files; 2 protected actual files SHA
  unchanged. 17 nested fixture files/source, help/idempotence and root INI hashes PASS.
- EXE was already ignored/untracked; publication includes code/docs, not a force-added binary.
- Raw sanitized evidence: docs/verification/exe-slimming-20261025.json.
  Physical devices, unrelated firmware, visual/DPI/clean-PC/Win32 NOT RUN.


## Layout / Clean Audit 2026-10-20

- Existing source layout already conforms; no application/protocol/firmware restructuring or dependencies were introduced.
- Replaced broad directory deletion with standalone `scripts/clean_intermediates.ps1`; batch compatibility remains.
- Build: PASS: scripts/build_mfc.ps1 -Configuration Release -Platform x64 -CloseRunningTool $false; 0 warnings/errors; normal generated version 1.0.0.266.
- Runtime: PASS: isolated captioned main frame/startup/normal close, exit 0; no connection/write.
- PASS: shared verifier `-CleanBatch scripts/clean_mfc.bat -CleanActualProject -CheckNestedDelivery -CheckReparseRejection`; 17 protected fixture build files plus source retained, object fixture removed, repeat-clean and pre-deletion junction rejection PASS. 2 actual protected build files unchanged (SHA-256).
- The initial expanded audit classified MSBuild `obj/*.recipe` compiler metadata as protected and reported FAIL; the classification was corrected against Microsoft.CppBuild.targets (DesktopDeployRecipeFile), then the audit passed. No delivery EXE was deleted.
- NOT RUN: physical hardware/firmware workflows, complete GUI/DPI, new Debug acceptance. Publication only updates already tracked deliverables; private INIs/data/logs and newly built test EXEs are excluded.
- Final commit/push state: workspace `#READ_ME_FIRST#/PROJECT_LAYOUT_MIGRATION.md`.

## Current Status

Implemented native MFC V1 of `PMBus GUI tool` in `simple_pmbus_gui_tool`.

The tool builds to:

```text
build\PmbusGuiTool.exe
```

## Implemented

- New MFC solution/project: `PmbusGuiTool.sln`, `PmbusGuiTool.vcxproj`.
- Reused HID bridge, I2C bridge service, PMBus contract generation, and PMBus decode helpers from the reference PMBus/SMBus project.
- Added:
  - `PmbusReader`
  - `DashboardPoller`
  - dashboard field profile
  - Main dashboard tab
  - ExtraCmd M-CRPS read-only tab
  - disabled placeholder tabs for reserved features
- Added Start/Stop polling workflow:
  - I2C master init
  - bus status/recover preflight
  - read-only PMBus polling
  - auto-stop after 5 consecutive polling-error cycles
  - I2C deinit on stop/exit
- The visible V1 M-CRPS dashboard is single 12V output. Do not expose PMBus `PAGE` as 12V/5V/3.3V rail selection unless a product profile defines that mapping.

## Verified

- Release x64 build: PASS
- PMBus contract validation: PASS
- EXE startup/close smoke without target: PASS

## Not Verified

- Real M032 EVB + target PMBus polling.
- 100% / 125% DPI visual screenshot review.
- Hardware error/unplug auto-stop path.

## Notes For Next Work

- Run hardware smoke with an M032 EVB flashed with the existing PMBus/SMBus bridge firmware.
- Confirm target PMBus address and PAGE mapping for the intended product.
- Replace any placeholder M-CRPS field interpretation only after product owners provide final command semantics.
- Keep `Update`, `Calibration`, and `EEP` disabled until a safety policy exists.

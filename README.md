# PMBus GUI tool

Native Windows MFC PMBus dashboard for end users.

The tool connects to an M032 EVB USB HID bridge, uses the EVB I2C controller as a PMBus master, and displays decoded PMBus/M-CRPS read-only values in a dashboard UI.


## Current Scope

- Primary bridge board: M032 EVB running the PMBus/SMBus HID bridge firmware.
- M261 EVB / M263KIAAE reference firmware is under `demo_code\M261BSP_USB_HID_PMBus_SMBus`, with HID IAP bootloader, application checksum post-build and an M261 I2C backend. M261 hardware operation is not production-verified.
- Target bus role: PMBus master only.
- UI flow: scan USB, connect HID bridge, press `Start`, poll and display values.
- Main dashboard: M-CRPS 12V output voltage/current/power, RPM, VIN/PIN/POUT, temperatures, protection thresholds, status, identity, and MFR fields.
- Detail tabs: `Config`, `Faults`, `Monitor`, `MFR`, and `M-CRPS` read-only PMBus/M-CRPS fields.
- V1 does not implement graph, calibration, EEP, update, or firmware upload workflows.

Out of V1 scope:

- Raw PMBus hex command workflow.
- Calibration, EEP writes, target firmware update, or MFR firmware upload.
- Destructive MFR flows such as blackbox clear or write/readback validation.

## Bridge Firmware

Bridge firmware sources:

- M031/M032 bridge firmware: `demo_code\M031BSP_USB_HID_PMBus_SMBus`
- M261 staged bridge firmware: `demo_code\M261BSP_USB_HID_PMBus_SMBus`

Both firmware folders use a USB HID IAP bootloader plus application split for bridge self-update:

- Bootloader HID PID: `0x3F00`
- Application HID PID: `0x5020`
- M031/M032 app base: `0x00004000`, CRC word at `0x0001FFFC`
- M261 app base: `0x00010000`, CRC word at `0x0007FFFC`

The bootloader is only for updating the bridge application image. It does not implement target-board firmware upload. Use the sibling `simple_programming_tool` after the board is already in HID ISP bootloader mode.

The top-level GUI includes `Return Bootloader`. It sends `BRIDGE_CMD_ENTER_IAP` (`0x04`) to the running bridge application, erases only the application CRC word, disconnects from the app HID device, and resets through the bootloader path. After the bootloader enumerates as `VID 0x0416`, `PID 0x3F00`, use `simple_programming_tool` to program a valid padded bridge application image. This button is an app-side bridge self-update entry point; it is not a target-board programming flow.

## Defaults

- Bridge VID/PID: `0x0416 / 0x5020`
- Target PMBus address: `0x5A`
- I2C pins: `I2C0 PB.4=SDA / PB.5=SCL`
- Speed: `400000`
- Poll interval: `1000 ms`
- PEC: on
- Output mode: `M-CRPS 12V output`

The V1 M-CRPS dashboard keeps PMBus `PAGE` handling profile-neutral. Page-specific output display should be enabled only after a product profile defines the page-to-output mapping.

For M261, use the default `I2C0 PB.4=SDA / PB.5=SCL` pin pair. The firmware backend follows the M261 pin support in `simple_pmbus_smbus_tool`; this dashboard has no dedicated board-profile selector.

## Polling Commands

The GUI uses the configured poll interval for the main dynamic dashboard. The default interval is `1000 ms`.

Startup/preflight:

- Bridge `Get Info`
- Bridge `I2cMasterInit`
- Bridge `I2cMasterBusStatus(recover=true)`
- PMBus `VOUT_MODE` (`0x20`) to configure `READ_VOUT` decoding

Every polling cycle in the default single-output mode:

- `READ_VOUT` (`0x8B`)
- `READ_IOUT` (`0x8C`)
- `READ_POUT` (`0x96`)
- `VOUT_OV_FAULT_LIMIT` (`0x40`)
- `VOUT_UV_FAULT_LIMIT` (`0x44`)
- `IOUT_OC_FAULT_LIMIT` (`0x46`)
- `READ_VIN` (`0x88`)
- `READ_FAN_SPEED_1` (`0x90`)
- `READ_PIN` (`0x97`)
- `READ_TEMPERATURE_1` (`0x8D`)
- `READ_TEMPERATURE_2` (`0x8E`)
- `READ_TEMPERATURE_3` (`0x8F`)
- `STATUS_WORD` (`0x79`)
- `STATUS_VOUT` (`0x7A`)
- `STATUS_IOUT` (`0x7B`)
- `STATUS_INPUT` (`0x7C`)
- `STATUS_TEMPERATURE` (`0x7D`)
- `STATUS_CML` (`0x7E`)

Static/detail refresh runs on cycle 1 and then every 30 cycles:

- Identity/static: `PMBUS_REVISION` (`0x98`), `MFR_ID` (`0x99`), `MFR_MODEL` (`0x9A`), `MFR_REVISION` (`0x9B`), `MFR_SERIAL` (`0x9E`), `MFR_FW_REVISION` (`0xD9`)
- Config tab: `OPERATION`, `ON_OFF_CONFIG`, `WRITE_PROTECT`, `CAPABILITY`, `VOUT_MODE`, `VOUT_COMMAND`, `VOUT_TRIM`, `VOUT_CAL_OFFSET`, `VOUT_MAX`, `VOUT_MARGIN_HIGH`, `VOUT_MARGIN_LOW`, `VOUT_TRANSITION_RATE`, `VOUT_DROOP`, `VOUT_SCALE_LOOP`, `VOUT_SCALE_MONITOR`, `VOUT_MIN`, `POUT_MAX`, `MAX_DUTY`, `FREQUENCY_SWITCH`, `POWER_MODE`, `VIN_ON`, `VIN_OFF`, `INTERLEAVE`, `IOUT_CAL_GAIN`, `IOUT_CAL_OFFSET`, `FAN_CONFIG_1_2`, `FAN_COMMAND_1`, `FAN_COMMAND_2`, `FAN_CONFIG_3_4`, `FAN_COMMAND_3`, `FAN_COMMAND_4`, `POWER_GOOD_ON`, `POWER_GOOD_OFF`, `TON_DELAY`, `TON_RISE`, `TOFF_DELAY`, `TOFF_FALL`
- Faults tab: `VOUT_OV_FAULT_RESPONSE`, `VOUT_OV_WARN_LIMIT`, `VOUT_UV_WARN_LIMIT`, `VOUT_UV_FAULT_RESPONSE`, `IOUT_OC_FAULT_RESPONSE`, `IOUT_OC_LV_FAULT_LIMIT`, `IOUT_OC_LV_FAULT_RESPONSE`, `IOUT_OC_WARN_LIMIT`, `IOUT_UC_FAULT_LIMIT`, `IOUT_UC_FAULT_RESPONSE`, `OT_FAULT_LIMIT`, `OT_FAULT_RESPONSE`, `OT_WARN_LIMIT`, `UT_WARN_LIMIT`, `UT_FAULT_LIMIT`, `UT_FAULT_RESPONSE`, `VIN_OV_FAULT_LIMIT`, `VIN_OV_FAULT_RESPONSE`, `VIN_OV_WARN_LIMIT`, `VIN_UV_WARN_LIMIT`, `VIN_UV_FAULT_LIMIT`, `VIN_UV_FAULT_RESPONSE`, `IIN_OC_FAULT_LIMIT`, `IIN_OC_FAULT_RESPONSE`, `IIN_OC_WARN_LIMIT`, `TON_MAX_FAULT_LIMIT`, `TON_MAX_FAULT_RESPONSE`, `TOFF_MAX_WARN_LIMIT`, `POUT_OP_FAULT_LIMIT`, `POUT_OP_FAULT_RESPONSE`, `POUT_OP_WARN_LIMIT`, `PIN_OP_WARN_LIMIT`, `STATUS_BYTE`, `STATUS_OTHER`, `STATUS_MFR_SPECIFIC`, `STATUS_FANS_1_2`, `STATUS_FANS_3_4`
- Monitor tab: `READ_KWH_IN`, `READ_KWH_OUT`, `READ_KWH_CONFIG`, `READ_EIN`, `READ_EOUT`, `READ_IIN`, `READ_VCAP`, `READ_FAN_SPEED_2`, `READ_FAN_SPEED_3`, `READ_FAN_SPEED_4`, `READ_DUTY_CYCLE`, `READ_FREQUENCY`
- MFR tab: `MFR_LOCATION`, `MFR_DATE`, `MFR_VIN_MIN`, `MFR_VIN_MAX`, `MFR_IIN_MAX`, `MFR_PIN_MAX`, `MFR_VOUT_MIN`, `MFR_VOUT_MAX`, `MFR_IOUT_MAX`, `MFR_POUT_MAX`, `MFR_TAMBIENT_MAX`, `MFR_TAMBIENT_MIN`, `MFR_EFFICIENCY_LL`, `MFR_EFFICIENCY_HL`, `MFR_PIN_ACCURACY`, `IC_DEVICE_ID`, `IC_DEVICE_REV`, `USER_DATA_00`, `USER_DATA_01`, `USER_DATA_02`, `MFR_EFFICIENCY_DATA`, `USER_DATA_04` through `USER_DATA_15`, `MFR_MAX_TEMP_1`, `MFR_MAX_TEMP_2`, `MFR_MAX_TEMP_3`, `MFR_COLD_REDUNDANCY_CONFIG`
- M-CRPS tab: `MFR_READ_CONFIG_FILE_SIZE`, `MFR_READ_CONFIG_BLOCK_SIZE`, `MFR_HW_COMPATIBILITY`, `MFR_FWUPLOAD_CAPABILITY`, `MFR_FWUPLOAD_MODE`, `MFR_FWUPLOAD_STATUS`, `MFR_FW_REVISION`, `MFR_FRU_PROTECTION`, `MFR_BLACKBOX`, `MFR_REAL_TIME_BLACK_BOX`, `MFR_SYSTEM_BLACK_BOX`, `MFR_BLACK_BOX_CONFIG`, `MFR_LINE_STATUS`, `MFR_SYSTEM_LED_CNTL`, `MFR_FWUPLOAD_BLOCK_SIZE`, `MFR_EN_STATUS_SIMULATION_CMD`, `MFR_PEAK_CURRENT_RECORD`, `MFR_COMPONENT_ID`, `MFR_TOT_POUT_MAX`, `MFR_VOUT_MARGINING`, `MFR_OCWPL1_SETTING`, `MFR_PWOK_WARNING_TIME`, `MFR_MAX_IOUT_CAPABILITY`, `MFR_FPC_MAIN_MIN_OFF_TIME`, `MFR_FPC_12VSB_MIN_OFF_TIME`

`QUERY` (`0x1A`) is displayed as parameterized and is not auto-polled in V1. The default GUI mode does not write PMBus `PAGE`.

## M031BSP Target Alignment

The default GUI settings match the `M031BSP_I2C_Slave_PMBus` reference target:

- Target address `0x5A`
- I2C speed `400000`
- PEC enabled by default
- Single M-CRPS 12V output mode

With the current M031BSP sample, `READ_VOUT`, `READ_IOUT`, and `READ_POUT` return the sample's single telemetry shadow values. The sample stores the received PMBus `PAGE` value, but the current GUI default mode reads and displays the single-output telemetry only.

## Build

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts\build_mfc.ps1 -Configuration Release -Platform x64
```

Expected output:

```text
build\PmbusGuiTool.exe
```

## Quick Start

1. Connect and scan the M032 USB HID bridge, then select the target settings.
2. Press `Start` to initialize I2C and poll the dashboard. Start requires a connected bridge.
3. Read the dynamic values and detail tabs; target-dependent fields may be unavailable.
4. Press `Stop` to stop polling before disconnecting.

Run `scripts/clean_mfc.bat` for compiler cleanup; executable/settings/data files remain.

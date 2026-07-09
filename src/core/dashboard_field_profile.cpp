#include "dashboard_field_profile.h"

namespace mfc_tool::core {
namespace {

const std::vector<DashboardFieldDefinition> kMainFields = {
    {L"vin", L"VIN", 0x88u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Dynamic, 0, 3, L"V"},
    {L"rpm", L"RPM", 0x90u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Dynamic, 0, 0, L"RPM"},
    {L"pout", L"POUT", 0x96u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Dynamic, 0, 3, L"W"},
    {L"pin", L"PIN", 0x97u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Dynamic, 0, 3, L"W"},
    {L"temp_sr_mos", L"Temp. (SR MOS)", 0x8Du, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Dynamic, 0, 2, L"C"},
    {L"temp_ambient", L"Temp. (Ambient)", 0x8Eu, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Dynamic, 0, 2, L"C"},
    {L"temp_primary", L"Temp. (Primary)", 0x8Fu, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Dynamic, 0, 2, L"C"},
    {L"status_word", L"STATUS_WORD", 0x79u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Dynamic, 0, 0, L""},
    {L"status_vout", L"STATUS_VOUT", 0x7Au, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Dynamic, 0, 0, L""},
    {L"status_iout", L"STATUS_IOUT", 0x7Bu, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Dynamic, 0, 0, L""},
    {L"status_input", L"STATUS_INPUT", 0x7Cu, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Dynamic, 0, 0, L""},
    {L"status_temp", L"STATUS_TEMPERATURE", 0x7Du, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Dynamic, 0, 0, L""},
    {L"status_cml", L"STATUS_CML", 0x7Eu, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Dynamic, 0, 0, L""},
    {L"pmbus_revision", L"PMBus Rev.", 0x98u, DashboardProtocol::ReadByte, DashboardDecode::PmbusRevision, DashboardFieldKind::Static, 0, 0, L""},
    {L"mfr_id", L"MFR ID", 0x99u, DashboardProtocol::BlockRead, DashboardDecode::BlockAscii, DashboardFieldKind::Static, 32, 0, L""},
    {L"mfr_model", L"MFR MODEL", 0x9Au, DashboardProtocol::BlockRead, DashboardDecode::BlockAscii, DashboardFieldKind::Static, 32, 0, L""},
    {L"mfr_revision", L"MFR Rev.", 0x9Bu, DashboardProtocol::BlockRead, DashboardDecode::BlockAscii, DashboardFieldKind::Static, 32, 0, L""},
    {L"mfr_serial", L"MFR Number", 0x9Eu, DashboardProtocol::BlockRead, DashboardDecode::BlockAscii, DashboardFieldKind::Static, 32, 0, L""},
    {L"fw_revision", L"Firmware Rev.", 0xD9u, DashboardProtocol::BlockRead, DashboardDecode::BlockAscii, DashboardFieldKind::Static, 8, 0, L""},
};

const std::vector<DashboardFieldDefinition> kExtraFields = {
    {L"crps_mfr_fw_revision", L"MFR_FW_REVISION", 0xD9u, DashboardProtocol::BlockRead, DashboardDecode::BlockAscii, DashboardFieldKind::Extra, 8, 0, L""},
    {L"crps_mfr_component_id", L"MFR_COMPONENT_ID", 0xEBu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 16, 0, L""},
    {L"crps_mfr_line_status", L"MFR_LINE_STATUS", 0xE1u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_mfr_tot_pout_max", L"MFR_TOT_POUT_MAX", 0xECu, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"W"},
    {L"crps_mfr_max_iout_capability", L"MFR_MAX_IOUT_CAPABILITY", 0xF1u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 16, 0, L""},
    {L"crps_mfr_config_file_size", L"MFR_READ_CONFIG_FILE_SIZE", 0xD1u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 8, 0, L""},
    {L"crps_mfr_config_block_size", L"MFR_READ_CONFIG_BLOCK_SIZE", 0xD2u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_mfr_hw_compatibility", L"MFR_HW_COMPATIBILITY", 0xD4u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_mfr_fwupload_capability", L"MFR_FWUPLOAD_CAPABILITY", 0xD5u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_mfr_fwupload_mode", L"MFR_FWUPLOAD_MODE", 0xD6u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_mfr_fwupload_status", L"MFR_FWUPLOAD_STATUS", 0xD8u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_mfr_blackbox", L"MFR_BLACKBOX", 0xDCu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
};

const std::vector<DashboardFieldDefinition> kConfigFields = {
    {L"cfg_operation", L"OPERATION", 0x01u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_on_off_config", L"ON_OFF_CONFIG", 0x02u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_write_protect", L"WRITE_PROTECT", 0x10u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_capability", L"CAPABILITY", 0x19u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_query", L"QUERY (parameterized)", 0x1Au, DashboardProtocol::NotAvailable, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_vout_mode", L"VOUT_MODE", 0x20u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_vout_command", L"VOUT_COMMAND", 0x21u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"cfg_vout_trim", L"VOUT_TRIM", 0x22u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"cfg_vout_cal_offset", L"VOUT_CAL_OFFSET", 0x23u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"cfg_vout_max", L"VOUT_MAX", 0x24u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"cfg_vout_margin_high", L"VOUT_MARGIN_HIGH", 0x25u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"cfg_vout_margin_low", L"VOUT_MARGIN_LOW", 0x26u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"cfg_vout_transition_rate", L"VOUT_TRANSITION_RATE", 0x27u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
    {L"cfg_vout_droop", L"VOUT_DROOP", 0x28u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
    {L"cfg_vout_scale_loop", L"VOUT_SCALE_LOOP", 0x29u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
    {L"cfg_vout_scale_monitor", L"VOUT_SCALE_MONITOR", 0x2Au, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
    {L"cfg_vout_min", L"VOUT_MIN", 0x2Bu, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"cfg_pout_max", L"POUT_MAX", 0x31u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"W"},
    {L"cfg_max_duty", L"MAX_DUTY", 0x32u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"%"},
    {L"cfg_frequency_switch", L"FREQUENCY_SWITCH", 0x33u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"Hz"},
    {L"cfg_power_mode", L"POWER_MODE", 0x34u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_vin_on", L"VIN_ON", 0x35u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"cfg_vin_off", L"VIN_OFF", 0x36u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"cfg_interleave", L"INTERLEAVE", 0x37u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_iout_cal_gain", L"IOUT_CAL_GAIN", 0x38u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
    {L"cfg_iout_cal_offset", L"IOUT_CAL_OFFSET", 0x39u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"A"},
    {L"cfg_fan_config_12", L"FAN_CONFIG_1_2", 0x3Au, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_fan_command_1", L"FAN_COMMAND_1", 0x3Bu, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_fan_command_2", L"FAN_COMMAND_2", 0x3Cu, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_fan_config_34", L"FAN_CONFIG_3_4", 0x3Du, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_fan_command_3", L"FAN_COMMAND_3", 0x3Eu, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_fan_command_4", L"FAN_COMMAND_4", 0x3Fu, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"cfg_power_good_on", L"POWER_GOOD_ON", 0x5Eu, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"cfg_power_good_off", L"POWER_GOOD_OFF", 0x5Fu, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"cfg_ton_delay", L"TON_DELAY", 0x60u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
    {L"cfg_ton_rise", L"TON_RISE", 0x61u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
    {L"cfg_toff_delay", L"TOFF_DELAY", 0x64u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
    {L"cfg_toff_fall", L"TOFF_FALL", 0x65u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
};

const std::vector<DashboardFieldDefinition> kFaultFields = {
    {L"fault_vout_ov_response", L"VOUT_OV_FAULT_RESPONSE", 0x41u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_vout_ov_warn", L"VOUT_OV_WARN_LIMIT", 0x42u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"fault_vout_uv_warn", L"VOUT_UV_WARN_LIMIT", 0x43u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"fault_vout_uv_response", L"VOUT_UV_FAULT_RESPONSE", 0x45u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_iout_oc_response", L"IOUT_OC_FAULT_RESPONSE", 0x47u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_iout_oc_lv_limit", L"IOUT_OC_LV_FAULT_LIMIT", 0x48u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"A"},
    {L"fault_iout_oc_lv_response", L"IOUT_OC_LV_FAULT_RESPONSE", 0x49u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_iout_oc_warn", L"IOUT_OC_WARN_LIMIT", 0x4Au, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"A"},
    {L"fault_iout_uc_limit", L"IOUT_UC_FAULT_LIMIT", 0x4Bu, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"A"},
    {L"fault_iout_uc_response", L"IOUT_UC_FAULT_RESPONSE", 0x4Cu, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_ot_limit", L"OT_FAULT_LIMIT", 0x4Fu, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 2, L"C"},
    {L"fault_ot_response", L"OT_FAULT_RESPONSE", 0x50u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_ot_warn", L"OT_WARN_LIMIT", 0x51u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 2, L"C"},
    {L"fault_ut_warn", L"UT_WARN_LIMIT", 0x52u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 2, L"C"},
    {L"fault_ut_limit", L"UT_FAULT_LIMIT", 0x53u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 2, L"C"},
    {L"fault_ut_response", L"UT_FAULT_RESPONSE", 0x54u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_vin_ov_limit", L"VIN_OV_FAULT_LIMIT", 0x55u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"fault_vin_ov_response", L"VIN_OV_FAULT_RESPONSE", 0x56u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_vin_ov_warn", L"VIN_OV_WARN_LIMIT", 0x57u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"fault_vin_uv_warn", L"VIN_UV_WARN_LIMIT", 0x58u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"fault_vin_uv_limit", L"VIN_UV_FAULT_LIMIT", 0x59u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"fault_vin_uv_response", L"VIN_UV_FAULT_RESPONSE", 0x5Au, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_iin_oc_limit", L"IIN_OC_FAULT_LIMIT", 0x5Bu, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"A"},
    {L"fault_iin_oc_response", L"IIN_OC_FAULT_RESPONSE", 0x5Cu, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_iin_oc_warn", L"IIN_OC_WARN_LIMIT", 0x5Du, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"A"},
    {L"fault_ton_max_limit", L"TON_MAX_FAULT_LIMIT", 0x62u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
    {L"fault_ton_max_response", L"TON_MAX_FAULT_RESPONSE", 0x63u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_toff_max_warn", L"TOFF_MAX_WARN_LIMIT", 0x66u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
    {L"fault_pout_op_limit", L"POUT_OP_FAULT_LIMIT", 0x68u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"W"},
    {L"fault_pout_op_response", L"POUT_OP_FAULT_RESPONSE", 0x69u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_pout_op_warn", L"POUT_OP_WARN_LIMIT", 0x6Au, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"W"},
    {L"fault_pin_op_warn", L"PIN_OP_WARN_LIMIT", 0x6Bu, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"W"},
    {L"fault_status_byte", L"STATUS_BYTE", 0x78u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_status_other", L"STATUS_OTHER", 0x7Fu, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_status_mfr", L"STATUS_MFR_SPECIFIC", 0x80u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_status_fans_12", L"STATUS_FANS_1_2", 0x81u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"fault_status_fans_34", L"STATUS_FANS_3_4", 0x82u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
};

const std::vector<DashboardFieldDefinition> kMonitorFields = {
    {L"mon_read_kwh_in", L"READ_KWH_IN", 0x83u, DashboardProtocol::Read32, DashboardDecode::RawDword, DashboardFieldKind::Extra, 0, 0, L""},
    {L"mon_read_kwh_out", L"READ_KWH_OUT", 0x84u, DashboardProtocol::Read32, DashboardDecode::RawDword, DashboardFieldKind::Extra, 0, 0, L""},
    {L"mon_read_kwh_config", L"READ_KWH_CONFIG", 0x85u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"mon_read_ein", L"READ_EIN", 0x86u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mon_read_eout", L"READ_EOUT", 0x87u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mon_read_iin", L"READ_IIN", 0x89u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"A"},
    {L"mon_read_vcap", L"READ_VCAP", 0x8Au, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"mon_read_fan_speed_2", L"READ_FAN_SPEED_2", 0x91u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L"RPM"},
    {L"mon_read_fan_speed_3", L"READ_FAN_SPEED_3", 0x92u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L"RPM"},
    {L"mon_read_fan_speed_4", L"READ_FAN_SPEED_4", 0x93u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L"RPM"},
    {L"mon_read_duty_cycle", L"READ_DUTY_CYCLE", 0x94u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"%"},
    {L"mon_read_frequency", L"READ_FREQUENCY", 0x95u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"Hz"},
};

const std::vector<DashboardFieldDefinition> kMfrFields = {
    {L"mfr_location", L"MFR_LOCATION", 0x9Cu, DashboardProtocol::BlockRead, DashboardDecode::BlockAscii, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_date", L"MFR_DATE", 0x9Du, DashboardProtocol::BlockRead, DashboardDecode::BlockAscii, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_vin_min", L"MFR_VIN_MIN", 0xA0u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"mfr_vin_max", L"MFR_VIN_MAX", 0xA1u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"mfr_iin_max", L"MFR_IIN_MAX", 0xA2u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"A"},
    {L"mfr_pin_max", L"MFR_PIN_MAX", 0xA3u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"W"},
    {L"mfr_vout_min", L"MFR_VOUT_MIN", 0xA4u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"mfr_vout_max", L"MFR_VOUT_MAX", 0xA5u, DashboardProtocol::ReadWord, DashboardDecode::Linear16Vout, DashboardFieldKind::Extra, 0, 3, L"V"},
    {L"mfr_iout_max", L"MFR_IOUT_MAX", 0xA6u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"A"},
    {L"mfr_pout_max", L"MFR_POUT_MAX", 0xA7u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"W"},
    {L"mfr_tambient_max", L"MFR_TAMBIENT_MAX", 0xA8u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 2, L"C"},
    {L"mfr_tambient_min", L"MFR_TAMBIENT_MIN", 0xA9u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 2, L"C"},
    {L"mfr_efficiency_ll", L"MFR_EFFICIENCY_LL", 0xAAu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_efficiency_hl", L"MFR_EFFICIENCY_HL", 0xABu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_pin_accuracy", L"MFR_PIN_ACCURACY", 0xACu, DashboardProtocol::ReadByte, DashboardDecode::Percent0p1, DashboardFieldKind::Extra, 0, 1, L"%"},
    {L"mfr_ic_device_id", L"IC_DEVICE_ID", 0xADu, DashboardProtocol::BlockRead, DashboardDecode::BlockAscii, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_ic_device_rev", L"IC_DEVICE_REV", 0xAEu, DashboardProtocol::BlockRead, DashboardDecode::BlockAscii, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_00", L"USER_DATA_00", 0xB0u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_01", L"USER_DATA_01", 0xB1u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_02", L"USER_DATA_02", 0xB2u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_efficiency_data", L"MFR_EFFICIENCY_DATA", 0xB3u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_04", L"USER_DATA_04", 0xB4u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_05", L"USER_DATA_05", 0xB5u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_06", L"USER_DATA_06", 0xB6u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_07", L"USER_DATA_07", 0xB7u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_08", L"USER_DATA_08", 0xB8u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_09", L"USER_DATA_09", 0xB9u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_10", L"USER_DATA_10", 0xBAu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_11", L"USER_DATA_11", 0xBBu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_12", L"USER_DATA_12", 0xBCu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_13", L"USER_DATA_13", 0xBDu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_14", L"USER_DATA_14", 0xBEu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_user_data_15", L"USER_DATA_15", 0xBFu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"mfr_max_temp_1", L"MFR_MAX_TEMP_1", 0xC0u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 2, L"C"},
    {L"mfr_max_temp_2", L"MFR_MAX_TEMP_2", 0xC1u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 2, L"C"},
    {L"mfr_max_temp_3", L"MFR_MAX_TEMP_3", 0xC2u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 2, L"C"},
    {L"mfr_cold_redundancy_config", L"MFR_COLD_REDUNDANCY_CONFIG", 0xD0u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
};

const std::vector<DashboardFieldDefinition> kCrpsFields = {
    {L"crps_config_file_size", L"MFR_READ_CONFIG_FILE_SIZE", 0xD1u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 8, 0, L""},
    {L"crps_config_block_size", L"MFR_READ_CONFIG_BLOCK_SIZE", 0xD2u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_hw_compatibility", L"MFR_HW_COMPATIBILITY", 0xD4u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_fwupload_capability", L"MFR_FWUPLOAD_CAPABILITY", 0xD5u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_fwupload_mode", L"MFR_FWUPLOAD_MODE", 0xD6u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_fwupload_status", L"MFR_FWUPLOAD_STATUS", 0xD8u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_fw_revision", L"MFR_FW_REVISION", 0xD9u, DashboardProtocol::BlockRead, DashboardDecode::BlockAscii, DashboardFieldKind::Extra, 8, 0, L""},
    {L"crps_fru_protection", L"MFR_FRU_PROTECTION", 0xDBu, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_blackbox", L"MFR_BLACKBOX", 0xDCu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"crps_real_time_black_box", L"MFR_REAL_TIME_BLACK_BOX", 0xDDu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"crps_system_black_box", L"MFR_SYSTEM_BLACK_BOX", 0xDEu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 40, 0, L""},
    {L"crps_black_box_config", L"MFR_BLACK_BOX_CONFIG", 0xDFu, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_line_status", L"MFR_LINE_STATUS", 0xE1u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_system_led_cntl", L"MFR_SYSTEM_LED_CNTL", 0xE2u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_fwupload_block_size", L"MFR_FWUPLOAD_BLOCK_SIZE", 0xE3u, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_status_simulation", L"MFR_EN_STATUS_SIMULATION_CMD", 0xE4u, DashboardProtocol::ReadByte, DashboardDecode::RawByte, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_peak_current_record", L"MFR_PEAK_CURRENT_RECORD", 0xE9u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"crps_component_id", L"MFR_COMPONENT_ID", 0xEBu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 16, 0, L""},
    {L"crps_tot_pout_max", L"MFR_TOT_POUT_MAX", 0xECu, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L"W"},
    {L"crps_vout_margining", L"MFR_VOUT_MARGINING", 0xEDu, DashboardProtocol::ReadWord, DashboardDecode::RawWord, DashboardFieldKind::Extra, 0, 0, L""},
    {L"crps_ocwpl1_setting", L"MFR_OCWPL1_SETTING", 0xEEu, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 32, 0, L""},
    {L"crps_pwok_warning_time", L"MFR_PWOK_WARNING_TIME", 0xF0u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
    {L"crps_max_iout_capability", L"MFR_MAX_IOUT_CAPABILITY", 0xF1u, DashboardProtocol::BlockRead, DashboardDecode::RawBlock, DashboardFieldKind::Extra, 16, 0, L""},
    {L"crps_fpc_main_min_off", L"MFR_FPC_MAIN_MIN_OFF_TIME", 0xF2u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
    {L"crps_fpc_12vsb_min_off", L"MFR_FPC_12VSB_MIN_OFF_TIME", 0xF3u, DashboardProtocol::ReadWord, DashboardDecode::Linear11, DashboardFieldKind::Extra, 0, 3, L""},
};

} // namespace

const std::vector<DashboardFieldDefinition>& DashboardMainFieldDefinitions() {
    return kMainFields;
}

const std::vector<DashboardFieldDefinition>& DashboardExtraFieldDefinitions() {
    return kExtraFields;
}

const std::vector<DashboardFieldDefinition>& DashboardConfigFieldDefinitions() {
    return kConfigFields;
}

const std::vector<DashboardFieldDefinition>& DashboardFaultFieldDefinitions() {
    return kFaultFields;
}

const std::vector<DashboardFieldDefinition>& DashboardMonitorFieldDefinitions() {
    return kMonitorFields;
}

const std::vector<DashboardFieldDefinition>& DashboardMfrFieldDefinitions() {
    return kMfrFields;
}

const std::vector<DashboardFieldDefinition>& DashboardCrpsFieldDefinitions() {
    return kCrpsFields;
}

const std::vector<DashboardFieldDefinition>& DashboardDetailFieldDefinitions() {
    static const std::vector<DashboardFieldDefinition> fields = []() {
        std::vector<DashboardFieldDefinition> out;
        const auto append = [&out](const std::vector<DashboardFieldDefinition>& src) {
            out.insert(out.end(), src.begin(), src.end());
        };
        append(kConfigFields);
        append(kFaultFields);
        append(kMonitorFields);
        append(kMfrFields);
        append(kCrpsFields);
        return out;
    }();
    return fields;
}

} // namespace mfc_tool::core

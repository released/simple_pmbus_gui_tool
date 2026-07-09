#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mfc_tool::core {

enum class DashboardProtocol {
    ReadByte = 0,
    ReadWord,
    Read32,
    BlockRead,
    NotAvailable,
};

enum class DashboardDecode {
    RawByte = 0,
    RawWord,
    RawDword,
    Linear11,
    Linear16Vout,
    BlockAscii,
    RawBlock,
    PmbusRevision,
    Percent0p1,
};

enum class DashboardFieldKind {
    Dynamic = 0,
    Static,
    Extra,
};

struct DashboardFieldDefinition {
    const wchar_t* id;
    const wchar_t* label;
    std::uint8_t command;
    DashboardProtocol protocol;
    DashboardDecode decode;
    DashboardFieldKind kind;
    int max_block_len;
    int precision;
    const wchar_t* unit;
};

const std::vector<DashboardFieldDefinition>& DashboardMainFieldDefinitions();
const std::vector<DashboardFieldDefinition>& DashboardExtraFieldDefinitions();
const std::vector<DashboardFieldDefinition>& DashboardConfigFieldDefinitions();
const std::vector<DashboardFieldDefinition>& DashboardFaultFieldDefinitions();
const std::vector<DashboardFieldDefinition>& DashboardMonitorFieldDefinitions();
const std::vector<DashboardFieldDefinition>& DashboardMfrFieldDefinitions();
const std::vector<DashboardFieldDefinition>& DashboardCrpsFieldDefinitions();
const std::vector<DashboardFieldDefinition>& DashboardDetailFieldDefinitions();

} // namespace mfc_tool::core

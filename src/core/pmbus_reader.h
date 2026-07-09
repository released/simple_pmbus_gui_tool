#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "bridge_service.h"
#include "dashboard_field_profile.h"

namespace mfc_tool::core {

struct PmbusReadSettings {
    int port = 0;
    int addr7 = 0x5A;
    bool pec = true;
};

struct PmbusReadResult {
    std::vector<std::uint8_t> raw;
    std::vector<std::uint8_t> data;
    bool pec_checked = false;
    bool pec_ok = true;
    std::uint8_t pec_rx = 0;
    std::uint8_t pec_calc = 0;
};

class PmbusReader {
public:
    explicit PmbusReader(BridgeService* service);

    void SetSettings(const PmbusReadSettings& settings);
    void SetVoutMode(std::uint8_t vout_mode);
    [[nodiscard]] std::uint8_t VoutMode() const noexcept { return vout_mode_; }

    PmbusReadResult ReadByte(std::uint8_t command);
    PmbusReadResult ReadWord(std::uint8_t command);
    PmbusReadResult Read32(std::uint8_t command);
    PmbusReadResult BlockRead(std::uint8_t command, int max_len);
    void WriteByte(std::uint8_t command, std::uint8_t value);
    std::wstring ReadFormatted(const DashboardFieldDefinition& field);
    std::wstring FormatResult(const DashboardFieldDefinition& field, const PmbusReadResult& result) const;

private:
    PmbusReadResult ReadByCommand(std::uint8_t command, int read_len);
    static std::uint16_t Le16(const std::vector<std::uint8_t>& data);
    static std::uint32_t Le32(const std::vector<std::uint8_t>& data);
    static std::wstring FormatHexByte(std::uint8_t value);
    static std::wstring FormatHexWord(std::uint16_t value);
    static std::wstring FormatHexDword(std::uint32_t value);
    static std::wstring FormatFixed(double value, int precision, const wchar_t* unit);
    static std::wstring FormatAscii(const std::vector<std::uint8_t>& data);
    static std::wstring FormatRevision(std::uint8_t raw);

private:
    BridgeService* service_ = nullptr;
    PmbusReadSettings settings_;
    std::uint8_t vout_mode_ = 0x17u;
};

} // namespace mfc_tool::core

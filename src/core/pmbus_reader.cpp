#include "pmbus_reader.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "pmbus_utils.h"
#include "text_utils.h"

namespace mfc_tool::core {

PmbusReader::PmbusReader(BridgeService* service) : service_(service) {}

void PmbusReader::SetSettings(const PmbusReadSettings& settings) {
    settings_ = settings;
}

void PmbusReader::SetVoutMode(std::uint8_t vout_mode) {
    vout_mode_ = vout_mode;
}

PmbusReadResult PmbusReader::ReadByte(std::uint8_t command) {
    return ReadByCommand(command, 1);
}

PmbusReadResult PmbusReader::ReadWord(std::uint8_t command) {
    return ReadByCommand(command, 2);
}

PmbusReadResult PmbusReader::Read32(std::uint8_t command) {
    return ReadByCommand(command, 4);
}

PmbusReadResult PmbusReader::ReadByCommand(std::uint8_t command, int read_len) {
    if (service_ == nullptr || !service_->IsConnected()) {
        throw std::runtime_error("PMBus reader is not connected");
    }

    PmbusReadResult result;
    const std::vector<std::uint8_t> command_bytes = {command};
    result.raw = service_->I2cMasterWriteThenReadRaw(settings_.port,
                                                     settings_.addr7,
                                                     true,
                                                     command_bytes,
                                                     read_len + (settings_.pec ? 1 : 0));
    result.data = result.raw;
    if (settings_.pec) {
        if (result.raw.empty()) {
            throw std::runtime_error("PMBus read returned no PEC byte");
        }
        result.pec_checked = true;
        result.pec_rx = result.raw.back();
        result.data.pop_back();
        std::vector<std::uint8_t> frame = {
            static_cast<std::uint8_t>(settings_.addr7 << 1),
            command,
            static_cast<std::uint8_t>((settings_.addr7 << 1) | 1u)
        };
        frame.insert(frame.end(), result.data.begin(), result.data.end());
        result.pec_calc = PmbusComputePec(frame);
        result.pec_ok = result.pec_calc == result.pec_rx;
        if (!result.pec_ok) {
            throw std::runtime_error("PMBus read PEC mismatch");
        }
    }
    return result;
}

PmbusReadResult PmbusReader::BlockRead(std::uint8_t command, int max_len) {
    if (service_ == nullptr || !service_->IsConnected()) {
        throw std::runtime_error("PMBus reader is not connected");
    }

    PmbusReadResult result;
    const std::vector<std::uint8_t> command_bytes = {command};
    result.raw = service_->I2cMasterPmbusBlockReadRaw(settings_.port,
                                                      settings_.addr7,
                                                      command_bytes,
                                                      settings_.pec,
                                                      std::clamp(max_len, 1, 32));
    result.data = result.raw;
    if (settings_.pec) {
        if (result.data.size() < 2u) {
            throw std::runtime_error("PMBus block read missing PEC byte");
        }
        result.pec_checked = true;
        result.pec_rx = result.data.back();
        result.data.pop_back();
    }
    if (result.data.empty()) {
        throw std::runtime_error("PMBus block read missing count byte");
    }
    const std::uint8_t count = result.data[0];
    if (result.data.size() != static_cast<std::size_t>(count) + 1u) {
        throw std::runtime_error("PMBus block read count mismatch");
    }
    if (settings_.pec) {
        std::vector<std::uint8_t> frame = {
            static_cast<std::uint8_t>(settings_.addr7 << 1),
            command,
            static_cast<std::uint8_t>((settings_.addr7 << 1) | 1u)
        };
        frame.insert(frame.end(), result.data.begin(), result.data.end());
        result.pec_calc = PmbusComputePec(frame);
        result.pec_ok = result.pec_calc == result.pec_rx;
        if (!result.pec_ok) {
            throw std::runtime_error("PMBus block read PEC mismatch");
        }
    }
    result.data.erase(result.data.begin());
    return result;
}

void PmbusReader::WriteByte(std::uint8_t command, std::uint8_t value) {
    if (service_ == nullptr || !service_->IsConnected()) {
        throw std::runtime_error("PMBus reader is not connected");
    }

    std::vector<std::uint8_t> tx = {command, value};
    if (settings_.pec) {
        std::vector<std::uint8_t> frame = {static_cast<std::uint8_t>(settings_.addr7 << 1), command, value};
        tx.push_back(PmbusComputePec(frame));
    }
    service_->I2cMasterWrite(settings_.port, settings_.addr7, tx);
}

std::wstring PmbusReader::ReadFormatted(const DashboardFieldDefinition& field) {
    PmbusReadResult result;

    switch (field.protocol) {
    case DashboardProtocol::ReadByte:
        result = ReadByte(field.command);
        break;
    case DashboardProtocol::ReadWord:
        result = ReadWord(field.command);
        break;
    case DashboardProtocol::Read32:
        result = Read32(field.command);
        break;
    case DashboardProtocol::BlockRead:
        result = BlockRead(field.command, field.max_block_len > 0 ? field.max_block_len : 32);
        break;
    default:
        return L"N/A";
    }
    return FormatResult(field, result);
}

std::wstring PmbusReader::FormatResult(const DashboardFieldDefinition& field, const PmbusReadResult& result) const {
    if (result.data.empty()) {
        return L"-";
    }

    switch (field.decode) {
    case DashboardDecode::RawByte:
        return FormatHexByte(result.data[0]);
    case DashboardDecode::RawWord:
        return FormatHexWord(Le16(result.data));
    case DashboardDecode::RawDword:
        return FormatHexDword(Le32(result.data));
    case DashboardDecode::Linear11:
        return FormatFixed(PmbusLinear11ToDouble(Le16(result.data)), field.precision, field.unit);
    case DashboardDecode::Linear16Vout:
        {
            std::int8_t exp = -9;
            (void)TryParseVoutModeExponent(vout_mode_, &exp);
            return FormatFixed(PmbusLinear16ToDouble(Le16(result.data), exp), field.precision, field.unit);
        }
    case DashboardDecode::BlockAscii:
        return FormatAscii(result.data);
    case DashboardDecode::PmbusRevision:
        return FormatRevision(result.data[0]);
    case DashboardDecode::Percent0p1:
        return FormatFixed(static_cast<double>(result.data[0]) / 10.0, field.precision, field.unit);
    case DashboardDecode::RawBlock:
    default:
        return HexDump(result.data);
    }
}

std::uint16_t PmbusReader::Le16(const std::vector<std::uint8_t>& data) {
    if (data.size() < 2u) {
        throw std::runtime_error("PMBus word payload is too short");
    }
    return static_cast<std::uint16_t>(data[0] | (data[1] << 8));
}

std::uint32_t PmbusReader::Le32(const std::vector<std::uint8_t>& data) {
    if (data.size() < 4u) {
        throw std::runtime_error("PMBus dword payload is too short");
    }
    return (static_cast<std::uint32_t>(data[0]) |
            (static_cast<std::uint32_t>(data[1]) << 8) |
            (static_cast<std::uint32_t>(data[2]) << 16) |
            (static_cast<std::uint32_t>(data[3]) << 24));
}

std::wstring PmbusReader::FormatHexByte(std::uint8_t value) {
    std::wstringstream ss;
    ss << L"0x" << std::uppercase << std::hex << std::setw(2) << std::setfill(L'0')
       << static_cast<unsigned int>(value);
    return ss.str();
}

std::wstring PmbusReader::FormatHexWord(std::uint16_t value) {
    std::wstringstream ss;
    ss << L"0x" << std::uppercase << std::hex << std::setw(4) << std::setfill(L'0')
       << static_cast<unsigned int>(value);
    return ss.str();
}

std::wstring PmbusReader::FormatHexDword(std::uint32_t value) {
    std::wstringstream ss;
    ss << L"0x" << std::uppercase << std::hex << std::setw(8) << std::setfill(L'0')
       << static_cast<unsigned long>(value);
    return ss.str();
}

std::wstring PmbusReader::FormatFixed(double value, int precision, const wchar_t* unit) {
    std::wstringstream ss;
    ss.setf(std::ios::fixed, std::ios::floatfield);
    ss.precision(precision >= 0 ? precision : 3);
    ss << value;
    if (unit != nullptr && unit[0] != L'\0') {
        ss << L" " << unit;
    }
    return ss.str();
}

std::wstring PmbusReader::FormatAscii(const std::vector<std::uint8_t>& data) {
    std::wstring out;
    for (std::uint8_t byte : data) {
        if (byte >= 32u && byte <= 126u) {
            out.push_back(static_cast<wchar_t>(byte));
        }
    }
    return out.empty() ? L"-" : out;
}

std::wstring PmbusReader::FormatRevision(std::uint8_t raw) {
    std::wstringstream ss;
    ss << L"1." << static_cast<unsigned int>((raw >> 4) & 0x0Fu)
       << L" / 1." << static_cast<unsigned int>(raw & 0x0Fu)
       << L" (" << FormatHexByte(raw) << L")";
    return ss.str();
}

} // namespace mfc_tool::core
